// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/net/default_dns_over_https_config_source.h"

#include <string>

#include "build/build_config.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/net/secure_dns_config.h"
#include "radium/common/pref_names.h"

DefaultDnsOverHttpsConfigSource::DefaultDnsOverHttpsConfigSource(
    PrefService* local_state,
    bool set_up_pref_defaults) {
  pref_change_registrar_.Init(local_state);

  // Update the DoH default preferences based on the corresponding
  // features before registering change callbacks for these preferences.
  // Changing prefs or defaults after registering change callbacks could
  // result in reentrancy and mess up registration between this code and
  // NetworkService creation.
  if (set_up_pref_defaults) {
    local_state->SetDefaultPrefValue(prefs::kDnsOverHttpsMode,
                                     base::Value(SecureDnsConfig::ModeToString(
                                         net::SecureDnsMode::kAutomatic)));
  }
}

DefaultDnsOverHttpsConfigSource::~DefaultDnsOverHttpsConfigSource() = default;

// static
void DefaultDnsOverHttpsConfigSource::RegisterPrefs(
    PrefRegistrySimple* registry) {
  registry->RegisterStringPref(prefs::kDnsOverHttpsMode, std::string());
  registry->RegisterStringPref(prefs::kDnsOverHttpsTemplates, std::string());
#if BUILDFLAG(IS_CHROMEOS)
  registry->RegisterStringPref(prefs::kDnsOverHttpsEffectiveTemplatesChromeOS,
                               std::string());
#endif
}

std::string DefaultDnsOverHttpsConfigSource::GetDnsOverHttpsMode() const {
  return pref_change_registrar_.prefs()->GetString(prefs::kDnsOverHttpsMode);
}

std::string DefaultDnsOverHttpsConfigSource::GetDnsOverHttpsTemplates() const {
  return pref_change_registrar_.prefs()->GetString(
      prefs::kDnsOverHttpsTemplates);
}

bool DefaultDnsOverHttpsConfigSource::IsConfigManaged() const {
  return pref_change_registrar_.prefs()
      ->FindPreference(prefs::kDnsOverHttpsMode)
      ->IsManaged();
}

void DefaultDnsOverHttpsConfigSource::SetDohChangeCallback(
    base::RepeatingClosure callback) {
  CHECK(pref_change_registrar_.IsEmpty());
  pref_change_registrar_.Add(prefs::kDnsOverHttpsMode, callback);
  pref_change_registrar_.Add(prefs::kDnsOverHttpsTemplates, callback);
}
