// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_LIFETIME_BROWSER_SHUTDOWN_H_
#define RADIUM_BROWSER_LIFETIME_BROWSER_SHUTDOWN_H_

namespace browser_shutdown {

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class ShutdownType {
  // An uninitialized value.
  kNotValid = 0,
  // The last browser window was closed.
  kWindowClose = 1,
  // The user clicked on the Exit menu item.
  kBrowserExit = 2,
  // User logoff or system shutdown.
  kEndSession = 3,
  // Exit without onbeforeunload or in-progress download prompts.
  kSilentExit = 4,
  // The browser process is exiting but not by a user action. These exit paths
  // can happen with early exit paths where the browser main is not executed.
  kOtherExit = 5,
  kMaxValue = kOtherExit
};

// Called when the browser starts shutting down so that we can measure shutdown
// time.
void OnShutdownStarting(ShutdownType type);

bool HasShutdownStarted();

// Returns true if OnShutdownStarting has been called and unload handlers (e.g.,
// an in-progress download or a page's beforeunload handler) should be ignored.
// This is true for kEndSession and kSilentExit shutdown types.
bool ShouldIgnoreUnloadHandlers();

// There are various situations where the browser process should continue to
// run after the last browser window has closed - the Mac always continues
// running until the user explicitly quits, and on Windows/Linux the application
// should not shutdown when the last browser window closes if there are any
// BackgroundContents running.
// When the user explicitly chooses to shutdown the app (via the "Exit" or
// "Quit" menu items) BrowserList will call SetTryingToQuit() to tell itself to
// initiate a shutdown when the last window closes.
// If the quit is aborted, then the flag should be reset.

// This is a low-level mutator; in general, don't call SetTryingToQuit(true),
// except from appropriate places in BrowserList. To quit, use usual means,
// e.g., using |chrome_browser_application_mac::Terminate()| on the Mac, or
// |BrowserList::CloseAllWindowsAndExit()| on other platforms. To stop quitting,
// use |chrome_browser_application_mac::CancelTerminate()| on the Mac; other
// platforms can call SetTryingToQuit(false) directly.
void SetTryingToQuit(bool quitting);

// General accessor.
bool IsTryingToQuit();

}  // namespace browser_shutdown

#endif  // RADIUM_BROWSER_LIFETIME_BROWSER_SHUTDOWN_H_
