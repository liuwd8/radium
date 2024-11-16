// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_process_platform_part_mac.h"

#import "radium/browser/app_controller_mac.h"
#include "radium/browser/radium_browser_application_mac.h"
#include "services/device/public/cpp/geolocation/system_geolocation_source_apple.h"

BrowserProcessPlatformPart::BrowserProcessPlatformPart() = default;
BrowserProcessPlatformPart::~BrowserProcessPlatformPart() = default;

void BrowserProcessPlatformPart::StartTearDown() {}

void BrowserProcessPlatformPart::AttemptExit(bool try_to_quit_application) {
  // On the Mac, the application continues to run once all windows are closed.
  // Terminate will result in a CloseAllBrowsers() call, and once (and if)
  // that is done, will cause the application to exit cleanly.
  //
  // This function is called for two types of attempted exits: URL requests
  // (chrome://quit or chrome://restart), and a keyboard menu invocations of
  // command-Q. (Interestingly, selecting the Quit command with the mouse don't
  // come down this code path at all.) URL requests to exit have
  // |try_to_quit_application| set to true; keyboard menu invocations have it
  // set to false.

  // if (!try_to_quit_application) {
  //   // A keyboard menu invocation.
  //   if (![AppController.sharedController runConfirmQuitPanel]) {
  //     return;
  //   }
  // }

  radium_browser_application_mac::Terminate();
}

void BrowserProcessPlatformPart::PreMainMessageLoopRun() {
  if (!device::GeolocationSystemPermissionManager::GetInstance()) {
    device::GeolocationSystemPermissionManager::SetInstance(
        device::SystemGeolocationSourceApple::
            CreateGeolocationSystemPermissionManager());
  }
}
