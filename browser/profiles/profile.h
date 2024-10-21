// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_PROFILE_H_
#define RADIUM_BROWSER_PROFILES_PROFILE_H_

#include "content/public/browser/browser_context.h"

class PrefService;

namespace content {
class WebUI;
}

namespace sync_preferences {
class PrefServiceSyncable;
}

class Profile : public content::BrowserContext {
 public:
  // Returns the profile corresponding to the given browser context.
  static Profile* FromBrowserContext(content::BrowserContext* browser_context);

  // Returns the profile corresponding to the given WebUI.
  static Profile* FromWebUI(content::WebUI* web_ui);

  explicit Profile();
  Profile(const Profile&) = delete;
  Profile& operator=(const Profile&) = delete;

  ~Profile() override;

  // Retrieves a pointer to the PrefService that manages the
  // preferences for this user profile.
  PrefService* GetPrefs();
  const PrefService* GetPrefs() const;

  // IsRegularProfile(), IsSystemProfile(), IsIncognitoProfile(), and
  // IsGuestSession() are mutually exclusive.
  // Note: IsGuestSession() is not mutually exclusive with the rest of the
  // methods mentioned above on Ash. TODO(crbug.com/40233408).
  //
  // IsSystemProfile() returns true for both regular and off-the-record profile
  //   of the system profile.
  // IsOffTheRecord() is true for the off the record profile of Incognito mode,
  // system profile, Guest sessions, and also non-primary OffTheRecord profiles.

  // Returns whether it's a regular profile.
  bool IsRegularProfile() const;

  // Returns whether it is an Incognito profile. An Incognito profile is an
  // off-the-record profile that is used for incognito mode.
  bool IsIncognitoProfile() const;

  // Returns whether it is a Guest session. This covers both regular and
  // off-the-record profiles of a Guest session.
  bool IsGuestSession() const;

  // Returns whether it is a system profile.
  bool IsSystemProfile() const;

  // Return the original "recording" profile. This method returns this if the
  // profile is not OffTheRecord.
  Profile* GetOriginalProfile();

  // Return the original "recording" profile. This method returns this if the
  // profile is not OffTheRecord.
  const Profile* GetOriginalProfile() const;

  bool ShouldRestoreOldSessionCookies() const;
  bool ShouldPersistSessionCookies() const;

 private:
  std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs_;
};

#endif  // RADIUM_BROWSER_PROFILES_PROFILE_H_
