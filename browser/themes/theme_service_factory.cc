// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/themes/theme_service_factory.h"

#include "base/trace_event/trace_event.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/themes/theme_service.h"
#include "ui/color/system_theme.h"

// static
ThemeService* ThemeServiceFactory::GetForProfile(Profile* profile) {
  TRACE_EVENT0("loading", "ThemeServiceFactory::GetForProfile");
  return static_cast<ThemeService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
ThemeServiceFactory* ThemeServiceFactory::GetInstance() {
  static base::NoDestructor<ThemeServiceFactory> instance;
  return instance.get();
}

ThemeServiceFactory::ThemeServiceFactory()
    : ProfileKeyedServiceFactory(
          "ThemeService",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kRedirectedToOriginal)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kRedirectedToOriginal)
              // TODO(crbug.com/41488885): Check if this service is needed for
              // Ash Internals.
              .WithAshInternals(ProfileSelection::kRedirectedToOriginal)
              .Build()) {}

ThemeServiceFactory::~ThemeServiceFactory() = default;

std::unique_ptr<KeyedService>
ThemeServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* profile) const {
  return std::make_unique<ThemeService>(static_cast<Profile*>(profile));
}

bool ThemeServiceFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}
