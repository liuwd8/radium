// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/badging/badge_manager_factory.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/memory/ptr_util.h"
#include "base/no_destructor.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/badging/badge_manager.h"
#include "radium/browser/profiles/profile.h"

namespace badging {

// static
BadgeManager* BadgeManagerFactory::GetForProfile(Profile* profile) {
  return static_cast<badging::BadgeManager*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
BadgeManagerFactory* BadgeManagerFactory::GetInstance() {
  static base::NoDestructor<BadgeManagerFactory> instance;
  return instance.get();
}

BadgeManagerFactory::BadgeManagerFactory()
    : ProfileKeyedServiceFactory(
          "BadgeManager",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOriginalOnly)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kOriginalOnly)
              // TODO(crbug.com/41488885): Check if this service is needed for
              // Ash Internals.
              .WithAshInternals(ProfileSelection::kOriginalOnly)
              .Build()) {
  // DependsOn(web_app::WebAppProviderFactory::GetInstance());
}

BadgeManagerFactory::~BadgeManagerFactory() = default;

std::unique_ptr<KeyedService>
BadgeManagerFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<BadgeManager>(Profile::FromBrowserContext(context));
}

}  // namespace badging
