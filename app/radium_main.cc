// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/compiler_specific.h"
#include "base/sampling_heap_profiler/poisson_allocation_sampler.h"
#include "build/build_config.h"
#include "content/public/app/content_main.h"
#include "radium/app/radium_main_delegate.h"
#include "radium/common/radium_result_codes.h"

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
  RadiumMainDelegate delegate(
      {.exe_entry_point_ticks = base::TimeTicks::Now()});
  content::ContentMainParams params(&delegate);
#if BUILDFLAG(IS_WIN)
  base::CommandLine::Init(0, nullptr);
#else
  params.argc = argc;
  params.argv = argv;
  base::CommandLine::Init(params.argc, params.argv);
#endif  // BUILDFLAG(IS_WIN)
  [[maybe_unused]] base::CommandLine* command_line(
      base::CommandLine::ForCurrentProcess());

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
