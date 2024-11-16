// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_BASE_H_
#define RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_BASE_H_

class BrowserProcessPlatformPartBase {
 public:
  explicit BrowserProcessPlatformPartBase();
  BrowserProcessPlatformPartBase(const BrowserProcessPlatformPartBase&) =
      delete;
  BrowserProcessPlatformPartBase& operator=(
      const BrowserProcessPlatformPartBase&) = delete;

  virtual ~BrowserProcessPlatformPartBase();

  // Called in the middle of BrowserProcessImpl::StartTearDown().
  virtual void StartTearDown();

  // Called from AttemptExitInternal().
  virtual void AttemptExit(bool try_to_quit_application);

  // Called at the end of BrowserProcessImpl::PreMainMessageLoopRun().
  virtual void PreMainMessageLoopRun();
};

#endif  // RADIUM_BROWSER_BROWSER_PROCESS_PLATFORM_PART_BASE_H_
