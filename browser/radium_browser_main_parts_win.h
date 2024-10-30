// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_WIN_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_WIN_H_

#include "radium/browser/radium_browser_main_parts.h"

class RadiumBrowserMainPartsWin : public RadiumBrowserMainParts {
 public:
  explicit RadiumBrowserMainPartsWin(
      RadiumFeatureListCreator* radium_feature_list_creator);
  RadiumBrowserMainPartsWin(const RadiumBrowserMainPartsWin&) = delete;
  RadiumBrowserMainPartsWin& operator=(const RadiumBrowserMainPartsWin&) =
      delete;

  ~RadiumBrowserMainPartsWin() override;

 private:
  // content::BrowserMainParts overrides.
  void ToolkitInitialized() override;
  void PreCreateMainMessageLoop() override;
  int PreCreateThreads() override;
  void PostCreateThreads() override;
  void PostMainMessageLoopRun() override;

  // RadiumBrowserMainParts:
  void PostBrowserStart() override;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_WIN_H_
