// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_MAC_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_MAC_H_

#include <memory>

#include "radium/browser/radium_browser_main_extra_parts.h"

namespace display {
class ScopedNativeScreen;
}

class RadiumBrowserMainExtraPartsMac : public RadiumBrowserMainExtraParts {
 public:
  explicit RadiumBrowserMainExtraPartsMac();
  RadiumBrowserMainExtraPartsMac(const RadiumBrowserMainExtraPartsMac&) =
      delete;
  RadiumBrowserMainExtraPartsMac& operator=(
      const RadiumBrowserMainExtraPartsMac&) = delete;

  ~RadiumBrowserMainExtraPartsMac() override;

  // RadiumBrowserMainParts:
  void PreEarlyInitialization() override;

 private:
  std::unique_ptr<display::ScopedNativeScreen> screen_;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_EXTRA_PARTS_MAC_H_
