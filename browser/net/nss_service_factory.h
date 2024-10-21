// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_NET_NSS_SERVICE_FACTORY_H_
#define RADIUM_BROWSER_NET_NSS_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "radium/browser/profiles/profile_keyed_service_factory.h"

namespace content {
class BrowserContext;
}  // namespace content

class NssService;

class NssServiceFactory : public ProfileKeyedServiceFactory {
 public:
  // Returns the NssService for `context`.
  static NssService* GetForContext(content::BrowserContext* context);
  static NssServiceFactory* GetInstance();

 private:
  friend base::NoDestructor<NssServiceFactory>;

  NssServiceFactory();
  ~NssServiceFactory() override;

  // `BrowserContextKeyedServiceFactory` implementation:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

#endif  // RADIUM_BROWSER_NET_NSS_SERVICE_FACTORY_H_
