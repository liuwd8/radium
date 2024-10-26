// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_content_browser_client.h"

#include <memory>

#include "base/version_info/version_info_values.h"
#include "build/build_config.h"
#include "components/embedder_support/user_agent_utils.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/devtools/radium_devtools_manager_delegate.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/browser/net/profile_network_context_service.h"
#include "radium/browser/net/profile_network_context_service_factory.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/profiles/radium_browser_main_extra_parts_profiles.h"
#include "radium/browser/radium_browser_main_parts.h"

#if BUILDFLAG(IS_LINUX)
#include "radium/browser/radium_browser_main_extra_parts_linux.h"
#include "radium/browser/radium_browser_main_parts_linux.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views_linux.h"
#elif BUILDFLAG(IS_OZONE)
#include "radium/browser/radium_browser_main_extra_parts_ozone.h"
#endif

RadiumContentBrowserClient::RadiumContentBrowserClient()
    : radium_feature_list_creator_(
          std::make_unique<RadiumFeatureListCreator>()) {}

RadiumContentBrowserClient::~RadiumContentBrowserClient() = default;

RadiumFeatureListCreator*
RadiumContentBrowserClient::radium_feature_list_creator() const {
  return radium_feature_list_creator_.get();
}

std::unique_ptr<content::BrowserMainParts>
RadiumContentBrowserClient::CreateBrowserMainParts(bool is_integration_test) {
  std::unique_ptr<RadiumBrowserMainParts> main_parts =
#if BUILDFLAG(IS_LINUX)
      std::make_unique<RadiumBrowserMainPartsLinux>(
          radium_feature_list_creator_.get());
#else
#error "Unimplemented platform"
#endif

  // Construct additional browser parts. Stages are called in the order in
  // which they are added.
#if defined(TOOLKIT_VIEWS)
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  main_parts->AddParts(
      std::make_unique<ChromeBrowserMainExtraPartsViewsLacros>());
// TODO(crbug.com/40118868): Revisit the macro expression once build flag switch
// of lacros-chrome is complete.
#elif BUILDFLAG(IS_LINUX)
  main_parts->AddParts(
      std::make_unique<RadiumBrowserMainExtraPartsViewsLinux>());
#else
  main_parts->AddParts(std::make_unique<ChromeBrowserMainExtraPartsViews>());
#endif
#endif

  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsProfiles>());

#if BUILDFLAG(IS_LINUX)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsLinux>());
#elif BUILDFLAG(IS_OZONE)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsOzone>());
#endif

  return main_parts;
}

std::unique_ptr<content::DevToolsManagerDelegate>
RadiumContentBrowserClient::CreateDevToolsManagerDelegate() {
  return std::make_unique<RadiumDevToolsManagerDelegate>();
}

void RadiumContentBrowserClient::OnNetworkServiceCreated(
    network::mojom::NetworkService* network_service) {
  PrefService* local_state;
  if (BrowserProcess::Get()) {
    local_state = BrowserProcess::Get()->local_state();
  } else {
    local_state = radium_feature_list_creator_->local_state();
  }
  DCHECK(local_state);

  // Create SystemNetworkContextManager if it has not been created yet. We need
  // to set up global NetworkService state before anything else uses it and this
  // is the first opportunity to initialize SystemNetworkContextManager with the
  // NetworkService.
  if (!SystemNetworkContextManager::HasInstance()) {
    SystemNetworkContextManager::CreateInstance(local_state);
  }

  SystemNetworkContextManager::GetInstance()->OnNetworkServiceCreated(
      network_service);
}

void RadiumContentBrowserClient::ConfigureNetworkContextParams(
    content::BrowserContext* context,
    bool in_memory,
    const base::FilePath& relative_partition_path,
    network::mojom::NetworkContextParams* network_context_params,
    cert_verifier::mojom::CertVerifierCreationParams*
        cert_verifier_creation_params) {
  ProfileNetworkContextService* service =
      ProfileNetworkContextServiceFactory::GetForContext(context);
  if (service) {
    service->ConfigureNetworkContextParams(in_memory, relative_partition_path,
                                           network_context_params,
                                           cert_verifier_creation_params);
  } else {
    // Set default params.
    network_context_params->user_agent = GetUserAgentBasedOnPolicy(context);
    network_context_params->accept_language = GetApplicationLocale();
  }
}

std::string RadiumContentBrowserClient::GetProduct() {
  return "Radium/" PRODUCT_VERSION;
}

std::string RadiumContentBrowserClient::GetUserAgent() {
  return embedder_support::GetUserAgent();
}
