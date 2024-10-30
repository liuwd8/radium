// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <array>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/debug/handle_hooks_win.h"
#include "base/feature_list.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/process/memory.h"
#include "base/process/process.h"
#include "base/strings/string_number_conversions_win.h"
#include "base/strings/string_util_win.h"
#include "base/win/current_module.h"
#include "base/win/win_util.h"
#include "base/win/windows_version.h"
#include "build/build_config.h"
#include "content/public/common/content_switches.h"
#include "radium/app/main_dll_loader_win.h"
// #include "radium/radium_elf/radium_elf_main.h"

namespace {
// // Sets the current working directory for the process to the directory
// holding
// // the executable if this is the browser process. This avoids leaking a
// handle
// // to an arbitrary directory to child processes (e.g., the crashpad handler
// // process) created before MainDllLoader changes the current working
// directory
// // to the browser's version directory.
// void SetCwdForBrowserProcess() {
//   if (!::IsBrowserProcess()) {
//     return;
//   }

//   std::array<wchar_t, MAX_PATH + 1> buffer;
//   buffer[0] = L'\0';
//   DWORD length = ::GetModuleFileName(nullptr, &buffer[0], buffer.size());
//   if (!length || length >= buffer.size()) {
//     return;
//   }

//   base::SetCurrentDirectory(
//       base::FilePath(base::FilePath::StringPieceType(&buffer[0], length))
//           .DirName());
// }

// Returns true if the child process |command_line| contains a /prefetch:#
// argument where # is in [1, 8] prior to Win11 and [1,16] for it and later.
// The intent of the function is to ensure that all child processes have a
// /prefetch:N cmd line arg in the required range.
// No child process shall have /prefetch:0 or it will interefere with the main
// browser process prefetch. This includes things like /prefetch:simians where
// simians will evalate to 0. Absence of a /prefetch:N argument is the same as
// /prefetch:0 and is also excluded.
// The function assumes only one /prefetch:N argument for child processes.
bool HasValidWindowsPrefetchArgument(const base::CommandLine& command_line) {
  static constexpr std::wstring_view kPrefetchArgumentPrefix(L"/prefetch:");

  for (const auto& arg : command_line.argv()) {
    if (!base::StartsWith(arg, kPrefetchArgumentPrefix)) {
      continue;  // Ignore arguments that don't start with "/prefetch:".
    }
    auto value = std::wstring_view(arg).substr(kPrefetchArgumentPrefix.size());
    int profile = 0;
    return base::StringToInt(value, &profile) && profile >= 1 &&
           profile <=
               (base::win::GetVersion() < base::win::Version::WIN11 ? 8 : 16);
  }
  return false;
}

}  // namespace

#if !defined(WIN_CONSOLE_APP)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE prev, wchar_t*, int) {
#else   // !defined(WIN_CONSOLE_APP)
int main() {
  HINSTANCE instance = GetModuleHandle(nullptr);
#endif  // !defined(WIN_CONSOLE_APP)

#if defined(ARCH_CPU_32_BITS)
  enum class FiberStatus { kConvertFailed, kCreateFiberFailed, kSuccess };
  FiberStatus fiber_status = FiberStatus::kSuccess;
  // GetLastError result if fiber conversion failed.
  DWORD fiber_error = ERROR_SUCCESS;
  if (!::IsThreadAFiber()) {
    // Make the main thread's stack size 4 MiB so that it has roughly the same
    // effective size as the 64-bit build's 8 MiB stack.
    constexpr size_t kStackSize = 4 * 1024 * 1024;  // 4 MiB
    // Leak the fiber on exit.
    LPVOID original_fiber =
        ::ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
    if (original_fiber) {
      FiberState fiber_state = {instance, original_fiber};
      // Create a fiber with a bigger stack and switch to it. Leak the fiber on
      // exit.
      LPVOID big_stack_fiber = ::CreateFiberEx(
          0, kStackSize, FIBER_FLAG_FLOAT_SWITCH, FiberBinder, &fiber_state);
      if (big_stack_fiber) {
        ::SwitchToFiber(big_stack_fiber);
        // The fibers must be cleaned up to avoid obscure TLS-related shutdown
        // crashes.
        ::DeleteFiber(big_stack_fiber);
        ::ConvertFiberToThread();
        // Control returns here after Chrome has finished running on FiberMain.
        return fiber_state.fiber_result;
      }
      fiber_status = FiberStatus::kCreateFiberFailed;
    } else {
      fiber_status = FiberStatus::kConvertFailed;
    }
    // If we reach here then creating and switching to a fiber has failed. This
    // probably means we are low on memory and will soon crash. Try to report
    // this error once crash reporting is initialized.
    fiber_error = ::GetLastError();
    base::debug::Alias(&fiber_error);
  }
  // If we are already a fiber then continue normal execution.
#endif  // defined(ARCH_CPU_32_BITS)

  // SetCwdForBrowserProcess();
  // SignalInitializeCrashReporting();
  // if (IsBrowserProcess()) {
  //   chrome::DisableDelayLoadFailureHooksForMainExecutable();
  // }

  // Done here to ensure that OOMs that happen early in process initialization
  // are correctly signaled to the OS.
  base::EnableTerminationOnOutOfMemory();
  logging::RegisterAbslAbortHook();

  // Initialize the CommandLine singleton from the environment.
  base::CommandLine::Init(0, nullptr);
  const base::CommandLine* command_line =
      base::CommandLine::ForCurrentProcess();

  const std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);

#if !defined(COMPONENT_BUILD) && DCHECK_IS_ON()
  // In non-component mode, chrome.exe contains its own base::FeatureList
  // instance pointer, which remains nullptr. Attempts to access feature state
  // from chrome.exe should fail, instead of silently returning a default state.
  base::FeatureList::FailOnFeatureAccessWithoutFeatureList();

  // Patch the main EXE on non-component builds when DCHECKs are enabled.
  // This allows detection of third party code that might attempt to meddle with
  // Chrome's handles. This must be done when single-threaded to avoid other
  // threads attempting to make calls through the hooks while they are being
  // emplaced.
  // Note: The DLL is patched separately, in chrome/app/chrome_main.cc.
  base::debug::HandleHooks::AddIATPatch(CURRENT_MODULE());
#endif  // !defined(COMPONENT_BUILD) && DCHECK_IS_ON()

  // Confirm that an explicit prefetch profile is used for all process types
  // except for the browser process. Any new process type will have to assign
  // itself a prefetch id. See kPrefetchArgument* constants in
  // content_switches.cc for details.
  DCHECK(process_type.empty() ||
         HasValidWindowsPrefetchArgument(*command_line));

  const base::TimeTicks exe_entry_point_ticks = base::TimeTicks::Now();

  // // Signal Chrome Elf that Chrome has begun to start.
  // SignalRadiumElf();

  // The exit manager is in charge of calling the dtors of singletons.
  base::AtExitManager exit_manager;

  // Load and launch the chrome dll. *Everything* happens inside.
  VLOG(1) << "About to load main DLL.";
  MainDllLoader* loader = MakeMainDllLoader();
  int rc = loader->Launch(instance, exe_entry_point_ticks);
  loader->RelaunchRadiumWithNewCommandLineIfNeeded();
  delete loader;

  // Process shutdown is hard and some process types have been crashing during
  // shutdown. TerminateCurrentProcessImmediately is safer and faster.
  if (process_type == switches::kUtilityProcess ||
      process_type == switches::kPpapiPluginProcess) {
    base::Process::TerminateCurrentProcessImmediately(rc);
  }
  return rc;
}
