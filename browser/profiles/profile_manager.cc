// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profile_manager.h"

#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/trace_event/trace_event.h"
#include "radium/browser/profiles/profile.h"
#include "radium/common/radium_paths.h"

std::unique_ptr<ProfileManager> ProfileManager::Create() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  return base::WrapUnique(new ProfileManager(user_data_dir));
}

ProfileManager::ProfileManager(const base::FilePath& user_data_dir)
    : user_data_dir_(user_data_dir) {}

ProfileManager::~ProfileManager() = default;

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
