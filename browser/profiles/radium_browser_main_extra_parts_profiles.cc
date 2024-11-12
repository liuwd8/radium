// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/radium_browser_main_extra_parts_profiles.h"

#include "radium/browser/content_settings/cookie_settings_factory.h"
#include "radium/browser/content_settings/host_content_settings_map_factory.h"
#include "radium/browser/net/profile_network_context_service_factory.h"

#if !BUILDFLAG(IS_ANDROID)
#include "radium/browser/badging/badge_manager_factory.h"
#include "radium/browser/themes/theme_service_factory.h"
#endif

RadiumBrowserMainExtraPartsProfiles::RadiumBrowserMainExtraPartsProfiles() =
    default;
RadiumBrowserMainExtraPartsProfiles::~RadiumBrowserMainExtraPartsProfiles() =
    default;

void RadiumBrowserMainExtraPartsProfiles::
    EnsureBrowserContextKeyedServiceFactoriesBuilt() {
#if !BUILDFLAG(IS_ANDROID)
  badging::BadgeManagerFactory::GetInstance();
#endif
  CookieSettingsFactory::GetInstance();
  HostContentSettingsMapFactory::GetInstance();
  ProfileNetworkContextServiceFactory::GetInstance();
#if !BUILDFLAG(IS_ANDROID)
  ThemeServiceFactory::GetInstance();
#endif
}

void RadiumBrowserMainExtraPartsProfiles::PreProfileInit() {
  EnsureBrowserContextKeyedServiceFactoriesBuilt();
}
