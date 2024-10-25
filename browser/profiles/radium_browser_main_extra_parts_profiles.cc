// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/radium_browser_main_extra_parts_profiles.h"

#include "radium/browser/themes/theme_service_factory.h"

RadiumBrowserMainExtraPartsProfiles::RadiumBrowserMainExtraPartsProfiles() =
    default;
RadiumBrowserMainExtraPartsProfiles::~RadiumBrowserMainExtraPartsProfiles() =
    default;

void RadiumBrowserMainExtraPartsProfiles::
    EnsureBrowserContextKeyedServiceFactoriesBuilt() {
  ThemeServiceFactory::GetInstance();
}

void RadiumBrowserMainExtraPartsProfiles::PreProfileInit() {
  EnsureBrowserContextKeyedServiceFactoriesBuilt();
}
