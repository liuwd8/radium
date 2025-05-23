// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_ENTERPRISE_CERTIFICATE_PROVISIONING_SERVICE_FACTORY_H_
#define RADIUM_BROWSER_ENTERPRISE_CERTIFICATE_PROVISIONING_SERVICE_FACTORY_H_

#include "radium/browser/profiles/profile_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
class NoDestructor;
}

namespace client_certificates {

class CertificateProvisioningService;

class CertificateProvisioningServiceFactory
    : public ProfileKeyedServiceFactory {
 public:
  static CertificateProvisioningServiceFactory* GetInstance();
  static CertificateProvisioningService* GetForProfile(Profile* profile);

 protected:
  bool ServiceIsCreatedWithBrowserContext() const override;
  bool ServiceIsNULLWhileTesting() const override;

 private:
  friend base::NoDestructor<CertificateProvisioningServiceFactory>;

  CertificateProvisioningServiceFactory();
  ~CertificateProvisioningServiceFactory() override;

  // ProfileKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace client_certificates

#endif  // RADIUM_BROWSER_ENTERPRISE_CERTIFICATE_PROVISIONING_SERVICE_FACTORY_H_
