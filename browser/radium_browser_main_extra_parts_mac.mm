// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_extra_parts_mac.h"

#include "ui/display/screen.h"

RadiumBrowserMainExtraPartsMac::RadiumBrowserMainExtraPartsMac() = default;
RadiumBrowserMainExtraPartsMac::~RadiumBrowserMainExtraPartsMac() = default;

void RadiumBrowserMainExtraPartsMac::PreEarlyInitialization() {
  screen_ = std::make_unique<display::ScopedNativeScreen>();
}
