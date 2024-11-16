// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_PROFILE_MANAGER_H_
#define RADIUM_BROWSER_PROFILES_PROFILE_MANAGER_H_

#include <map>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "radium/browser/profiles/profile.h"

class ProfileManager : public Profile::Delegate {
 public:
  using ProfileLoadedCallback = base::OnceCallback<void(Profile*)>;
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

  ~ProfileManager() override;

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

  // Creates or loads the profile located at |profile_path|.
  // Should be called on the UI thread.
  // Params:
  // - `initialized_callback`: called when profile initialization is done, will
  // return nullptr if failed. If the profile has already been fully loaded then
  // this callback is called immediately.
  // - `created_callback`: called when profile creation is done (default
  // implementation to do nothing).
  // Note: Refer to `Profile::CreateStatus` for the definition of CREATED and
  // INITIALIZED profile creation status.
  void CreateProfileAsync(
      const base::FilePath& profile_path,
      base::OnceCallback<void(Profile*)> initialized_callback,
      base::OnceCallback<void(Profile*)> created_callback = {});

  // Returns loaded and fully initialized profiles. Notes:
  // - profiles order is NOT guaranteed to be related with the creation order.
  // - only returns profiles owned by the ProfileManager. In particular, this
  //   does not return incognito profiles, because they are owned by their
  //   original profiles.
  // - may also return irregular profiles like on-the-record System or Guest
  //   profiles.
  std::vector<Profile*> GetLoadedProfiles() const;

  // If a profile with the given path is currently managed by this object and
  // fully initialized, return a pointer to the corresponding Profile object;
  // otherwise return null.
  Profile* GetProfileByPath(const base::FilePath& path) const;

  // Returns whether |path| is allowed for profile creation.
  bool IsAllowedProfilePath(const base::FilePath& path) const;

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
  // This class contains information about profiles which are being loaded or
  // were loaded.
  struct ProfileInfo {
    ProfileInfo();
    ~ProfileInfo();

    // For when the Profile is owned, via FromOwnedProfile() or
    // TakeOwnershipOfProfile().
    std::unique_ptr<Profile> profile;

    // List of callbacks to run when profile initialization (success or fail) is
    // done. Note, when profile is fully loaded this vector will be empty.
    std::vector<ProfileLoadedCallback> init_callbacks;
    // List of callbacks to run when profile is created. Note, when
    // profile is fully loaded this vector will be empty.
    std::vector<ProfileLoadedCallback> created_callbacks;

    bool created_ = false;
  };

  explicit ProfileManager(const base::FilePath& user_data_dir);

  void OnProfileCreationStarted(Profile* profile,
                                Profile::CreateMode create_mode) override;
  void OnProfileCreationFinished(Profile* profile,
                                 Profile::CreateMode create_mode,
                                 bool success,
                                 bool is_new_profile) override;

  // Synchronously creates and returns a profile. This handles both the full
  // creation and adds it to the set managed by this ProfileManager. Returns
  // null if creation fails.
  Profile* CreateAndInitializeProfile(
      const base::FilePath& profile_dir,
      base::OnceCallback<std::unique_ptr<Profile>(const base::FilePath&)>
          factory);

  // Whether a new profile can be created at |path|.
  bool CanCreateProfileAtPath(const base::FilePath& path) const;

  // The path to the user data directory (DIR_USER_DATA).
  const base::FilePath user_data_dir_;

  // Maps profile path to `ProfileInfo` (if profile has been loaded). Use
  // `RegisterProfile()` to add into this map. This map owns all loaded profile
  // objects in a running instance of Chrome.
  using ProfilesInfoMap =
      std::map<base::FilePath, std::unique_ptr<ProfileInfo>>;
  ProfilesInfoMap profiles_info_;
};

#endif  // RADIUM_BROWSER_PROFILES_PROFILE_MANAGER_H_
