// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_FAVICON_FAVICON_SERVICE_FACTORY_H_
#define RADIUM_BROWSER_FAVICON_FAVICON_SERVICE_FACTORY_H_

#include "components/keyed_service/core/service_access_type.h"
#include "radium/browser/profiles/profile_keyed_service_factory.h"

namespace base {
template <typename T>
class NoDestructor;
}

class Profile;

namespace favicon {
class FaviconService;
}

// Singleton that owns all FaviconService and associates them with
// Profiles.
class FaviconServiceFactory : public ProfileKeyedServiceFactory {
 public:
  // |sat| defines what the caller plans to do with the service. See
  // the ServiceAccessType definition in profile.h.
  static favicon::FaviconService* GetForProfile(Profile* profile,
                                                ServiceAccessType sat);

  // Returns the FaviconServiceFactory singleton.
  static FaviconServiceFactory* GetInstance();

  FaviconServiceFactory(const FaviconServiceFactory&) = delete;
  FaviconServiceFactory& operator=(const FaviconServiceFactory&) = delete;

  // Returns the default factory used to build FaviconService. Can be registered
  // with SetTestingFactory to use the FaviconService instance during testing.
  static TestingFactory GetDefaultFactory();

 private:
  friend base::NoDestructor<FaviconServiceFactory>;

  FaviconServiceFactory();
  ~FaviconServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* profile) const override;
  bool ServiceIsNULLWhileTesting() const override;
};

#endif  // RADIUM_BROWSER_FAVICON_FAVICON_SERVICE_FACTORY_H_
