// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_STARTUP_STARTUP_BROWSER_CREATOR_H_
#define RADIUM_BROWSER_UI_STARTUP_STARTUP_BROWSER_CREATOR_H_

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"

class Browser;
class GURL;
class PrefRegistrySimple;
class Profile;

namespace base {
class CommandLine;
}

// Indicates how Chrome should start up the first profile.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class StartupProfileMode {
  // Regular startup with a browser window.
  kBrowserWindow = 0,
  // Profile picker window should be shown on startup.
  kProfilePicker = 1,
  // Chrome cannot start because no profiles are available.
  kError = 2,

  kMaxValue = kError,
};

struct StartupProfileInfo {
  raw_ptr<Profile> profile;
  StartupProfileMode mode;
};

class StartupBrowserCreator {
 public:
  typedef std::vector<Profile*> Profiles;

  explicit StartupBrowserCreator();
  StartupBrowserCreator(const StartupBrowserCreator&) = delete;
  StartupBrowserCreator& operator=(const StartupBrowserCreator&) = delete;

  ~StartupBrowserCreator();

  // This function is equivalent to ProcessCommandLine but should only be
  // called during actual process startup.
  bool Start(const base::CommandLine& cmd_line,
             const base::FilePath& cur_dir,
             StartupProfileInfo profile_info,
             const Profiles& last_opened_profiles);
};

#endif  // RADIUM_BROWSER_UI_STARTUP_STARTUP_BROWSER_CREATOR_H_
