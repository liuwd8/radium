// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_APP_CONTROLLER_MAC_H_
#define RADIUM_BROWSER_APP_CONTROLLER_MAC_H_

#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "components/sessions/core/session_id.h"
#include "components/sessions/core/tab_restore_service.h"
#include "components/sessions/core/tab_restore_service_observer.h"
#include "radium/browser/profiles/profile.h"

#if defined(__OBJC__)

#import <AuthenticationServices/AuthenticationServices.h>
#import <Cocoa/Cocoa.h>

#include <memory>
#include <vector>

#include "base/files/file_path.h"
#include "components/prefs/pref_change_registrar.h"

class GURL;
class Profile;

namespace ui {
class ColorProvider;
}  // namespace ui

// The application controller object, created by loading the MainMenu nib.
// This handles things like responding to menus when there are no windows
// open, etc and acts as the NSApplication delegate.
@interface AppController : NSObject <NSUserInterfaceValidations,
                                     NSMenuDelegate,
                                     NSApplicationDelegate>

// The app-wide singleton AppController. Guaranteed to be the delegate of NSApp
// inside of Chromium (not inside of app shims; see AppShimDelegate). Guaranteed
// to not be nil.
@property(readonly, nonatomic, class) AppController* sharedController;

@property(readonly, nonatomic) BOOL startupComplete;

// Do not create new instances of AppController; use the `sharedController`
// property so that the invariants of there always being exactly one
// AppController and that that instance is the NSApp delegate always hold true.
- (instancetype)init NS_UNAVAILABLE;

// Try to close all browser windows, and if that succeeds then quit.
- (BOOL)tryToTerminateApplication:(NSApplication*)app;

// Stop trying to terminate the application. That is, prevent the final browser
// window closure from causing the application to quit.
- (void)stopTryingToTerminateApplication:(NSApplication*)app;

// This is called when the system wide light or dark mode changes.
- (void)nativeThemeDidChange;

// Certain NSMenuItems [Close Tab and Close Window] have different
// keyEquivalents depending on context. This must be invoked in two locations:
//   * In menuNeedsUpdate:, which is called prior to showing the NSMenu.
//   * In CommandDispatcher, which independently searches for a matching
//     keyEquivalent.
- (void)updateMenuItemKeyEquivalents;

// Indicate that the system is powering off or logging out.
- (void)willPowerOff:(NSNotification*)inNotification;

@end

#endif  // __OBJC__

#endif  // RADIUM_BROWSER_APP_CONTROLLER_MAC_H_
