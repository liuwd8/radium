// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profile_manager.h"

#include "absl/cleanup/cleanup.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/trace_event/trace_event.h"
#include "chrome/browser/profiles/profiles_state.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/global_features.h"
#include "radium/browser/profiles/profile.h"
#include "radium/common/pref_names.h"
#include "radium/common/radium_constants.h"
#include "radium/common/radium_paths.h"

namespace {
void RunCallbacks(std::vector<base::OnceCallback<void(Profile*)>>& callbacks,
                  Profile* profile) {
  for (base::OnceCallback<void(Profile*)>& callback : callbacks) {
    std::move(callback).Run(profile);
  }
}

}  // namespace

ProfileManager::ProfileInfo::ProfileInfo() = default;
ProfileManager::ProfileInfo::~ProfileInfo() = default;

std::unique_ptr<ProfileManager> ProfileManager::Create() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  return base::WrapUnique(new ProfileManager(user_data_dir));
}

// static
Profile* ProfileManager::GetLastUsedProfile() {
  ProfileManager* profile_manager =
      BrowserProcess::Get()->GetFeatures()->profile_manager();
  if (!profile_manager) {  // Can be null in unit tests.
    return nullptr;
  }

  return profile_manager->GetProfile(profile_manager->GetLastUsedProfileDir());
}

ProfileManager::ProfileManager(const base::FilePath& user_data_dir)
    : user_data_dir_(user_data_dir) {}

ProfileManager::~ProfileManager() = default;

base::FilePath ProfileManager::GetLastUsedProfileDir() {
  return user_data_dir_.Append(GetLastUsedProfileBaseName());
}

// static
base::FilePath ProfileManager::GetLastUsedProfileBaseName() {
  PrefService* local_state = BrowserProcess::Get()->local_state();
  DCHECK(local_state);
  base::FilePath last_used_profile_base_name =
      local_state->GetFilePath(prefs::kProfileLastUsed);
  // Make sure the system profile can't be the one marked as the last one used
  // since it shouldn't get a browser.
  if (!last_used_profile_base_name.empty() &&
      last_used_profile_base_name.value() != radium::kSystemProfileDir) {
    return last_used_profile_base_name;
  }

  return base::FilePath(radium::kInitialProfile);
}

Profile* ProfileManager::GetProfile(const base::FilePath& profile_dir,
                                    bool create) {
  TRACE_EVENT0("browser", "ProfileManager::GetProfile");

  // If the profile is already loaded (e.g., chrome.exe launched twice), just
  // return it.
  Profile* profile = GetProfileByPath(profile_dir);
  if (profile || !create) {
    return profile;
  }
  return CreateAndInitializeProfile(
      profile_dir,
      // Because the callback is called synchronously, it's safe to use
      // Unretained here.
      base::BindOnce(&ProfileManager::CreateProfileHelper,
                     base::Unretained(this)));
}

void ProfileManager::CreateProfileAsync(
    const base::FilePath& profile_path,
    base::OnceCallback<void(Profile*)> initialized_callback,
    base::OnceCallback<void(Profile*)> created_callback) {
  absl::Cleanup run_callback_on_return = [&]() {
    if (!initialized_callback.is_null()) {
      std::move(initialized_callback).Run(nullptr);
    }
  };

  if (!CanCreateProfileAtPath(profile_path)) {
    LOG(ERROR) << "Cannot create profile at path " << profile_path;
    return;
  }

  // Create the profile if needed and collect its ProfileInfo.
  auto iter = profiles_info_.find(profile_path);
  ProfileInfo* info = nullptr;
  if (iter != profiles_info_.end()) {
    info = iter->second.get();
    // Profile has already been created. Run callback immediately.
    if (info->created_ && !initialized_callback.is_null()) {
      std::move(initialized_callback).Run(info->profile.get());
      return;
    }
  } else {
    // Initiate asynchronous creation process.
    std::unique_ptr<Profile> profile = CreateProfileAsyncHelper(profile_path);
    if (!profile) {
      return;
    }

    auto iter_result =
        profiles_info_.insert({profile_path, std::make_unique<ProfileInfo>()});
    CHECK(iter_result.second);
    info = iter_result.first->second.get();
    info->profile = std::move(profile);
  }

  std::move(run_callback_on_return).Cancel();

  // Profile is either already in the process of being created, or new.
  // Add callback to the list.
  if (!initialized_callback.is_null()) {
    info->init_callbacks.push_back(std::move(initialized_callback));
  }
  if (!created_callback.is_null()) {
    info->created_callbacks.push_back(std::move(created_callback));
  }
}

bool ProfileManager::IsAllowedProfilePath(const base::FilePath& path) const {
  return path.DirName() == user_data_dir();
}

std::vector<Profile*> ProfileManager::GetLoadedProfiles() const {
  std::vector<Profile*> profiles;
  for (const auto& iter : profiles_info_) {
    if (iter.second->created_) {
      profiles.push_back(iter.second->profile.get());
    }
  }
  return profiles;
}

Profile* ProfileManager::GetProfileByPath(const base::FilePath& path) const {
  auto it = profiles_info_.find(path);
  return it != profiles_info_.end() ? it->second->profile.get() : nullptr;
}

void ProfileManager::SetProfileAsLastUsed(Profile* last_active) {
  // Only keep track of profiles that we are managing; tests may create others.
  // Also never consider the SystemProfile as "active".
  if (profiles_info_.find(last_active->GetPath()) != profiles_info_.end() &&
      !last_active->IsSystemProfile()) {
    base::FilePath profile_path_base = last_active->GetBaseName();
    if (profile_path_base != GetLastUsedProfileBaseName()) {
      profiles::SetLastUsedProfile(profile_path_base);
    }
  }
}

std::unique_ptr<Profile> ProfileManager::CreateProfileHelper(
    const base::FilePath& path) {
  return Profile::CreateProfile(path, this, Profile::CreateMode::kSynchronous);
}

std::unique_ptr<Profile> ProfileManager::CreateProfileAsyncHelper(
    const base::FilePath& path) {
  return Profile::CreateProfile(path, this, Profile::CreateMode::kAsynchronous);
}

void ProfileManager::OnProfileCreationStarted(Profile* profile,
                                              Profile::CreateMode create_mode) {
}

void ProfileManager::OnProfileCreationFinished(Profile* profile,
                                               Profile::CreateMode create_mode,
                                               bool success,
                                               bool is_new_profile) {
  if (create_mode == Profile::CreateMode::kSynchronous) {
    // Already initialized in OnProfileCreationStarted().
    return;
  }

  auto iter = profiles_info_.find(profile->GetPath());
  CHECK(iter != profiles_info_.end());
  ProfileInfo* info = iter->second.get();
  info->created_ = success;

  std::vector<base::OnceCallback<void(Profile*)>> created_callbacks;
  info->created_callbacks.swap(created_callbacks);

  std::vector<base::OnceCallback<void(Profile*)>> init_callbacks;
  info->init_callbacks.swap(init_callbacks);

  if (success) {
    RunCallbacks(created_callbacks, profile);
  }

  // Perform initialization.
  if (!success) {
    profile = nullptr;
    profiles_info_.erase(iter);
  }

  // Invoke INITIALIZED for all profiles.
  // Profile might be null, meaning that the creation failed.
  RunCallbacks(init_callbacks, profile);
}

Profile* ProfileManager::CreateAndInitializeProfile(
    const base::FilePath& profile_dir,
    base::OnceCallback<std::unique_ptr<Profile>(const base::FilePath&)>
        factory) {
  std::unique_ptr<Profile> profile = std::move(factory).Run(profile_dir);
  if (!profile) {
    return nullptr;
  }

  Profile* profile_ptr = profile.get();
  auto iter_result = profiles_info_.insert(
      {profile_ptr->GetPath(), std::make_unique<ProfileInfo>()});
  CHECK(iter_result.second);
  iter_result.first->second->profile = std::move(profile);
  return profile_ptr;
}

bool ProfileManager::CanCreateProfileAtPath(const base::FilePath& path) const {
  if (!IsAllowedProfilePath(path)) {
    LOG(ERROR) << "Cannot create profile at path " << path.AsUTF8Unsafe();
    return false;
  }

  return true;
}
