// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_RADIUM_DESCRIPTORS_H_
#define RADIUM_COMMON_RADIUM_DESCRIPTORS_H_

#include "build/build_config.h"
#include "content/public/common/content_descriptors.h"

enum {
#if BUILDFLAG(IS_ANDROID)
  kAndroidLocalePakDescriptor = kContentIPCDescriptorMax + 1,
  kAndroidSecondaryLocalePakDescriptor,
  kAndroidChrome100PercentPakDescriptor,
  kAndroidUIResourcesPakDescriptor,
  // DFMs with native resources typically do not share file descriptors with
  // child processes. Hence no corresponding *PakDescriptor is defined.
  kAndroidMinidumpDescriptor,
#endif
};

#endif  // RADIUM_COMMON_RADIUM_DESCRIPTORS_H_
