// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_ANDROID_DEVTOOLS_MANAGER_DELEGATE_ANDROID_H_
#define RADIUM_BROWSER_ANDROID_DEVTOOLS_MANAGER_DELEGATE_ANDROID_H_

#include <memory>

#include "content/public/browser/devtools_agent_host_observer.h"
#include "content/public/browser/devtools_manager_delegate.h"

class DevToolsManagerDelegateAndroid : public content::DevToolsManagerDelegate {
 public:
  DevToolsManagerDelegateAndroid();

  DevToolsManagerDelegateAndroid(const DevToolsManagerDelegateAndroid&) =
      delete;
  DevToolsManagerDelegateAndroid& operator=(
      const DevToolsManagerDelegateAndroid&) = delete;

  ~DevToolsManagerDelegateAndroid() override;

 private:
  // content::DevToolsManagerDelegate implementation.
  content::BrowserContext* GetDefaultBrowserContext() override;
  std::string GetTargetType(content::WebContents* web_contents) override;
  content::DevToolsAgentHost::List RemoteDebuggingTargets(
      TargetType target_type) override;
  scoped_refptr<content::DevToolsAgentHost> CreateNewTarget(
      const GURL& url,
      TargetType target_type,
      bool new_window) override;
  bool IsBrowserTargetDiscoverable() override;
};

#endif  // RADIUM_BROWSER_ANDROID_DEVTOOLS_MANAGER_DELEGATE_ANDROID_H_
