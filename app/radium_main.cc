// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/compiler_specific.h"
#include "base/functional/callback_helpers.h"
#include "base/sampling_heap_profiler/poisson_allocation_sampler.h"
#include "build/build_config.h"
#include "content/public/app/content_main.h"
#include "radium/app/radium_main_delegate.h"
#include "radium/common/radium_result_codes.h"

#if BUILDFLAG(IS_MAC)
#include "radium/app/radium_main_mac.h"
#endif

#if BUILDFLAG(IS_WIN)
#include <timeapi.h>

#include "base/dcheck_is_on.h"
#include "base/debug/dump_without_crashing.h"
#include "base/debug/handle_hooks_win.h"
#include "base/win/current_module.h"
#include "base/win/win_util.h"
#include "radium/install_static/initialize_from_primary_module.h"
#include "radium/install_static/install_util.h"
#include "radium/radium_elf/radium_elf_main.h"

#define DLLEXPORT __declspec(dllexport)
#endif  // BUILDFLAG(IS_WIN)

#if BUILDFLAG(IS_WIN)
// We use extern C for the prototype DLLEXPORT to avoid C++ name mangling.
extern "C" {
DLLEXPORT int __cdecl RadiumMain(HINSTANCE instance,
                                 sandbox::SandboxInterfaceInfo* sandbox_info,
                                 int64_t exe_main_entry_point_ticks,
                                 int64_t preread_begin_ticks,
                                 int64_t preread_end_ticks);
}
#elif BUILDFLAG(IS_POSIX)
extern "C" {
// This function must be marked with NO_STACK_PROTECTOR or it may crash on
// return, see the --change-stack-guard-on-fork command line flag.
NO_STACK_PROTECTOR __attribute__((visibility("default"))) int RadiumMain(
    int argc,
    const char** argv);
}
#else
#error Unknown platform.
#endif

#if BUILDFLAG(IS_WIN)
DLLEXPORT int __cdecl RadiumMain(HINSTANCE instance,
                                 sandbox::SandboxInterfaceInfo* sandbox_info,
                                 int64_t exe_entry_point_ticks,
                                 int64_t preread_begin_ticks,
                                 int64_t preread_end_ticks) {
#elif BUILDFLAG(IS_POSIX)
int RadiumMain(int argc, const char** argv) {
#else
#error Unknown platform.
#endif

#if BUILDFLAG(IS_WIN)
  install_static::InitializeFromPrimaryModule();
#if !defined(COMPONENT_BUILD) && DCHECK_IS_ON()
  // Patch the main EXE on non-component builds when DCHECKs are enabled.
  // This allows detection of third party code that might attempt to meddle with
  // Chrome's handles. This must be done when single-threaded to avoid other
  // threads attempting to make calls through the hooks while they are being
  // emplaced.
  // Note: The EXE is patched separately, in chrome/app/chrome_exe_main_win.cc.
  base::debug::HandleHooks::AddIATPatch(CURRENT_MODULE());
#endif  // !defined(COMPONENT_BUILD) && DCHECK_IS_ON()
  StartupTimestamps timestamps{
      base::TimeTicks::FromInternalValue(exe_entry_point_ticks),
      base::TimeTicks::FromInternalValue(preread_begin_ticks),
      base::TimeTicks::FromInternalValue(preread_end_ticks)};
  RadiumMainDelegate delegate(timestamps);
#else  // BUILDFLAG(IS_WIN)
  RadiumMainDelegate delegate(
      {.exe_entry_point_ticks = base::TimeTicks::Now()});
#endif
  content::ContentMainParams params(&delegate);

#if BUILDFLAG(IS_WIN)
  // The process should crash when going through abnormal termination, but we
  // must be sure to reset this setting when ChromeMain returns normally.
  auto crash_on_detach_resetter = base::ScopedClosureRunner(
      base::BindOnce(&base::win::SetShouldCrashOnProcessDetach,
                     base::win::ShouldCrashOnProcessDetach()));
  base::win::SetShouldCrashOnProcessDetach(true);
  base::win::SetAbortBehaviorForCrashReporting();
  params.instance = instance;
  params.sandbox_info = sandbox_info;

  // Pass chrome_elf's copy of DumpProcessWithoutCrash resolved via load-time
  // dynamic linking.
  base::debug::SetDumpWithoutCrashingFunction(&DumpProcessWithoutCrash);

  // Verify that chrome_elf and this module (chrome.dll and chrome_child.dll)
  // have the same version.
  if (install_static::VersionMismatch()) {
    base::debug::DumpWithoutCrashing();
  }

  base::CommandLine::Init(0, nullptr);
#else
  params.argc = argc;
  params.argv = argv;
  base::CommandLine::Init(params.argc, params.argv);
#endif  // BUILDFLAG(IS_WIN)
  [[maybe_unused]] base::CommandLine* command_line(
      base::CommandLine::ForCurrentProcess());

#if BUILDFLAG(IS_MAC)
  SetUpBundleOverrides();
#endif

  // PoissonAllocationSampler's TLS slots need to be set up before
  // MainThreadStackSamplingProfiler (in RadiumMainDelegate::ThreadPoolCreated),
  // which can allocate TLS slots of its own. On some platforms pthreads can
  // malloc internally to access higher-numbered TLS slots, which can cause
  // reentry in the heap profiler. (See the comment on
  // ReentryGuard::InitTLSSlot().)
  base::PoissonAllocationSampler::Init();

  int rv = content::ContentMain(std::move(params));

  if (radium::IsNormalResultCode(static_cast<radium::ResultCode>(rv))) {
    return content::RESULT_CODE_NORMAL_EXIT;
  }
  return rv;
}
