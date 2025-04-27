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

}  // namespace browser_shutdown

#endif  // RADIUM_BROWSER_LIFETIME_BROWSER_SHUTDOWN_H_
