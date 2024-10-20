// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string_view>

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/debug/debugger.h"
#include "base/environment.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/logging.h"
#include "base/metrics/statistics_recorder.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/common/content_switches.h"
#include "radium/common/logging_radium.h"
#include "radium/common/radium_paths.h"

namespace logging {
namespace {

// When true, this means that error dialogs should not be shown.
bool dialogs_are_suppressed_ = false;
ScopedLogAssertHandler* assert_handler_ = nullptr;

// This should be true for exactly the period between the end of
// InitRadiumLogging() and the beginning of CleanupChromeLogging().
bool radium_logging_initialized_ = false;

// Set if we called InitChromeLogging() but failed to initialize.
bool radium_logging_failed_ = false;

// Assertion handler for logging errors that occur when dialogs are
// silenced.  To record a new error, pass the log string associated
// with that error in the str parameter.
NOINLINE void SilentRuntimeAssertHandler(const char* file,
                                         int line,
                                         std::string_view message,
                                         std::string_view stack_trace) {
  base::debug::BreakDebugger();
}

// Suppresses error/assertion dialogs and enables the logging of
// those errors into silenced_errors_.
void SuppressDialogs() {
  if (dialogs_are_suppressed_) {
    return;
  }

  assert_handler_ = new ScopedLogAssertHandler(
      base::BindRepeating(SilentRuntimeAssertHandler));

#if BUILDFLAG(IS_WIN)
  UINT new_flags =
      SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX;
  // Preserve existing error mode, as discussed at http://t/dmea
  UINT existing_flags = SetErrorMode(new_flags);
  SetErrorMode(existing_flags | new_flags);
#endif

  dialogs_are_suppressed_ = true;
}

base::FilePath GetLogFileName(const base::CommandLine& command_line) {
  // Try the command line.
  auto filename = command_line.GetSwitchValueNative(switches::kLogFile);
  if (!filename.empty()) {
    base::FilePath candidate_path(filename);
#if BUILDFLAG(IS_WIN)
    // Windows requires an absolute path for the --log-file switch. Windows
    // cannot log to the current directory as it cds() to the exe's directory
    // earlier than this function runs.
    candidate_path = candidate_path.NormalizePathSeparators();
    if (candidate_path.IsAbsolute()) {
      return candidate_path;
    } else {
      PLOG(ERROR) << "Invalid logging destination: " << filename;
    }
#else
    return candidate_path;
#endif  // BUILDFLAG(IS_WIN)
  }

  // If command line and environment do not provide a log file we can use,
  // fallback to the default.
  const base::FilePath log_filename(FILE_PATH_LITERAL("radium_debug.log"));
  base::FilePath log_path;

  if (base::PathService::Get(radium::DIR_LOGS, &log_path)) {
    log_path = log_path.Append(log_filename);
    return log_path;
  } else {
#if BUILDFLAG(IS_WIN)
    // On Windows we cannot use a non-absolute path so we cannot provide a file.
    return base::FilePath();
#else
    // Error with path service, just use the default in our current directory.
    return log_filename;
#endif  // BUILDFLAG(IS_WIN)
  }
}

}  // namespace

void InitRadiumLogging(const base::CommandLine& command_line,
                       OldFileDeletionState delete_old_log_file) {
  DCHECK(!radium_logging_initialized_)
      << "Attempted to initialize logging when it was already initialized.";
  bool filename_is_handle = false;
  LoggingDestination logging_dest = LOG_TO_ALL;
  LogLockingState log_locking_state = LOCK_LOG_FILE;
  base::FilePath log_path;

#if BUILDFLAG(IS_WIN)
  base::win::ScopedHandle log_handle;
#endif

  if (logging_dest & LOG_TO_FILE) {
    if (filename_is_handle) {
#if BUILDFLAG(IS_WIN)
      // Child processes on Windows are provided a file handle if logging is
      // enabled as sandboxed processes cannot open files.
      log_handle = GetLogInheritedHandle(command_line);
      if (!log_handle.is_valid()) {
        DLOG(ERROR) << "Unable to initialize logging from handle.";
        radium_logging_failed_ = true;
        return;
      }
#endif
    } else {
      log_path = GetLogFileName(command_line);

#if BUILDFLAG(IS_CHROMEOS_ASH)
      // For BWSI (Incognito) logins, we want to put the logs in the user
      // profile directory that is created for the temporary session instead
      // of in the system log directory, for privacy reasons.
      if (command_line.HasSwitch(ash::switches::kGuestSession)) {
        log_path = GetSessionLogFile(command_line);
      }

      // Prepares a log file.  We rotate the previous log file and prepare a new
      // log file if we've been asked to delete the old log, since that
      // indicates the start of a new session.
      target_path =
          SetUpLogFile(log_path, delete_old_log_file == DELETE_OLD_LOG_FILE);

      // Because ChromeOS manages the move to a new session by redirecting
      // the link, it shouldn't remove the old file in the logging code,
      // since that will remove the newly created link instead.
      delete_old_log_file = APPEND_TO_OLD_LOG_FILE;
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)
    }
  } else {
    log_locking_state = DONT_LOCK_LOG_FILE;
  }

  LoggingSettings settings;
  settings.logging_dest = logging_dest;
  if (!log_path.empty()) {
    settings.log_file_path = log_path.value().c_str();
  }
#if BUILDFLAG(IS_WIN)
  // Avoid initializing with INVALID_HANDLE_VALUE.
  // This handle is owned by the logging framework and is closed when the
  // process exits.
  // TODO(crbug.com/328285906) Use a ScopedHandle in logging settings.
  settings.log_file = log_handle.is_valid() ? log_handle.release() : nullptr;
#endif
  settings.lock_log = log_locking_state;
  settings.delete_old = delete_old_log_file;
  bool success = InitLogging(settings);

#if BUILDFLAG(IS_CHROMEOS_ASH)
  if (!success) {
    DPLOG(ERROR) << "Unable to initialize logging to " << log_path.value()
                 << " (which should be a link to " << target_path.value()
                 << ")";
    RemoveSymlinkAndLog(log_path, target_path);
    radium_logging_failed_ = true;
    return;
  }
#else   // BUILDFLAG(IS_CHROMEOS_ASH)
  if (!success) {
    DPLOG(ERROR) << "Unable to initialize logging to " << log_path.value();
    radium_logging_failed_ = true;
    return;
  }
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

  // We call running in unattended mode "headless", and allow headless mode to
  // be configured either by the Environment Variable or by the Command Line
  // Switch. This is for automated test purposes.
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  const bool is_headless =  // env->HasVar(env_vars::kHeadless) ||
      command_line.HasSwitch(switches::kNoErrorDialogs);

  // Show fatal log messages in a dialog in debug builds when not headless.
  if (!is_headless) {
    SetShowErrorDialogs(true);
  }

  // we want process and thread IDs because we have a lot of things running
  SetLogItems(true,    // enable_process_id
              true,    // enable_thread_id
              true,    // enable_timestamp
              false);  // enable_tickcount

  // Suppress system error dialogs when headless.
  if (is_headless) {
    SuppressDialogs();
  }

  // Use a minimum log level if the command line asks for one. Ignore this
  // switch if there's vlog level switch present too (as both of these switches
  // refer to the same underlying log level, and the vlog level switch has
  // already been processed inside InitLogging). If there is neither
  // log level nor vlog level specified, then just leave the default level
  // (INFO).
  if (command_line.HasSwitch(switches::kLoggingLevel) &&
      GetMinLogLevel() >= 0) {
    std::string log_level =
        command_line.GetSwitchValueASCII(switches::kLoggingLevel);
    int level = 0;
    if (base::StringToInt(log_level, &level) && level >= 0 &&
        level < LOGGING_NUM_SEVERITIES) {
      SetMinLogLevel(level);
    } else {
      DLOG(WARNING) << "Bad log level: " << log_level;
    }
  }

#if BUILDFLAG(IS_WIN)
  // Enable trace control and transport through event tracing for Windows.
  LogEventProvider::Initialize(kChromeTraceProviderName);

  // Enable logging to the Windows Event Log.
  SetEventSource(base::WideToASCII(
                     install_static::InstallDetails::Get().install_full_name()),
                 BROWSER_CATEGORY, MSG_LOG_MESSAGE);
#endif

  base::StatisticsRecorder::InitLogOnShutdown();

  radium_logging_initialized_ = true;
}

// This is a no-op, but we'll keep it around in case
// we need to do more cleanup in the future.
void CleanupRadiumLogging() {
  if (radium_logging_failed_) {
    return;  // We failed to initiailize logging, no cleanup.
  }

  // Logging was not initialized, no cleanup required. This is happening with
  // the Chrome early exit error paths (i.e Process Singleton).
  if (!radium_logging_initialized_) {
    return;
  }

  CloseLogFile();

  radium_logging_initialized_ = false;
  radium_logging_failed_ = false;
}

}  // namespace logging
