// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_DEVTOOLS_RADIUM_DEVTOOLS_MANAGER_DELEGATE_H_
#define RADIUM_BROWSER_DEVTOOLS_RADIUM_DEVTOOLS_MANAGER_DELEGATE_H_

#include "content/public/browser/devtools_manager_delegate.h"

class RadiumDevToolsSession;
class ScopedKeepAlive;

class RadiumDevToolsManagerDelegate : public content::DevToolsManagerDelegate {
 public:
  explicit RadiumDevToolsManagerDelegate();
  RadiumDevToolsManagerDelegate(const RadiumDevToolsManagerDelegate&) = delete;
  RadiumDevToolsManagerDelegate& operator=(
      const RadiumDevToolsManagerDelegate&) = delete;

  ~RadiumDevToolsManagerDelegate() override;

  static RadiumDevToolsManagerDelegate* GetInstance();

  // Release browser keep alive allowing browser to close.
  static void AllowBrowserToClose();

 private:
  // content::DevToolsManagerDelegate implementation.
  void Inspect(content::DevToolsAgentHost* agent_host) override;
  void Activate(content::DevToolsAgentHost* agent_host) override;
  void HandleCommand(content::DevToolsAgentHostClientChannel* channel,
                     base::span<const uint8_t> message,
                     NotHandledCallback callback) override;
  std::string GetTargetType(content::WebContents* web_contents) override;

  std::vector<content::BrowserContext*> GetBrowserContexts() override;
  content::BrowserContext* GetDefaultBrowserContext() override;

  content::BrowserContext* CreateBrowserContext() override;
  void DisposeBrowserContext(content::BrowserContext*,
                             DisposeCallback callback) override;

  bool AllowInspectingRenderFrameHost(content::RenderFrameHost* rfh) override;
  void ClientAttached(
      content::DevToolsAgentHostClientChannel* channel) override;
  void ClientDetached(
      content::DevToolsAgentHostClientChannel* channel) override;
  scoped_refptr<content::DevToolsAgentHost> CreateNewTarget(
      const GURL& url,
      TargetType target_type,
      bool new_window) override;
  bool HasBundledFrontendResources() override;

 private:
  std::map<content::DevToolsAgentHostClientChannel*,
           std::unique_ptr<RadiumDevToolsSession>>
      sessions_;

  std::unique_ptr<ScopedKeepAlive> keep_alive_;
};

#endif  // RADIUM_BROWSER_DEVTOOLS_RADIUM_DEVTOOLS_MANAGER_DELEGATE_H_
