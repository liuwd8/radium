// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/content_settings/cookie_settings_factory.h"

#include "base/check_op.h"
#include "base/functional/callback_forward.h"
#include "base/metrics/histogram_functions.h"
#include "components/content_settings/core/browser/cookie_settings.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_features.h"
#include "radium/browser/content_settings/host_content_settings_map_factory.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/profiles/profiles_state.h"
#include "third_party/blink/public/common/features_generated.h"

using content_settings::CookieControlsMode;

// static
scoped_refptr<content_settings::CookieSettings>
CookieSettingsFactory::GetForProfile(Profile* profile) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return static_cast<content_settings::CookieSettings*>(
      GetInstance()->GetServiceForBrowserContext(profile, true).get());
}

// static
CookieSettingsFactory* CookieSettingsFactory::GetInstance() {
  static base::NoDestructor<CookieSettingsFactory> instance;
  return instance.get();
}

CookieSettingsFactory::CookieSettingsFactory()
    : RefcountedProfileKeyedServiceFactory(
          "CookieSettings",
          // The incognito profile has its own content settings map. Therefore,
          // it should get its own CookieSettings.
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOwnInstance)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kOwnInstance)
              // TODO(crbug.com/41488885): Check if this service is needed for
              // Ash Internals.
              .WithAshInternals(ProfileSelection::kOwnInstance)
              .Build()) {
  DependsOn(HostContentSettingsMapFactory::GetInstance());
}

CookieSettingsFactory::~CookieSettingsFactory() = default;

scoped_refptr<RefcountedKeyedService>
CookieSettingsFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  Profile* profile = Profile::FromBrowserContext(context);
  PrefService* prefs = profile->GetPrefs();

  if (profiles::IsRegularUserProfile(profile)) {
    // Record cookie setting histograms.
    auto cookie_controls_mode = static_cast<CookieControlsMode>(
        prefs->GetInteger(prefs::kCookieControlsMode));
    base::UmaHistogramBoolean(
        "Privacy.ThirdPartyCookieBlockingSetting.RegularProfile",
        cookie_controls_mode == CookieControlsMode::kBlockThirdParty);
    base::UmaHistogramEnumeration(
        "Privacy.CookieControlsSetting.RegularProfile", cookie_controls_mode);
  }

  HostContentSettingsMap* host_content_settings_map =
      HostContentSettingsMapFactory::GetForProfile(profile);

  return new content_settings::CookieSettings(
      host_content_settings_map, prefs, nullptr, profile->IsIncognitoProfile(),
      content_settings::CookieSettings::NoFedCmSharingPermissionsCallback(),
      nullptr);
}
