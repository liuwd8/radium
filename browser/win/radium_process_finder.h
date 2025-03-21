// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_WIN_RADIUM_PROCESS_FINDER_H_
#define RADIUM_BROWSER_WIN_RADIUM_PROCESS_FINDER_H_

#include <windows.h>

#include "base/time/time.h"

namespace base {
class FilePath;
}

enum class NotifyRadiumResult {
  NOTIFY_SUCCESS,
  NOTIFY_FAILED,
  NOTIFY_WINDOW_HUNG,
};

// Finds an already running Radium window if it exists.
HWND FindRunningRadiumWindow(const base::FilePath& user_data_dir);

// Attempts to send the current command line to an already running instance of
// Radium via a WM_COPYDATA message.
// Returns true if a running Radium is found and successfully notified.
NotifyRadiumResult AttemptToNotifyRunningRadium(HWND remote_window);

// Changes the notification timeout to |new_timeout|, returns the old timeout.
base::TimeDelta SetNotificationTimeoutForTesting(base::TimeDelta new_timeout);

#endif  // RADIUM_BROWSER_WIN_RADIUM_PROCESS_FINDER_H_
