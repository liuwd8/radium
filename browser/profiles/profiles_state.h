// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_PROFILES_STATE_H_
#define RADIUM_BROWSER_PROFILES_PROFILES_STATE_H_

class PrefRegistrySimple;
class Profile;

namespace base {
class CommandLine;
class FilePath;
}  // namespace base

namespace profiles {

// Returns the path to the default profile directory, based on the given
// user data directory.
base::FilePath GetDefaultProfileDir(const base::FilePath& user_data_dir);

// Register multi-profile related preferences in Local State.
void RegisterPrefs(PrefRegistrySimple* registry);

// Sets the last used profile pref to |profile_dir|, unless |profile_dir| is the
// System Profile directory, which is an invalid last used profile.
void SetLastUsedProfile(const base::FilePath& profile_dir);

// Returns true if the profile is a regular profile and specifically not an Ash
// internal profile. Callers who do not care about checking for Ash internal
// profiles should use `Profile::IsRegularProfile()` instead.
bool IsRegularUserProfile(Profile* profile);

}  // namespace profiles

#endif  // RADIUM_BROWSER_PROFILES_PROFILES_STATE_H_
