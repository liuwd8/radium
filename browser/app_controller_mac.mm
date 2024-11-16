// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "radium/browser/app_controller_mac.h"

#include <dispatch/dispatch.h>
#include <stddef.h>

#include <memory>
#include <vector>

#include "base/apple/bridging.h"
#include "base/apple/foundation_util.h"
#include "base/auto_reset.h"
#include "base/check_op.h"
#include "base/command_line.h"
#include "base/containers/flat_map.h"
#include "base/debug/dump_without_crashing.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/mac/mac_util.h"
#include "base/memory/raw_ptr.h"
#include "base/run_loop.h"
#include "base/scoped_multi_source_observation.h"
#include "base/scoped_observation.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/thread_restrictions.h"
#include "build/branding_buildflags.h"
#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "net/base/apple/url_conversions.h"
#include "radium/browser/lifetime/application_lifetime_desktop.h"
#include "ui/base/cocoa/nsmenu_additions.h"
#include "ui/native_theme/native_theme_mac.h"
#include "ui/native_theme/native_theme_observer.h"
#include "url/gurl.h"

class AppControllerNativeThemeObserver : public ui::NativeThemeObserver {
 public:
  explicit AppControllerNativeThemeObserver(AppController* app_controller)
      : app_controller_(app_controller) {
    native_theme_observation_.Observe(
        ui::NativeThemeMac::GetInstanceForNativeUi());
  }

  // NativeThemeObserver:
  void OnNativeThemeUpdated(ui::NativeTheme* observed_theme) override {
    [app_controller_ nativeThemeDidChange];
  }

 private:
  base::ScopedObservation<ui::NativeTheme, ui::NativeThemeObserver>
      native_theme_observation_{this};
  AppController* const app_controller_;  // Weak; owns us.
};

@implementation AppController {
  BOOL _startupComplete;

  // This will be true after receiving a NSWorkspaceWillPowerOffNotification.
  BOOL _isPoweringOff;

  // Request to keep the browser alive during that object's lifetime. Every
  // Browser instance holds a ScopedKeepAlive as well to make sure the browser
  // stays alive as long as any windows are open, but on macOS we want the
  // browser process to also stay alive without any windows open. To support
  // this, this ScopedKeepAlive is created in -applicationDidFinishLaunching.
  //
  // When the user launches an app shim while Chrome isn't running, the app shim
  // launches Chrome with the _kLSOpenOptionBackgroundLaunchKey option. This
  // causes the activationPolicy to be equal to
  // NSApplicationActivationPolicyProhibited (i.e. Chrome is not visibly running
  // in the Dock and task switcher). In this state we don't want to keep the
  // browser process alive indefinitely (but can't skip creating this
  // ScopedKeepAlive entirely, as that could result in the  browser process
  // terminating before it has fully initialized and had a chance to for example
  // process messages from app shims). Once the app shim launch has been
  // processed to the point of having created the expected Browser instances or
  // other ScopedKeepAlive instances, -resetKeepAliveWhileHidden is called to
  // reset `_keepAlive`.
  //
  // When the user explicitly launches Chrome, or when Chrome creates any
  // windows, the `activationPolicy` is changed by the OS. By observing
  // `activationPolicy` for `NSRunningApplication.currentApplication` we can
  // recreate `_keepAlive` if and when the activation policy changes.
  std::unique_ptr<ScopedKeepAlive> _keepAlive;

  // Set to `NSRunningApplication.currentApplication` while we're observing
  // the `activationPolicy` of ourselves.
  NSRunningApplication* __strong _runningApplication;
}

@synthesize startupComplete = _startupComplete;

+ (AppController*)sharedController {
  static AppController* sharedController = [] {
    AppController* sharedController = [[AppController alloc] init];
    NSApp.delegate = sharedController;
    return sharedController;
  }();

  CHECK_NE(nil, sharedController);
  CHECK_EQ(NSApp.delegate, sharedController);
  return sharedController;
}

- (instancetype)init {
  if (self = [super init]) {
    // -[NSMenu cr_menuItemForKeyEquivalentEvent:] lives in /content, but
    // we need to execute special update code before the search begins.
    // Setting this block gives us the hook we need.
    [NSMenu cr_setMenuItemForKeyEquivalentEventPreSearchBlock:^{
      // We avoid calling -[NSMenuDelegate menuNeedsUpdate:] on each submenu's
      // delegate as that can be slow. Instead, we update the relevant
      // NSMenuItems.
      [AppController.sharedController updateMenuItemKeyEquivalents];
    }];
  }
  return self;
}

- (void)dealloc {
  NOTREACHED();
}

- (BOOL)tryToTerminateApplication:(NSApplication*)app {
  radium::CloseAllBrowsers();
  return YES;
}

- (void)stopTryingToTerminateApplication:(NSApplication*)app {
}

- (void)allowApplicationToTerminate {
  // Tell BrowserList to stop the RunLoop and terminate the application when the
  // last Browser is closed.
  _keepAlive.reset();
}

// Called when shutting down or logging out.
- (void)willPowerOff:(NSNotification*)notify {
  // Don't attempt any shutdown here. Cocoa will shortly call
  // -[BrowserCrApplication terminate:].
  _isPoweringOff = YES;
}

// Validates menu items in the dock (always) and in the menu bar (if there is no
// browser).
- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item {
  return NO;
}

- (void)nativeThemeDidChange {
}

// Updates menu items in the File menu to match the main window.
- (void)updateMenuItemKeyEquivalents {
}

- (id)targetForPerformClose {
  return [NSApp targetForAction:@selector(performClose:)];
}

// Returns the NSWindow that's the target of the Close Window command.
- (NSWindow*)windowForPerformClose {
  NSWindow* targetWindow = nil;
  id target = [self targetForPerformClose];

  // If `target` is a popover (likely the dictionary lookup popover), the
  // main window should handle the close menu item action.
  if ([target isKindOfClass:[NSPopover class]]) {
    targetWindow =
        [[[base::apple::ObjCCast<NSPopover>(target) contentViewController] view]
            window];
  } else {
    targetWindow = base::apple::ObjCCast<NSWindow>(target);
  }

  // If `targetWindow` is a child (a popover or bubble), the topmost parent
  // window should handle the command.
  while (targetWindow.parentWindow) {
    targetWindow = targetWindow.parentWindow;
  }

  return targetWindow;
}

// This only has an effect on macOS 12+, and requests any state restoration
// archive to be created with secure encoding. See the article at
// https://sector7.computest.nl/post/2022-08-process-injection-breaking-all-macos-security-layers-with-a-single-vulnerability/
// for more details.
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication*)app {
  return YES;
}

- (BOOL)application:(NSApplication*)application
    willContinueUserActivityWithType:(NSString*)userActivityType {
  return [userActivityType isEqualToString:NSUserActivityTypeBrowsingWeb];
}

- (void)application:(NSApplication*)application
    didFailToContinueUserActivityWithType:(NSString*)userActivityType
                                    error:(NSError*)error {
}

@end  // @implementation AppController
