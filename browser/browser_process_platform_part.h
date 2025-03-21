// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_H_
#define RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_H_

#include "build/build_config.h"
#include "build/chromeos_buildflags.h"

// Include the appropriate BrowserProcessPlatformPart based on the platform.
#if BUILDFLAG(IS_ANDROID)
#include "radium/browser/browser_process_platform_part_android.h"  // IWYU pragma: export
#elif BUILDFLAG(IS_MAC)
#include "radium/browser/browser_process_platform_part_mac.h"  // IWYU pragma: export
#else
#include "radium/browser/browser_process_platform_part_base.h"  // IWYU pragma: export
class BrowserProcessPlatformPart : public BrowserProcessPlatformPartBase {};
#endif

#endif  // RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_H_
