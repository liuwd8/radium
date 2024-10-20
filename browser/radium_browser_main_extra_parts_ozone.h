// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_OZONE_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_OZONE_H_

#include "radium/browser/radium_browser_main_extra_parts.h"

class RadiumBrowserMainExtraPartsOzone : public RadiumBrowserMainExtraParts {
 public:
  explicit RadiumBrowserMainExtraPartsOzone();
  RadiumBrowserMainExtraPartsOzone(const RadiumBrowserMainExtraPartsOzone&) =
      delete;
  RadiumBrowserMainExtraPartsOzone& operator=(
      const RadiumBrowserMainExtraPartsOzone&) = delete;

  ~RadiumBrowserMainExtraPartsOzone() override;

 protected:
  // RadiumBrowserMainExtraParts:
  void PostCreateMainMessageLoop() override;
  void PostMainMessageLoopRun() override;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_OZONE_H_
