// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/app/radium_main_mac.h"

#import <Cocoa/Cocoa.h>

#include <string>

#import "base/apple/bundle_locations.h"
#import "base/apple/foundation_util.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "radium/common/radium_constants.h"
#include "radium/common/radium_paths_internal.h"

void SetUpBundleOverrides() {
  @autoreleasepool {
    base::apple::SetOverrideFrameworkBundlePath(
        radium::GetFrameworkBundlePath());

    NSBundle* base_bundle = radium::OuterAppBundle();
    base::apple::SetBaseBundleIDOverride(
        base::SysNSStringToUTF8(base_bundle.bundleIdentifier));

    base::FilePath child_exe_path =
        radium::GetFrameworkBundlePath().Append("Helpers").Append(
            radium::kHelperProcessExecutablePath);

    // On the Mac, the child executable lives at a predefined location within
    // the app bundle's versioned directory.
    base::PathService::OverrideAndCreateIfNeeded(
        content::CHILD_PROCESS_EXE, child_exe_path, /*is_absolute=*/true,
        /*create=*/false);
  }
}

bool IsAlertsHelperLaunchedViaNotificationAction() {
  // We allow the main Chrome app to be launched via a notification action. We
  // detect and log that to UMA by checking the passed in NSNotification in
  // -applicationDidFinishLaunching: (//chrome/browser/app_controller_mac.mm).
  if (!base::apple::IsBackgroundOnlyProcess()) {
    return false;
  }

  // If we have a process type then we were not launched by the system.
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kProcessType)) {
    return false;
  }

  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path)) {
    return false;
  }

  // Check if our executable name matches the helper app for notifications.
  std::string helper_name = path.BaseName().value();
  return base::EndsWith(helper_name, radium::kMacHelperSuffixAlerts);
}
