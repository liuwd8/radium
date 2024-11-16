// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts_win.h"

// windows.h must be included before shellapi.h
#include <windows.h>

#include <delayimp.h>

#include "base/files/file_util.h"
#include "base/files/important_file_writer_cleaner.h"
#include "base/path_service.h"
#include "base/process/process.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/threading/platform_thread.h"
#include "base/version_info/channel.h"
#include "base/win/wrapped_window_proc.h"
#include "components/crash/core/app/crash_export_thunks.h"
#include "components/crash/core/app/dump_hung_process_with_ptype.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "content/public/browser/render_process_host.h"
#include "radium/browser/browser_process.h"
#include "radium/common/channel_info.h"
#include "radium/common/radium_paths.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_win.h"
#include "ui/gfx/system_fonts_win.h"
#include "ui/gfx/win/crash_id_helper.h"
#include "ui/strings/grit/app_locale_settings.h"

namespace {

void InitializeWindowProcExceptions() {
  base::win::WinProcExceptionFilter exception_filter =
      base::win::SetWinProcExceptionFilter(&CrashForException_ExportThunk);
  DCHECK(!exception_filter);
}

// TODO(siggi): Remove once https://crbug.com/806661 is resolved.
void DumpHungRendererProcessImpl(const base::Process& renderer) {
  // Use a distinguishing process type for these reports.
  crash_reporter::DumpHungProcessWithPtype(renderer, "hung-renderer");
}

int GetMinimumFontSize() {
  int min_font_size;
  base::StringToInt(l10n_util::GetStringUTF16(IDS_MINIMUM_UI_FONT_SIZE),
                    &min_font_size);
  return min_font_size;
}

}  // namespace

RadiumBrowserMainPartsWin::RadiumBrowserMainPartsWin(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : RadiumBrowserMainParts(radium_feature_list_creator) {}

RadiumBrowserMainPartsWin::~RadiumBrowserMainPartsWin() = default;

void RadiumBrowserMainPartsWin::ToolkitInitialized() {
  DCHECK_NE(base::PlatformThread::CurrentId(), base::kInvalidThreadId);
  gfx::CrashIdHelper::RegisterMainThread(base::PlatformThread::CurrentId());
  RadiumBrowserMainParts::ToolkitInitialized();
  gfx::win::SetAdjustFontCallback(&l10n_util::AdjustUiFont);
  gfx::win::SetGetMinimumFontSizeCallback(&GetMinimumFontSize);
}

void RadiumBrowserMainPartsWin::PreCreateMainMessageLoop() {
  // // installer_util references strings that are normally compiled into
  // // setup.exe.  In Chrome, these strings are in the locale files.
  // SetupInstallerUtilStrings();

  PrefService* local_state = BrowserProcess::Get()->local_state();
  DCHECK(local_state);

  // Initialize the OSCrypt.
  bool os_crypt_init = OSCrypt::Init(local_state);
  DCHECK(os_crypt_init);

  base::SetExtraNoExecuteAllowedPath(radium::DIR_USER_DATA);

  RadiumBrowserMainParts::PreCreateMainMessageLoop();
  // if (!is_integration_test()) {
  // Make sure that we know how to handle exceptions from the message loop.
  InitializeWindowProcExceptions();
  // }
}

int RadiumBrowserMainPartsWin::PreCreateThreads() {
  // // Set crash keys containing the registry values used to determine Chrome's
  // // update channel at process startup; see https://crbug.com/579504.
  // const auto& details = install_static::InstallDetails::Get();

  static crash_reporter::CrashKeyString<50> ap_value("ap");
  // ap_value.Set(base::WideToUTF8(details.update_ap()));
  ap_value.Set("radium");

  static crash_reporter::CrashKeyString<32> update_cohort_name("cohort-name");
  update_cohort_name.Set("radium");

  if (radium::GetChannel() == version_info::Channel::CANARY) {
    content::RenderProcessHost::SetHungRendererAnalysisFunction(
        &DumpHungRendererProcessImpl);
  }

  // // Pass the value of the UiAutomationProviderEnabled enterprise policy, if
  // // set, down to the accessibility platform after the platform is
  // initialized
  // // in BrowserMainLoop::PostCreateMainMessageLoop() but before any UI is
  // // created.
  // if (auto* local_state = BrowserProcess::Get()->local_state(); local_state)
  // {
  //   if (auto* pref =
  //           local_state->FindPreference(prefs::kUiAutomationProviderEnabled);
  //       pref && pref->IsManaged()) {
  //     ui::AXPlatform::GetInstance().SetUiaProviderEnabled(
  //         pref->GetValue()->GetBool());
  //   }
  // }

  return RadiumBrowserMainParts::PreCreateThreads();
}

void RadiumBrowserMainPartsWin::PostCreateThreads() {
  // performance_manager::InitializeDllPrereadPolicy();

  RadiumBrowserMainParts::PostCreateThreads();
}

void RadiumBrowserMainPartsWin::PostMainMessageLoopRun() {
  base::ImportantFileWriterCleaner::GetInstance().Stop();

  // // The `ProfileManager` has been destroyed, so no new platform
  // authentication
  // // requests will be created.
  // platform_auth_policy_observer_.reset();

  RadiumBrowserMainParts::PostMainMessageLoopRun();
}

void RadiumBrowserMainPartsWin::PostBrowserStart() {
  RadiumBrowserMainParts::PostBrowserStart();

  // Verify that the delay load helper hooks are in place. This cannot be tested
  // from unit tests, so rely on this failing here.
  DCHECK(__pfnDliFailureHook2);

  // InitializeRadiumElf();

  // // Some users are getting stuck in compatibility mode. Try to help them
  // // escape; see http://crbug.com/581499.
  // base::ThreadPool::PostTask(
  //     FROM_HERE, {base::TaskPriority::BEST_EFFORT, base::MayBlock()},
  //     base::BindOnce([]() {
  //       base::FilePath current_exe;
  //       if (base::PathService::Get(base::FILE_EXE, &current_exe)) {
  //         RemoveAppCompatEntries(current_exe);
  //       }
  //     }));

  base::ImportantFileWriterCleaner::GetInstance().Start();
}
