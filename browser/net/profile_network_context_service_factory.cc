// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/net/profile_network_context_service_factory.h"

#include "build/build_config.h"
#include "crypto/crypto_buildflags.h"
#include "radium/browser/net/profile_network_context_service.h"
#include "radium/browser/profiles/profile.h"

#if BUILDFLAG(USE_NSS_CERTS)
#include "radium/browser/net/nss_service_factory.h"
#endif

ProfileNetworkContextService*
ProfileNetworkContextServiceFactory::GetForContext(
    content::BrowserContext* browser_context) {
  return static_cast<ProfileNetworkContextService*>(
      GetInstance()->GetServiceForBrowserContext(browser_context, true));
}

ProfileNetworkContextServiceFactory*
ProfileNetworkContextServiceFactory::GetInstance() {
  static base::NoDestructor<ProfileNetworkContextServiceFactory> instance;
  return instance.get();
}

ProfileNetworkContextServiceFactory::ProfileNetworkContextServiceFactory()
    : ProfileKeyedServiceFactory(
          "ProfileNetworkContextService",
          // Create separate service for incognito profiles.
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOwnInstance)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kOwnInstance)
              // TODO(crbug.com/41488885): Check if this service is needed for
              // Ash Internals.
              .WithAshInternals(ProfileSelection::kOwnInstance)
              .Build()) {
#if BUILDFLAG(USE_NSS_CERTS)
  // On platforms that use NSS, NSS should be initialized when a
  // ProfileNetworkContextService is created to ensure that NSS trust anchors
  // are available and NSS can be used to enumerate client certificates if
  // requested.
  DependsOn(NssServiceFactory::GetInstance());
#endif
}

ProfileNetworkContextServiceFactory::~ProfileNetworkContextServiceFactory() =
    default;

std::unique_ptr<KeyedService>
ProfileNetworkContextServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* profile) const {
  return std::make_unique<ProfileNetworkContextService>(
      Profile::FromBrowserContext(profile));
}

bool ProfileNetworkContextServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}
