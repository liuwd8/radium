// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PREFS_PROFILE_PREFS_H_
#define RADIUM_BROWSER_PREFS_PROFILE_PREFS_H_

#include <string>

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* pref_registry,
                          const std::string& locale);

}  // namespace prefs

#endif  // RADIUM_BROWSER_PREFS_PROFILE_PREFS_H_
