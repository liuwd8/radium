// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/prefs/profile_prefs.h"

#include <string>

#include "base/trace_event/trace_event.h"

namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* pref_registry,
                          const std::string& locale) {
  TRACE_EVENT0("browser", "chrome::RegisterProfilePrefs");
}

}  // namespace prefs
