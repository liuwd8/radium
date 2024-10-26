// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/devtools/devtools_browser_context_manager.h"

#include "base/no_destructor.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/profiles/profile_manager.h"

// static
DevToolsBrowserContextManager& DevToolsBrowserContextManager::GetInstance() {
  static base::NoDestructor<DevToolsBrowserContextManager> instance;
  return *instance;
}

DevToolsBrowserContextManager::DevToolsBrowserContextManager() = default;
DevToolsBrowserContextManager::~DevToolsBrowserContextManager() = default;

std::vector<content::BrowserContext*>
DevToolsBrowserContextManager::GetBrowserContexts() {
  return {};
}

content::BrowserContext*
DevToolsBrowserContextManager::GetDefaultBrowserContext() {
  return ProfileManager::GetLastUsedProfile()->GetOriginalProfile();
}

content::BrowserContext* DevToolsBrowserContextManager::CreateBrowserContext() {
  return nullptr;
}
