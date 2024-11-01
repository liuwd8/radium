// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_MAC_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_MAC_H_

#include "radium/browser/radium_browser_main_parts_posix.h"

class RadiumBrowserMainPartsMac : public RadiumBrowserMainPartsPosix {
 public:
  explicit RadiumBrowserMainPartsMac(
      RadiumFeatureListCreator* radium_feature_list_creator);
  RadiumBrowserMainPartsMac(const RadiumBrowserMainPartsMac&) = delete;
  RadiumBrowserMainPartsMac& operator=(const RadiumBrowserMainPartsMac&) =
      delete;

  ~RadiumBrowserMainPartsMac() override;

  // BrowserParts overrides.
  void PostCreateMainMessageLoop() override;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_MAC_H_
