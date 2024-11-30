// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_THEME_THEME_SERVICE_FACTORY_H_
#define RADIUM_BROWSER_THEME_THEME_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "radium/browser/profiles/profile_keyed_service_factory.h"

class Profile;
class ThemeService;

class ThemeServiceFactory : public ProfileKeyedServiceFactory {
 public:
  // Returns the ThemeService that provides theming resources for
  // |profile|. Note that even if a Profile doesn't have a theme installed, it
  // still needs a ThemeService to hand back the default theme images.
  static ThemeService* GetForProfile(Profile* profile);

  static ThemeServiceFactory* GetInstance();

  ThemeServiceFactory(const ThemeServiceFactory&) = delete;
  ThemeServiceFactory& operator=(const ThemeServiceFactory&) = delete;

 private:
  friend base::NoDestructor<ThemeServiceFactory>;

  explicit ThemeServiceFactory();
  ~ThemeServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
};

#endif  // RADIUM_BROWSER_THEME_THEME_SERVICE_FACTORY_H_
