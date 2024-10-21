// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/content_settings/host_content_settings_map_factory.h"

#include <utility>

#include "base/feature_list.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "components/content_settings/core/browser/content_settings_pref_provider.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/permissions/features.h"
#include "content/public/browser/browser_thread.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/profiles/profiles_state.h"
#include "ui/webui/webui_allowlist_provider.h"

#if BUILDFLAG(IS_ANDROID)
#include "radium/browser/flags/android/radium_feature_list.h"
#include "radium/browser/notifications/notification_channels_provider_android.h"
#include "radium/browser/webapps/installable/installed_webapp_provider.h"
#endif  // BUILDFLAG(IS_ANDROID)

using content_settings::ProviderType;

HostContentSettingsMapFactory::HostContentSettingsMapFactory()
    : RefcountedProfileKeyedServiceFactory(
          "HostContentSettingsMap",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOwnInstance)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kOwnInstance)
              // TODO(crbug.com/41488885): Check if this service is needed for
              // Ash Internals.
              .WithAshInternals(ProfileSelection::kOwnInstance)
              .Build()) {}

HostContentSettingsMapFactory::~HostContentSettingsMapFactory() = default;

// static
HostContentSettingsMap* HostContentSettingsMapFactory::GetForProfile(
    content::BrowserContext* browser_context) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  return static_cast<HostContentSettingsMap*>(
      GetInstance()->GetServiceForBrowserContext(browser_context, true).get());
}

// static
HostContentSettingsMapFactory* HostContentSettingsMapFactory::GetInstance() {
  static base::NoDestructor<HostContentSettingsMapFactory> instance;
  return instance.get();
}

scoped_refptr<RefcountedKeyedService>
HostContentSettingsMapFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  Profile* profile = static_cast<Profile*>(context);
  // extensions::ContentSettingsService::Get() needs the original profile.
  Profile* original_profile = profile->GetOriginalProfile();

  // In OffTheRecord mode, retrieve the host content settings map of the parent
  // profile in order to ensure the preferences have been migrated.
  // This is not required for guest sessions, since the parent profile of a
  // guest OTR profile is empty.
  if (profile->IsOffTheRecord() && !profile->IsGuestSession()) {
    GetForProfile(original_profile);
  }

  scoped_refptr<HostContentSettingsMap> settings_map(new HostContentSettingsMap(
      profile->GetPrefs(),
      profile->IsOffTheRecord() || profile->IsGuestSession(),
      /*store_last_modified=*/true, profile->ShouldRestoreOldSessionCookies(),
      profiles::IsRegularUserProfile(profile)));

  auto allowlist_provider = std::make_unique<WebUIAllowlistProvider>(
      WebUIAllowlist::GetOrCreate(profile));
  settings_map->RegisterProvider(ProviderType::kWebuiAllowlistProvider,
                                 std::move(allowlist_provider));

#if BUILDFLAG(IS_ANDROID)
  if (!profile->IsOffTheRecord()) {
    auto channels_provider =
        std::make_unique<NotificationChannelsProviderAndroid>(
            profile->GetPrefs());

    channels_provider->Initialize(
        settings_map->GetPrefProvider(),
        TemplateURLServiceFactory::GetForProfile(profile));

    settings_map->RegisterUserModifiableProvider(
        ProviderType::kNotificationAndroidProvider,
        std::move(channels_provider));

    auto webapp_provider = std::make_unique<InstalledWebappProvider>();
    settings_map->RegisterProvider(ProviderType::kInstalledWebappProvider,
                                   std::move(webapp_provider));
  }
#endif  // defined (OS_ANDROID)

  return settings_map;
}
