// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_H_
#define RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_H_

#include <memory>

#include "content/public/browser/content_browser_client.h"

class RadiumFeatureListCreator;

class RadiumContentBrowserClient : public content::ContentBrowserClient {
 public:
  explicit RadiumContentBrowserClient();
  RadiumContentBrowserClient(const RadiumContentBrowserClient&) = delete;
  RadiumContentBrowserClient& operator=(const RadiumContentBrowserClient&) =
      delete;

  ~RadiumContentBrowserClient() override;

  RadiumFeatureListCreator* radium_feature_list_creator() const;

 private:
  // content::ContentBrowserClient
  std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(
      bool is_integration_test) override;
  void GetAdditionalWebUISchemes(
      std::vector<std::string>* additional_schemes) override;
  std::unique_ptr<content::DevToolsManagerDelegate>
  CreateDevToolsManagerDelegate() override;
  void RegisterBrowserInterfaceBindersForFrame(
      content::RenderFrameHost* render_frame_host,
      mojo::BinderMapWithContext<content::RenderFrameHost*>* map) override;
  void RegisterWebUIInterfaceBrokers(
      content::WebUIBrowserInterfaceBrokerRegistry& registry) override;
  void OnNetworkServiceCreated(
      network::mojom::NetworkService* network_service) override;
  void ConfigureNetworkContextParams(
      content::BrowserContext* context,
      bool in_memory,
      const base::FilePath& relative_partition_path,
      network::mojom::NetworkContextParams* network_context_params,
      cert_verifier::mojom::CertVerifierCreationParams*
          cert_verifier_creation_params) override;
#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)
  void GetAdditionalMappedFilesForChildProcess(
      const base::CommandLine& command_line,
      int child_process_id,
      content::PosixFileDescriptorInfo* mappings) override;
#endif  // BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)
  std::string GetProduct() override;
  std::string GetUserAgent() override;

 private:
  // Initializes `network_contexts_parent_directory_` and
  // `safe_browsing_service_` on the UI thread.
  void InitOnUIThread();

  std::unique_ptr<RadiumFeatureListCreator> radium_feature_list_creator_;

  // Returned from GetNetworkContextsParentDirectory() but created on the UI
  // thread because it needs to access the Local State prefs.
  std::vector<base::FilePath> network_contexts_parent_directory_;

  base::WeakPtrFactory<RadiumContentBrowserClient> weak_factory_{this};
};

#endif  // RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_H_
