// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_LINUX_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_LINUX_H_

#include "radium/browser/radium_browser_main_extra_parts_ozone.h"

class RadiumBrowserMainExtraPartsLinux
    : public RadiumBrowserMainExtraPartsOzone {
 public:
  explicit RadiumBrowserMainExtraPartsLinux();
  RadiumBrowserMainExtraPartsLinux(const RadiumBrowserMainExtraPartsLinux&) =
      delete;
  RadiumBrowserMainExtraPartsLinux& operator=(
      const RadiumBrowserMainExtraPartsLinux&) = delete;

  ~RadiumBrowserMainExtraPartsLinux() override;

  static void InitOzonePlatformHint();
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_LINUX_H_
