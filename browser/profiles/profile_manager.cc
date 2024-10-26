// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profile_manager.h"

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

  return base::FilePath::FromASCII(radium::kInitialProfile);
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

Profile* ProfileManager::GetProfileByPath(const base::FilePath& path) const {
  auto it = profiles_info_.find(path);
  return it != profiles_info_.end() ? it->second.get() : nullptr;
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
  return Profile::CreateProfile(path, nullptr,
                                Profile::CreateMode::kSynchronous);
}

std::unique_ptr<Profile> ProfileManager::CreateProfileAsyncHelper(
    const base::FilePath& path) {
  return Profile::CreateProfile(path, nullptr,
                                Profile::CreateMode::kAsynchronous);
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
  profiles_info_.insert(
      std::make_pair(profile_ptr->GetPath(), std::move(profile)));
  return profile_ptr;
}
