// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_prefs.h"

#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/ssl/ssl_config_service_manager.h"

void RegisterLocalState(PrefRegistrySimple* registry) {
  BrowserProcess::RegisterPrefs(registry);
  PrefProxyConfigTrackerImpl::RegisterPrefs(registry);
  SSLConfigServiceManager::RegisterPrefs(registry);
  SystemNetworkContextManager::RegisterPrefs(registry);
}
