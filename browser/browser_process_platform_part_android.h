// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_ANDROID_H_
#define RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_ANDROID_H_

#include "radium/browser/browser_process_platform_part_base.h"

class BrowserProcessPlatformPart : public BrowserProcessPlatformPartBase {
 public:
  explicit BrowserProcessPlatformPart();
  BrowserProcessPlatformPart(const BrowserProcessPlatformPart&) = delete;
  BrowserProcessPlatformPart& operator=(const BrowserProcessPlatformPart&) =
      delete;

  ~BrowserProcessPlatformPart() override;

  // Overridden from BrowserProcessPlatformPartBase:
  void AttemptExit(bool try_to_quit_application) override;
};

#endif  // RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_ANDROID_H_
