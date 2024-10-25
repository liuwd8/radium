// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/themes/theme_service.h"

#include "radium/browser/profiles/profile.h"
#include "radium/browser/themes/theme_service_factory.h"

const ui::ThemeProvider& ThemeService::GetThemeProviderForProfile(
    Profile* profile) {
  ThemeService* service = ThemeServiceFactory::GetForProfile(profile);
  return service->theme_provider_;
}

CustomThemeSupplier* ThemeService::GetThemeSupplierForProfile(
    Profile* profile) {
  return nullptr;
}

ThemeService::ThemeService(Profile* profile)
    : theme_provider_(profile->IsIncognitoProfile()) {}

ThemeService::~ThemeService() = default;
