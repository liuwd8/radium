// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_H_

class RadiumBrowserMainExtraParts {
 public:
  virtual ~RadiumBrowserMainExtraParts() = default;

  // EarlyInitialization methods.
  virtual void PreEarlyInitialization() {}
  virtual void PostEarlyInitialization() {}

  // ToolkitInitialized methods.
  virtual void ToolkitInitialized() {}

  // CreateMainMessageLoop methods.
  virtual void PreCreateMainMessageLoop() {}
  virtual void PostCreateMainMessageLoop() {}

  // MainMessageLoopRun methods.
  virtual void PreCreateThreads() {}
  virtual void PreProfileInit() {}
  virtual void PreBrowserStart() {}
  virtual void PostBrowserStart() {}
  virtual void PreMainMessageLoopRun() {}
  virtual void PostMainMessageLoopRun() {}
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_H_
