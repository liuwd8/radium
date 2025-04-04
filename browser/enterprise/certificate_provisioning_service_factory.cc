// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/enterprise/certificate_provisioning_service_factory.h"

#include <memory>
#include <utility>

#include "base/memory/scoped_refptr.h"
#include "base/no_destructor.h"
#include "components/enterprise/browser/identifiers/profile_id_service.h"
#include "components/enterprise/client_certificates/core/certificate_provisioning_service.h"
#include "components/enterprise/client_certificates/core/certificate_store.h"
#include "components/enterprise/client_certificates/core/cloud_management_delegate.h"
#include "components/enterprise/client_certificates/core/dm_server_client.h"
#include "components/enterprise/client_certificates/core/features.h"
#include "components/enterprise/client_certificates/core/key_upload_client.h"
#include "components/enterprise/client_certificates/core/profile_cloud_management_delegate.h"
#include "components/enterprise/core/dependency_factory.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/policy/core/browser/browser_policy_connector.h"
#include "components/policy/core/common/cloud/device_management_service.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/net/profile_network_context_service_factory.h"
#include "radium/browser/policy/radium_browser_policy_connector.h"
#include "radium/browser/profiles/profile.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace client_certificates {

namespace {

ProfileSelections BuildCertificateProvisioningProfileSelections() {
  if (!features::IsManagedClientCertificateForUserEnabled()) {
    return ProfileSelections::BuildNoProfilesSelected();
  }

  return ProfileSelections::BuildForRegularProfile();
}

policy::DeviceManagementService* GetDeviceManagementService() {
  policy::BrowserPolicyConnector* connector =
      g_browser_process->browser_policy_connector();
  return connector ? connector->device_management_service() : nullptr;
}

}  // namespace

// static
CertificateProvisioningServiceFactory*
CertificateProvisioningServiceFactory::GetInstance() {
  static base::NoDestructor<CertificateProvisioningServiceFactory> instance;
  return instance.get();
}

// static
CertificateProvisioningService*
CertificateProvisioningServiceFactory::GetForProfile(Profile* profile) {
  return static_cast<CertificateProvisioningService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

CertificateProvisioningServiceFactory::CertificateProvisioningServiceFactory()
    : ProfileKeyedServiceFactory(
          "CertificateProvisioningService",
          BuildCertificateProvisioningProfileSelections()) {
  DependsOn(CertificateStoreFactory::GetInstance());
  DependsOn(enterprise::ProfileIdServiceFactory::GetInstance());
  DependsOn(ProfileNetworkContextServiceFactory::GetInstance());
}

CertificateProvisioningServiceFactory::
    ~CertificateProvisioningServiceFactory() = default;

bool CertificateProvisioningServiceFactory::ServiceIsCreatedWithBrowserContext()
    const {
  return true;
}

bool CertificateProvisioningServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}

std::unique_ptr<KeyedService>
CertificateProvisioningServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  auto* profile = Profile::FromBrowserContext(context);
  if (!profile) {
    return nullptr;
  }

  auto* certificate_store = CertificateStoreFactory::GetForProfile(profile);
  auto url_loader_factory = profile->GetURLLoaderFactory();
  auto* device_management_service = GetDeviceManagementService();
  auto* profile_id_service =
      enterprise::ProfileIdServiceFactory::GetForProfile(profile);
  auto* profile_network_context_service =
      ProfileNetworkContextServiceFactory::GetForContext(context);
  if (!certificate_store || !url_loader_factory || !device_management_service ||
      !profile_id_service || !profile_network_context_service) {
    return nullptr;
  }

  return CertificateProvisioningService::Create(
      profile->GetPrefs(), certificate_store,
      std::make_unique<ProfileContextDelegate>(profile_network_context_service),
      KeyUploadClient::Create(
          std::make_unique<
              enterprise_attestation::ProfileCloudManagementDelegate>(
              std::make_unique<enterprise_core::DependencyFactoryImpl>(profile),
              profile_id_service,
              enterprise_attestation::DMServerClient::Create(
                  device_management_service, std::move(url_loader_factory)))));
}

}  // namespace client_certificates
