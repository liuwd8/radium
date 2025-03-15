// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/devtools/radium_devtools_manager_delegate.h"

#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "content/public/browser/devtools_agent_host.h"
#include "radium/browser/devtools/devtools_browser_context_manager.h"
#include "radium/browser/devtools/radium_devtools_session.h"
#include "ui/views/controls/webview/webview.h"

namespace {
RadiumDevToolsManagerDelegate* g_instance = nullptr;
}

// static
RadiumDevToolsManagerDelegate* RadiumDevToolsManagerDelegate::GetInstance() {
  return g_instance;
}

// static
void RadiumDevToolsManagerDelegate::AllowBrowserToClose() {
  if (auto* instance = GetInstance()) {
    instance->keep_alive_.reset();
  }
}

RadiumDevToolsManagerDelegate::RadiumDevToolsManagerDelegate() {
  DCHECK(!g_instance);
  g_instance = this;
}

RadiumDevToolsManagerDelegate::~RadiumDevToolsManagerDelegate() {
  DCHECK(g_instance == this);
  g_instance = nullptr;
}

void RadiumDevToolsManagerDelegate::Inspect(
    content::DevToolsAgentHost* agent_host) {}

void RadiumDevToolsManagerDelegate::Activate(
    content::DevToolsAgentHost* agent_host) {
  auto* web_contents = agent_host->GetWebContents();
  if (!web_contents) {
    return;
  }

  // Brings the tab to foreground. We need to do this in case the devtools
  // window is undocked and this is being called from another tab that is in
  // the foreground.
  web_contents->GetDelegate()->ActivateContents(web_contents);
}

void RadiumDevToolsManagerDelegate::HandleCommand(
    content::DevToolsAgentHostClientChannel* channel,
    base::span<const uint8_t> message,
    NotHandledCallback callback) {
  auto it = sessions_.find(channel);
  if (it == sessions_.end()) {
    std::move(callback).Run(message);
    // This should not happen, but happens. NOTREACHED tries to get
    // a repro in some test.
    NOTREACHED();
  }
  it->second->HandleCommand(message, std::move(callback));
}

std::string RadiumDevToolsManagerDelegate::GetTargetType(
    content::WebContents* web_contents) {
  // if (base::Contains(AllTabContentses(), web_contents)) {
  //   return content::DevToolsAgentHost::kTypePage;
  // }

  if (views::WebView::IsWebViewContents(web_contents)) {
    return content::DevToolsAgentHost::kTypePage;
  }

  return content::DevToolsAgentHost::kTypeOther;
}

std::vector<content::BrowserContext*>
RadiumDevToolsManagerDelegate::GetBrowserContexts() {
  return DevToolsBrowserContextManager::GetInstance().GetBrowserContexts();
}

content::BrowserContext*
RadiumDevToolsManagerDelegate::GetDefaultBrowserContext() {
  return DevToolsBrowserContextManager::GetInstance()
      .GetDefaultBrowserContext();
}

content::BrowserContext* RadiumDevToolsManagerDelegate::CreateBrowserContext() {
  return nullptr;
}

void RadiumDevToolsManagerDelegate::DisposeBrowserContext(
    content::BrowserContext*,
    DisposeCallback callback) {}

bool RadiumDevToolsManagerDelegate::AllowInspectingRenderFrameHost(
    content::RenderFrameHost* rfh) {
  return true;
}

void RadiumDevToolsManagerDelegate::ClientAttached(
    content::DevToolsAgentHostClientChannel* channel) {
  DCHECK(sessions_.find(channel) == sessions_.end());
  sessions_.emplace(channel, std::make_unique<RadiumDevToolsSession>(channel));
}

void RadiumDevToolsManagerDelegate::ClientDetached(
    content::DevToolsAgentHostClientChannel* channel) {
  sessions_.erase(channel);
}

scoped_refptr<content::DevToolsAgentHost>
RadiumDevToolsManagerDelegate::CreateNewTarget(const GURL& url,
                                               TargetType target_type,
                                               bool new_window) {
  return nullptr;
}

bool RadiumDevToolsManagerDelegate::HasBundledFrontendResources() {
  return false;
}
