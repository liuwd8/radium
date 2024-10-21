// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_PROFILES_STATE_H_
#define RADIUM_BROWSER_PROFILES_PROFILES_STATE_H_

class Profile;

namespace profiles {

// Returns true if the profile is a regular profile and specifically not an Ash
// internal profile. Callers who do not care about checking for Ash internal
// profiles should use `Profile::IsRegularProfile()` instead.
bool IsRegularUserProfile(Profile* profile);

}  // namespace profiles

#endif  // RADIUM_BROWSER_PROFILES_PROFILES_STATE_H_
