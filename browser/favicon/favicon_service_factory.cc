// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/favicon/favicon_service_factory.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "components/favicon/core/favicon_service_impl.h"
#include "components/history/core/browser/history_service.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/favicon/chrome_favicon_client.h"
#include "radium/browser/history/history_service_factory.h"
#include "radium/browser/profiles/profile.h"

namespace {

std::unique_ptr<KeyedService> BuildFaviconService(
    content::BrowserContext* context) {
  Profile* profile = Profile::FromBrowserContext(context);
  history::HistoryService* history_service =
      HistoryServiceFactory::GetForProfile(profile,
                                           ServiceAccessType::EXPLICIT_ACCESS);
  // |history_service| may be null, most likely because initialization failed.
  if (!history_service) {
    // This is rare enough that it's worth logging.
    LOG(WARNING) << "FaviconService not created as HistoryService is null";
    return nullptr;
  }
  return std::make_unique<favicon::FaviconServiceImpl>(
      std::make_unique<ChromeFaviconClient>(profile), history_service);
}

}  // namespace

// static
favicon::FaviconService* FaviconServiceFactory::GetForProfile(
    Profile* profile,
    ServiceAccessType sat) {
  if (!profile->IsOffTheRecord()) {
    return static_cast<favicon::FaviconService*>(
        GetInstance()->GetServiceForBrowserContext(profile, true));
  } else if (sat == ServiceAccessType::EXPLICIT_ACCESS) {
    // Profile must be OffTheRecord in this case.
    return static_cast<favicon::FaviconService*>(
        GetInstance()->GetServiceForBrowserContext(
            profile->GetOriginalProfile(), true));
  }

  // Profile is OffTheRecord without access.
  NOTREACHED() << "This profile is OffTheRecord";
}

// static
FaviconServiceFactory* FaviconServiceFactory::GetInstance() {
  static base::NoDestructor<FaviconServiceFactory> instance;
  return instance.get();
}

// static
BrowserContextKeyedServiceFactory::TestingFactory
FaviconServiceFactory::GetDefaultFactory() {
  return base::BindRepeating(&BuildFaviconService);
}

FaviconServiceFactory::FaviconServiceFactory()
    : ProfileKeyedServiceFactory(
          "FaviconService",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOriginalOnly)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kOriginalOnly)
              // TODO(crbug.com/41488885): Check if this service is needed for
              // Ash Internals.
              .WithAshInternals(ProfileSelection::kOriginalOnly)
              .Build()) {
  DependsOn(HistoryServiceFactory::GetInstance());
}

FaviconServiceFactory::~FaviconServiceFactory() = default;

std::unique_ptr<KeyedService>
FaviconServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return BuildFaviconService(context);
}

bool FaviconServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}
