// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_LIFETIME_APPLICATION_LIFETIME_DESKTOP_H_
#define RADIUM_BROWSER_LIFETIME_APPLICATION_LIFETIME_DESKTOP_H_

#include "base/callback_list.h"
#include "base/functional/callback.h"

static_assert(!BUILDFLAG(IS_ANDROID), "For non-Android Chrome only");

namespace radium {

// Closes all browsers. If the session is ending the windows are closed
// directly. Otherwise the windows are closed by way of posting a WM_CLOSE
// message. This will quit the application if there is nothing other than
// browser windows keeping it alive or the application is quitting.
void CloseAllBrowsers();

// If there are no browsers open and we aren't already shutting down,
// initiate a shutdown.
void ShutdownIfNeeded();

// Begins shutdown of the application when the desktop session is ending.
void SessionEnding();

// Called once the application is exiting.
void OnAppExiting();

// Called once the application is exiting to do any platform specific
// processing required.
void HandleAppExitingForPlatform();

// Called when the process of closing all browsers starts or is cancelled.
void OnClosingAllBrowsers(bool closing);

// Registers a callback that will be invoked with true when all browsers start
// closing, and false if and when that process is cancelled.
base::CallbackListSubscription AddClosingAllBrowsersCallback(
    base::RepeatingCallback<void(bool)> closing_all_browsers_callback);

// Sets the exit type to `ExitType::kClean` for all browser profiles.
void MarkAsCleanShutdown();

// Returns true if all browsers can be closed without user interaction.
// This currently checks if there is pending download, or if it needs to
// handle unload handler.
bool AreAllBrowsersCloseable();

}  // namespace radium

#endif  // RADIUM_BROWSER_LIFETIME_APPLICATION_LIFETIME_DESKTOP_H_
