// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts_mac.h"

#include "net/cert/internal/system_trust_store.h"

RadiumBrowserMainPartsMac::RadiumBrowserMainPartsMac(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : RadiumBrowserMainPartsPosix(radium_feature_list_creator) {}

RadiumBrowserMainPartsMac::~RadiumBrowserMainPartsMac() = default;

void RadiumBrowserMainPartsMac::PostCreateMainMessageLoop() {
  RadiumBrowserMainPartsPosix::PostCreateMainMessageLoop();

  net::InitializeTrustStoreMacCache();
}
