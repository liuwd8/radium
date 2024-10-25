// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_RADIUM_BROWSER_MAIN_EXTRA_PARTS_PROFILES_H_
#define RADIUM_BROWSER_PROFILES_RADIUM_BROWSER_MAIN_EXTRA_PARTS_PROFILES_H_

#include "radium/browser/radium_browser_main_extra_parts.h"

class RadiumBrowserMainExtraPartsProfiles : public RadiumBrowserMainExtraParts {
 public:
  explicit RadiumBrowserMainExtraPartsProfiles();
  RadiumBrowserMainExtraPartsProfiles(
      const RadiumBrowserMainExtraPartsProfiles&) = delete;
  RadiumBrowserMainExtraPartsProfiles& operator=(
      const RadiumBrowserMainExtraPartsProfiles&) = delete;

  ~RadiumBrowserMainExtraPartsProfiles() override;

  // Instantiates all radium KeyedService factories, which is
  // especially important for services that should be created at profile
  // creation time as compared to lazily on first access.
  static void EnsureBrowserContextKeyedServiceFactoriesBuilt();

  // RadiumBrowserMainExtraPartsProfiles:
  void PreProfileInit() override;
};

#endif  // RADIUM_BROWSER_PROFILES_RADIUM_BROWSER_MAIN_EXTRA_PARTS_PROFILES_H_
