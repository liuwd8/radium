// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profile_keyed_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "radium/browser/profiles/profile.h"

ProfileKeyedServiceFactory::ProfileKeyedServiceFactory(const char* name)
    : ProfileKeyedServiceFactory(name, ProfileSelections::Builder().Build()) {}

ProfileKeyedServiceFactory::ProfileKeyedServiceFactory(
    const char* name,
    const ProfileSelections& profile_selections)
    : BrowserContextKeyedServiceFactory(
          name,
          BrowserContextDependencyManager::GetInstance()),
      profile_selections_(profile_selections) {}

ProfileKeyedServiceFactory::~ProfileKeyedServiceFactory() = default;

content::BrowserContext* ProfileKeyedServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  Profile* profile = Profile::FromBrowserContext(context);
  return profile_selections_.ApplyProfileSelection(profile);
}
