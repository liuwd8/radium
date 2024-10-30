// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_prefs.h"

#include "components/policy/core/common/policy_pref_names.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/net/profile_network_context_service.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/profiles/profiles_state.h"
#include "radium/browser/ssl/ssl_config_service_manager.h"
#include "radium/common/pref_names.h"

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC)
#include "components/os_crypt/sync/os_crypt.h"  // nogncheck
#endif

void RegisterLocalState(PrefRegistrySimple* registry) {
  BrowserProcess::RegisterPrefs(registry);
  PrefProxyConfigTrackerImpl::RegisterPrefs(registry);
  ProfileNetworkContextService::RegisterLocalStatePrefs(registry);
  profiles::RegisterPrefs(registry);
  SSLConfigServiceManager::RegisterPrefs(registry);
  SystemNetworkContextManager::RegisterPrefs(registry);

  registry->RegisterFilePathPref(prefs::kDiskCacheDir, base::FilePath());
  registry->RegisterIntegerPref(prefs::kDiskCacheSize, 0);

#if BUILDFLAG(IS_WIN)
  OSCrypt::RegisterLocalPrefs(registry);
  registry->RegisterBooleanPref(
      policy::policy_prefs::kNativeWindowOcclusionEnabled, true);
#endif
}
