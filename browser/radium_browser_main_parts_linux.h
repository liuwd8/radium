// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_LINUX_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_LINUX_H_

#include "radium/browser/radium_browser_main_parts_posix.h"

class RadiumBrowserMainPartsLinux : public RadiumBrowserMainPartsPosix {
 public:
  explicit RadiumBrowserMainPartsLinux(
      RadiumFeatureListCreator* radium_feature_list_creator);
  RadiumBrowserMainPartsLinux(const RadiumBrowserMainPartsLinux&) = delete;
  RadiumBrowserMainPartsLinux& operator=(const RadiumBrowserMainPartsLinux&) =
      delete;

  ~RadiumBrowserMainPartsLinux() override;

  void PostCreateMainMessageLoop() override;
  void PreProfileInit() override;
  void PostMainMessageLoopRun() override;
  void PostDestroyThreads() override;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_LINUX_H_
