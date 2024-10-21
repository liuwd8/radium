// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_POSIX_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_POSIX_H_

#include "radium/browser/radium_browser_main_parts.h"

class RadiumBrowserMainPartsPosix : public RadiumBrowserMainParts {
 public:
  explicit RadiumBrowserMainPartsPosix(
      RadiumFeatureListCreator* radium_feature_list_creator);
  RadiumBrowserMainPartsPosix(const RadiumBrowserMainPartsPosix&) = delete;
  RadiumBrowserMainPartsPosix& operator=(const RadiumBrowserMainPartsPosix&) =
      delete;

  ~RadiumBrowserMainPartsPosix() override;

  // content::BrowserMainParts overrides.
  int PreEarlyInitialization() override;
  void PostCreateMainMessageLoop() override;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_POSIX_H_
