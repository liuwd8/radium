// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profile.h"

#include "components/profile_metrics/browser_profile_type.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"

// static
Profile* Profile::FromBrowserContext(content::BrowserContext* browser_context) {
  if (!browser_context) {
    return nullptr;
  }

  return static_cast<Profile*>(browser_context);
}
// static
Profile* Profile::FromWebUI(content::WebUI* web_ui) {
  return FromBrowserContext(web_ui->GetWebContents()->GetBrowserContext());
}

Profile::Profile() = default;
Profile::~Profile() = default;

PrefService* Profile::GetPrefs() {
  return const_cast<PrefService*>(
      static_cast<const Profile*>(this)->GetPrefs());
}

const PrefService* Profile::GetPrefs() const {
  DCHECK(prefs_);  // Should explicitly be initialized.
  return prefs_.get();
}

bool Profile::IsRegularProfile() const {
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kRegular;
}

bool Profile::IsIncognitoProfile() const {
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kIncognito;
}

bool Profile::IsGuestSession() const {
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kGuest;
}

bool Profile::IsSystemProfile() const {
#if BUILDFLAG(IS_ANDROID)
  DCHECK_NE(profile_metrics::GetBrowserProfileType(this),
            profile_metrics::BrowserProfileType::kSystem);
  return false;
#else  // BUILDFLAG(IS_ANDROID)
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kSystem;
#endif
}

Profile* Profile::GetOriginalProfile() {
  return this;
}

const Profile* Profile::GetOriginalProfile() const {
  return this;
}

bool Profile::ShouldRestoreOldSessionCookies() const {
  return true;
}

bool Profile::ShouldPersistSessionCookies() const {
  return true;
}
