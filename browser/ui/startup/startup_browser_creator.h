// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_STARTUP_STARTUP_BROWSER_CREATOR_H_
#define RADIUM_BROWSER_UI_STARTUP_STARTUP_BROWSER_CREATOR_H_

class StartupBrowserCreator {
 public:
  explicit StartupBrowserCreator();
  StartupBrowserCreator(const StartupBrowserCreator&) = delete;
  StartupBrowserCreator& operator=(const StartupBrowserCreator&) = delete;

  ~StartupBrowserCreator();
};

#endif  // RADIUM_BROWSER_UI_STARTUP_STARTUP_BROWSER_CREATOR_H_
