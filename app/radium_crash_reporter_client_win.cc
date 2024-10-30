// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/app/radium_crash_reporter_client_win.h"

#include <windows.h>

#include <assert.h>
#include <shellapi.h>

#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/debug/leak_annotations.h"
#include "base/files/file_path.h"
#include "base/format_macros.h"
#include "base/notreached.h"
#include "base/rand_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/current_module.h"
#include "components/crash/core/app/crashpad.h"
#include "components/version_info/channel.h"
#include "radium/common/radium_result_codes.h"
#include "radium/install_static/install_util.h"
#include "radium/install_static/user_data_dir.h"
#include "radium/radium_elf/radium_elf_constants.h"

RadiumCrashReporterClient::RadiumCrashReporterClient() = default;

RadiumCrashReporterClient::~RadiumCrashReporterClient() = default;

// static
void RadiumCrashReporterClient::InitializeCrashReportingForProcess() {
  static RadiumCrashReporterClient* instance = nullptr;
  if (instance) {
    return;
  }

  instance = new RadiumCrashReporterClient();
  ANNOTATE_LEAKING_OBJECT_PTR(instance);

  std::wstring process_type = install_static::GetCommandLineSwitchValue(
      ::GetCommandLine(), install_static::kProcessType);

  // Don't set up Crashpad crash reporting in the Crashpad handler itself, nor
  // in the fallback crash handler for the Crashpad handler process.
  if (process_type != install_static::kCrashpadHandler &&
      process_type != install_static::kFallbackHandler) {
    crash_reporter::SetCrashReporterClient(instance);

    std::wstring user_data_dir;
    if (process_type.empty()) {
      install_static::GetUserDataDirectory(&user_data_dir, nullptr);
    }

    // TODO(wfh): Add a DCHECK for success. See https://crbug.com/1329269.
    std::ignore = crash_reporter::InitializeCrashpadWithEmbeddedHandler(
        /*initial_client=*/process_type.empty(),
        install_static::WideToUTF8(process_type),
        install_static::WideToUTF8(user_data_dir), base::FilePath());
  }
}

bool RadiumCrashReporterClient::GetAlternativeCrashDumpLocation(
    std::wstring* crash_dir) {
  // By setting the BREAKPAD_DUMP_LOCATION environment variable, an alternate
  // location to write breakpad crash dumps can be set.
  *crash_dir = install_static::GetEnvironmentString(L"BREAKPAD_DUMP_LOCATION");
  return !crash_dir->empty();
}

void RadiumCrashReporterClient::GetProductNameAndVersion(
    const std::wstring& exe_path,
    std::wstring* product_name,
    std::wstring* version,
    std::wstring* special_build,
    std::wstring* channel_name) {
  assert(product_name);
  assert(version);
  assert(special_build);
  assert(channel_name);

  install_static::GetExecutableVersionDetails(exe_path, product_name, version,
                                              special_build, channel_name);
}

bool RadiumCrashReporterClient::ShouldShowRestartDialog(std::wstring* title,
                                                        std::wstring* message,
                                                        bool* is_rtl_locale) {
  return false;
}

bool RadiumCrashReporterClient::AboutToRestart() {
  return false;
}

bool RadiumCrashReporterClient::GetIsPerUserInstall() {
  return true;
}

bool RadiumCrashReporterClient::GetShouldDumpLargerDumps() {
  return false;
}

int RadiumCrashReporterClient::GetResultCodeRespawnFailed() {
  return radium::RESULT_CODE_RESPAWN_FAILED;
}

bool RadiumCrashReporterClient::ReportingIsEnforcedByPolicy(
    bool* crashpad_enabled) {
  return false;
}

bool RadiumCrashReporterClient::GetCrashDumpLocation(std::wstring* crash_dir) {
  // By setting the BREAKPAD_DUMP_LOCATION environment variable, an alternate
  // location to write breakpad crash dumps can be set.
  // If this environment variable exists, then for the time being,
  // short-circuit how it's handled on Windows. Honoring this
  // variable is required in order to symbolize stack traces in
  // Telemetry based tests: http://crbug.com/561763.
  if (GetAlternativeCrashDumpLocation(crash_dir)) {
    return true;
  }

  *crash_dir = install_static::GetCrashDumpLocation();
  return !crash_dir->empty();
}

bool RadiumCrashReporterClient::GetCrashMetricsLocation(
    std::wstring* metrics_dir) {
  if (!GetCollectStatsConsent()) {
    return false;
  }
  install_static::GetUserDataDirectory(metrics_dir, nullptr);
  return !metrics_dir->empty();
}

bool RadiumCrashReporterClient::IsRunningUnattended() {
  return false;
}

bool RadiumCrashReporterClient::GetCollectStatsConsent() {
  return false;
}

bool RadiumCrashReporterClient::GetCollectStatsInSample() {
  return false;
}

bool RadiumCrashReporterClient::ShouldMonitorCrashHandlerExpensively() {
  return false;
}

bool RadiumCrashReporterClient::EnableBreakpadForProcess(
    const std::string& process_type) {
  // This is not used by Crashpad (at least on Windows).
  NOTREACHED_IN_MIGRATION();
  return true;
}

std::wstring RadiumCrashReporterClient::GetWerRuntimeExceptionModule() {
  // We require that chrome_wer.dll is installed next to chrome_elf.dll. This
  // approach means we don't need to check for the dll's existence on disk early
  // in the process lifetime - we never load this dll ourselves - it is only
  // loaded by WerFault.exe after certain crashes. We do not use base::FilePath
  // and friends as chrome_elf will eventually not depend on //base.

  wchar_t elf_file[MAX_PATH];
  DWORD len = GetModuleFileName(CURRENT_MODULE(), elf_file, MAX_PATH);
  // On error return an empty path to indicate than a module is not to be
  // registered. This is harmless.
  if (len == 0 || len == MAX_PATH) {
    return std::wstring();
  }

  wchar_t elf_dir[MAX_PATH];
  wchar_t* file_start = nullptr;
  DWORD dir_len = GetFullPathName(elf_file, MAX_PATH, elf_dir, &file_start);
  if (dir_len == 0 || dir_len > len || !file_start) {
    return std::wstring();
  }

  // file_start points to the start of the filename in the elf_dir buffer.
  return std::wstring(elf_dir, file_start).append(kWerDll);
}
