// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profiles_state.h"

#include "base/files/file_path.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/profiles/profile_selections.h"
#include "radium/common/pref_names.h"
#include "radium/common/radium_constants.h"

namespace profiles {

base::FilePath GetDefaultProfileDir(const base::FilePath& user_data_dir) {
  base::FilePath default_profile_dir(user_data_dir);
  default_profile_dir =
      default_profile_dir.AppendASCII(radium::kInitialProfile);
  return default_profile_dir;
}

void RegisterPrefs(PrefRegistrySimple* registry) {
  // Preferences about global profile information.
  registry->RegisterStringPref(prefs::kProfileLastUsed, std::string());
}

void SetLastUsedProfile(const base::FilePath& profile_dir) {
  // We should never be saving the System Profile as the last one used since it
  // shouldn't have a browser.
  if (profile_dir == base::FilePath(radium::kSystemProfileDir)) {
    return;
  }

  PrefService* local_state = BrowserProcess::Get()->local_state();
  DCHECK(local_state);
  local_state->SetFilePath(prefs::kProfileLastUsed, profile_dir);
}

bool IsRegularUserProfile(Profile* profile) {
  ProfileSelections selections =
      ProfileSelections::Builder()
          .WithRegular(ProfileSelection::kOriginalOnly)  // the default
          // Filter out ChromeOS irregular profiles (login, lock screen...);
          // they are of type kRegular (returns true for `Profile::IsRegular()`)
          // but aren't used to browse the web and users can't configure them.
          .WithAshInternals(ProfileSelection::kNone)
          .Build();
  return selections.ApplyProfileSelection(profile);
}

}  // namespace profiles
