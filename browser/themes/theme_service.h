// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_THEMES_THEME_SERVICE_H_
#define RADIUM_BROWSER_THEMES_THEME_SERVICE_H_

#include "components/keyed_service/core/keyed_service.h"
#include "radium/browser/themes/radium_theme_provider.h"
#include "ui/base/theme_provider.h"

class CustomThemeSupplier;
class Profile;

namespace ui {
class ColorProvider;
}  // namespace ui

class ThemeService : public KeyedService {
 public:
  // Gets the ThemeProvider for |profile|. This will be different for an
  // incognito profile and its original profile, even though both profiles use
  // the same ThemeService.
  //
  // Before using this function, consider if the caller is in a rooted UI tree.
  // If it is, strongly favor referring to the conceptual roots for a
  // ThemeProvider. For Views, this is the Widget. For Cocoa, this is the
  // AppControl.
  static const ui::ThemeProvider& GetThemeProviderForProfile(Profile* profile);
  static CustomThemeSupplier* GetThemeSupplierForProfile(Profile* profile);

  explicit ThemeService(Profile* profile);
  ThemeService(const ThemeService&) = delete;
  ThemeService& operator=(const ThemeService&) = delete;

  ~ThemeService() override;

 private:
  RadiumThemeProvider theme_provider_;
};

#endif  // RADIUM_BROWSER_THEMES_THEME_SERVICE_H_
