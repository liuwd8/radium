// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_LIFETIME_APPLICATION_LIFETIME_H_
#define RADIUM_BROWSER_LIFETIME_APPLICATION_LIFETIME_H_

#include "build/build_config.h"

class Browser;

namespace radium {

// Attempt to exit by closing all browsers.  This is equivalent to
// CloseAllBrowsers() on platforms where the application exits
// when no more windows are remaining. On other platforms (the Mac),
// this will additionally exit the application if all browsers are
// successfully closed.
//  Note that the exit process may be interrupted by download or
// unload handler, and the browser may or may not exit.
void AttemptExit();

}  // namespace radium

#endif  // RADIUM_BROWSER_LIFETIME_APPLICATION_LIFETIME_H_
