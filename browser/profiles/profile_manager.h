// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_PROFILE_MANAGER_H_
#define RADIUM_BROWSER_PROFILES_PROFILE_MANAGER_H_

#include <map>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/callback_forward.h"

class Profile;

class ProfileManager {
 public:
  static std::unique_ptr<ProfileManager> Create();

  // Get the `Profile` last used (the `Profile` which owns the most recently
  // focused window) with this Chrome build. If no signed profile has been
  // stored in Local State, hand back the Default profile.
  // If the profile is going to be used to open a new window then consider using
  // `GetLastUsedProfileAllowedByPolicy()` instead.
  // Except in ChromeOS guest sessions, the returned profile is always a regular
  // profile (non-OffTheRecord).
  // WARNING: if the profile is not loaded, this function loads it
  // synchronously, causing blocking file I/O. Use
  // `GetLastUsedProfileIfLoaded()` to avoid loading the profile synchronously.
  static Profile* GetLastUsedProfile();

  ProfileManager(const ProfileManager&) = delete;
  ProfileManager& operator=(const ProfileManager&) = delete;

  ~ProfileManager();

  const base::FilePath& user_data_dir() const { return user_data_dir_; }

  // Get the path of the last used profile, or if that's undefined, the default
  // profile.
  base::FilePath GetLastUsedProfileDir();
  static base::FilePath GetLastUsedProfileBaseName();

  // Returns a profile for a specific profile directory within the user data
  // dir. This will return an existing profile it had already been created,
  // otherwise it will create and manage it.
  // Because this method might synchronously load a new profile, it should
  // only be called for the initial profile or in tests, where blocking is
  // acceptable. Returns nullptr if loading the new profile fails.
  Profile* GetProfile(const base::FilePath& profile_dir, bool create = false);

  // If a profile with the given path is currently managed by this object and
  // fully initialized, return a pointer to the corresponding Profile object;
  // otherwise return null.
  Profile* GetProfileByPath(const base::FilePath& path) const;

  // Sets the last-used profile to `last_active`, and also sets that profile's
  // last-active time to now. If the profile has a primary account, this also
  // sets its last-active time to now.
  // Public so that `ProfileManagerAndroid` can call it.
  void SetProfileAsLastUsed(Profile* last_active);

 protected:
  // Creates a new profile by calling into the profile's profile creation
  // method.
  std::unique_ptr<Profile> CreateProfileHelper(const base::FilePath& path);

  // Creates a new profile asynchronously by calling into the profile's
  // asynchronous profile creation method.
  std::unique_ptr<Profile> CreateProfileAsyncHelper(const base::FilePath& path);

 private:
  explicit ProfileManager(const base::FilePath& user_data_dir);

  // Synchronously creates and returns a profile. This handles both the full
  // creation and adds it to the set managed by this ProfileManager. Returns
  // null if creation fails.
  Profile* CreateAndInitializeProfile(
      const base::FilePath& profile_dir,
      base::OnceCallback<std::unique_ptr<Profile>(const base::FilePath&)>
          factory);

  // The path to the user data directory (DIR_USER_DATA).
  const base::FilePath user_data_dir_;

  // Maps profile path to `ProfileInfo` (if profile has been loaded). Use
  // `RegisterProfile()` to add into this map. This map owns all loaded profile
  // objects in a running instance of Chrome.
  using ProfilesInfoMap = std::map<base::FilePath, std::unique_ptr<Profile>>;
  ProfilesInfoMap profiles_info_;
};

#endif  // RADIUM_BROWSER_PROFILES_PROFILE_MANAGER_H_
