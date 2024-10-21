// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profiles_state.h"

#include "radium/browser/profiles/profile_selections.h"

namespace profiles {

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
