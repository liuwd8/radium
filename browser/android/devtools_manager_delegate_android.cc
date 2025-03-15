// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/android/devtools_manager_delegate_android.h"

#include <map>
#include <memory>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_agent_host_client.h"
#include "content/public/browser/devtools_external_agent_proxy.h"
#include "content/public/browser/devtools_external_agent_proxy_delegate.h"
#include "content/public/browser/web_contents.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/profiles/profile_manager.h"
#include "ui/base/resource/resource_bundle.h"

using content::DevToolsAgentHost;
using content::WebContents;

namespace {

// class ClientProxy : public content::DevToolsAgentHostClient {
//  public:
//   explicit ClientProxy(content::DevToolsExternalAgentProxy* proxy)
//       : proxy_(proxy) {}

//   ClientProxy(const ClientProxy&) = delete;
//   ClientProxy& operator=(const ClientProxy&) = delete;

//   ~ClientProxy() override {}

//   void DispatchProtocolMessage(DevToolsAgentHost* agent_host,
//                                base::span<const uint8_t> message) override {
//     proxy_->DispatchOnClientHost(message);
//   }

//   void AgentHostClosed(DevToolsAgentHost* agent_host) override {
//     proxy_->ConnectionClosed();
//   }

//  private:
//   raw_ptr<content::DevToolsExternalAgentProxy> proxy_;
// };

// static const void* const kCreatedByDevTools = &kCreatedByDevTools;

// bool IsCreatedByDevTools(const WebContents& web_contents) {
//   return !!web_contents.GetUserData(kCreatedByDevTools);
// }

// void MarkCreatedByDevTools(WebContents& web_contents) {
//   DCHECK(!IsCreatedByDevTools(web_contents));
//   web_contents.SetUserData(kCreatedByDevTools,
//                            std::make_unique<base::SupportsUserData::Data>());
// }

}  //  namespace

DevToolsManagerDelegateAndroid::DevToolsManagerDelegateAndroid() = default;

DevToolsManagerDelegateAndroid::~DevToolsManagerDelegateAndroid() = default;

content::BrowserContext*
DevToolsManagerDelegateAndroid::GetDefaultBrowserContext() {
  return ProfileManager::GetLastUsedProfile()->GetOriginalProfile();
}

std::string DevToolsManagerDelegateAndroid::GetTargetType(
    content::WebContents* web_contents) {
  return DevToolsAgentHost::kTypePage;
}

DevToolsAgentHost::List DevToolsManagerDelegateAndroid::RemoteDebuggingTargets(
    DevToolsManagerDelegate::TargetType target_type) {
  return DevToolsAgentHost::GetOrCreateAll();
}

scoped_refptr<DevToolsAgentHost>
DevToolsManagerDelegateAndroid::CreateNewTarget(const GURL& url,
                                                TargetType target_type,
                                                bool new_window) {
  return nullptr;
}

bool DevToolsManagerDelegateAndroid::IsBrowserTargetDiscoverable() {
  return true;
}
