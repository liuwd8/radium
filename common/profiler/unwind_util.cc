// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/profiler/unwind_util.h"

#include "radium/common/channel_info.h"

bool AreUnwindPrerequisitesAvailable(
    version_info::Channel channel,
    UnwindPrerequisitesDelegate* prerequites_delegate) {
// While non-Android platforms do not need any specific prerequisites beyond
// what is already bundled and available with Chrome for their platform-specific
// unwinders to work, Android, in particular, requires a DFM to be installed.
//
// Therefore, unwind prerequisites for non-supported Android platforms are not
// considered to be available by default, but prerequisites for non-Android
// platforms are considered to be available by default.
//
// This is also why we do not need to check `prerequites_delegate` for
// non-Android platforms. Regardless of the provided delegate, unwind
// prerequisites are always considered to be available for non-Android
// platforms.
#if BUILDFLAG(IS_ANDROID)
#if ANDROID_UNWINDING_SUPPORTED
#if defined(OFFICIAL_BUILD) && BUILDFLAG(GOOGLE_CHROME_BRANDING)
  // Sometimes, DFMs can be installed even if not requested by Chrome
  // explicitly (for instance, in some app stores). Therefore, even if the
  // unwinder module is installed, we only consider it to be available for
  // specific channels.
  if (!(channel == version_info::Channel::CANARY ||
        channel == version_info::Channel::DEV ||
        channel == version_info::Channel::BETA)) {
    return false;
  }
#endif  // defined(OFFICIAL_BUILD) && BUILDFLAG(GOOGLE_CHROME_BRANDING)
  ModuleUnwindPrerequisitesDelegate default_delegate;
  if (prerequites_delegate == nullptr) {
    prerequites_delegate = &default_delegate;
  }
  return prerequites_delegate->AreAvailable(channel);
#else   // ANDROID_UNWINDING_SUPPORTED
  return false;
#endif  // ANDROID_UNWINDING_SUPPORTED
#else   // BUILDFLAG(IS_ANDROID)
  return true;
#endif  // BUILDFLAG(IS_ANDROID)
}

#if ANDROID_UNWINDING_SUPPORTED
stack_unwinder::Module* GetOrLoadModule() {
  CHECK(AreUnwindPrerequisitesAvailable(chrome::GetChannel()));
  static base::NoDestructor<std::unique_ptr<stack_unwinder::Module>>
      stack_unwinder_module(stack_unwinder::Module::Load());
  return stack_unwinder_module.get()->get();
}
#endif  // ANDROID_UNWINDING_SUPPORTED

base::StackSamplingProfiler::UnwindersFactory CreateCoreUnwindersFactory() {
  if (!AreUnwindPrerequisitesAvailable(radium::GetChannel())) {
    return base::StackSamplingProfiler::UnwindersFactory();
  }
#if ANDROID_UNWINDING_SUPPORTED
  return base::BindOnce(CreateCoreUnwinders, GetOrLoadModule());
#else   // ANDROID_UNWINDING_SUPPORTED
  return base::StackSamplingProfiler::UnwindersFactory();
#endif  // ANDROID_UNWINDING_SUPPORTED
}

base::StackSamplingProfiler::UnwindersFactory
CreateLibunwindstackUnwinderFactory() {
  if (!AreUnwindPrerequisitesAvailable(radium::GetChannel())) {
    return base::StackSamplingProfiler::UnwindersFactory();
  }
#if ANDROID_UNWINDING_SUPPORTED
  return base::BindOnce(CreateLibunwindstackUnwinders, GetOrLoadModule());
#else   // ANDROID_UNWINDING_SUPPORTED
  return base::StackSamplingProfiler::UnwindersFactory();
#endif  // ANDROID_UNWINDING_SUPPORTED
}
