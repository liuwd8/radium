// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/prefs/profile_prefs.h"

#include <string>

#include "base/trace_event/trace_event.h"
#include "components/certificate_transparency/pref_names.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/language/core/browser/language_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "radium/browser/net/profile_network_context_service.h"
#include "radium/browser/ui/prefs/prefs_tab_helper.h"
#include "radium/common/pref_names.h"

namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry,
                          const std::string& locale) {
  TRACE_EVENT0("browser", "radium::RegisterProfilePrefs");
  certificate_transparency::prefs::RegisterPrefs(registry);
  content_settings::CookieSettings::RegisterProfilePrefs(registry);
  HostContentSettingsMap::RegisterProfilePrefs(registry);
  language::LanguagePrefs::RegisterProfilePrefs(registry);
  PrefProxyConfigTrackerImpl::RegisterProfilePrefs(registry);
  ProfileNetworkContextService::RegisterProfilePrefs(registry);
  PrefsTabHelper::RegisterProfilePrefs(registry, locale);

  registry->RegisterBooleanPref(prefs::kCorsNonWildcardRequestHeadersSupport,
                                true);
  registry->RegisterBooleanPref(prefs::kIPv6ReachabilityOverrideEnabled, false);
  registry->RegisterBooleanPref(
      prefs::kAccessControlAllowMethodsInCORSPreflightSpecConformant, true);
}

}  // namespace prefs
