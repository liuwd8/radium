// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_content_browser_client.h"

#include <memory>

#include "base/path_service.h"
#include "base/version_info/version_info_values.h"
#include "build/build_config.h"
#include "components/embedder_support/user_agent_utils.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/devtools/radium_devtools_manager_delegate.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/browser/net/profile_network_context_service.h"
#include "radium/browser/net/profile_network_context_service_factory.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/profiles/radium_browser_main_extra_parts_profiles.h"
#include "radium/browser/radium_browser_main_extra_parts.h"
#include "radium/browser/radium_browser_main_parts.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views.h"
#include "radium/common/pref_names.h"
#include "radium/common/radium_paths.h"
#include "radium/common/radium_paths_internal.h"

#if BUILDFLAG(IS_WIN)
#include "radium/browser/radium_browser_main_parts_win.h"
#elif BUILDFLAG(IS_LINUX)
#include "radium/browser/radium_browser_main_parts_linux.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views_linux.h"
#elif BUILDFLAG(IS_MAC)
#include "radium/browser/radium_browser_main_extra_parts_mac.h"
#include "radium/browser/radium_browser_main_parts_mac.h"
#endif

#if BUILDFLAG(IS_LINUX)
#include "radium/browser/radium_browser_main_extra_parts_linux.h"
#elif BUILDFLAG(IS_OZONE)
#include "radium/browser/radium_browser_main_extra_parts_ozone.h"
#endif

namespace {

// A small ChromeBrowserMainExtraParts that invokes a callback when threads are
// ready. Used to initialize ChromeContentBrowserClient data that needs the UI
// thread.
class RadiumBrowserMainExtraPartsThreadNotifier final
    : public RadiumBrowserMainExtraParts {
 public:
  explicit RadiumBrowserMainExtraPartsThreadNotifier(
      base::OnceClosure threads_ready_closure)
      : threads_ready_closure_(std::move(threads_ready_closure)) {}

  // RadiumBrowserMainExtraParts:
  void PostCreateThreads() final { std::move(threads_ready_closure_).Run(); }

 private:
  base::OnceClosure threads_ready_closure_;
};

}  // namespace

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
#if BUILDFLAG(IS_WIN)
      std::make_unique<RadiumBrowserMainPartsWin>(
          radium_feature_list_creator_.get());
#elif BUILDFLAG(IS_LINUX)
      std::make_unique<RadiumBrowserMainPartsLinux>(
          radium_feature_list_creator_.get());
#elif BUILDFLAG(IS_MAC)
      std::make_unique<RadiumBrowserMainPartsMac>(
          radium_feature_list_creator_.get());
#else
#error "Unimplemented platform"
#endif

  main_parts->AddParts(
      std::make_unique<RadiumBrowserMainExtraPartsThreadNotifier>(
          base::BindOnce(&RadiumContentBrowserClient::InitOnUIThread,
                         weak_factory_.GetWeakPtr())));

  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsProfiles>());

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
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsViews>());
#endif
#endif

#if BUILDFLAG(IS_MAC)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsMac>());
#endif

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

void RadiumContentBrowserClient::InitOnUIThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // Initialize `network_contexts_parent_directory_`.
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  DCHECK(!user_data_dir.empty());
  network_contexts_parent_directory_.push_back(user_data_dir);

  base::FilePath cache_dir;
  radium::GetUserCacheDirectory(user_data_dir, &cache_dir);
  DCHECK(!cache_dir.empty());
  // On some platforms, the cache is a child of the user_data_dir so only
  // return the one path.
  if (!user_data_dir.IsParent(cache_dir)) {
    network_contexts_parent_directory_.push_back(cache_dir);
  }

  // If the cache location has been overridden by a switch or preference,
  // include that as well.
  if (auto* local_state = BrowserProcess::Get()->local_state()) {
    base::FilePath pref_cache_dir =
        local_state->GetFilePath(prefs::kDiskCacheDir);
    if (!pref_cache_dir.empty() && !user_data_dir.IsParent(cache_dir)) {
      network_contexts_parent_directory_.push_back(pref_cache_dir);
    }
  }
}
