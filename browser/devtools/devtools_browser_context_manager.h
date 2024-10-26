// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_DEVTOOLS_DEVTOOLS_BROWSER_CONTEXT_MANAGER_H_
#define RADIUM_BROWSER_DEVTOOLS_DEVTOOLS_BROWSER_CONTEXT_MANAGER_H_

#include <vector>

namespace content {
class BrowserContext;
}

class DevToolsBrowserContextManager {
 public:
  static DevToolsBrowserContextManager& GetInstance();

  explicit DevToolsBrowserContextManager();
  DevToolsBrowserContextManager(const DevToolsBrowserContextManager&) = delete;
  DevToolsBrowserContextManager& operator=(
      const DevToolsBrowserContextManager&) = delete;

  ~DevToolsBrowserContextManager();

  std::vector<content::BrowserContext*> GetBrowserContexts();
  content::BrowserContext* GetDefaultBrowserContext();
  content::BrowserContext* CreateBrowserContext();
};

#endif  // RADIUM_BROWSER_DEVTOOLS_DEVTOOLS_BROWSER_CONTEXT_MANAGER_H_
