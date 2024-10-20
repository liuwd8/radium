// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PLATFORM_UTIL_H_
#define RADIUM_BROWSER_PLATFORM_UTIL_H_

#include "ui/gfx/native_widget_types.h"

namespace platform_util {

// Returns a NativeView handle for parenting dialogs off |window|. This can be
// used to position a dialog using a NativeWindow, when a NativeView (e.g.
// browser tab) isn't available.
gfx::NativeView GetViewForWindow(gfx::NativeWindow window);

}  // namespace platform_util

#endif  // RADIUM_BROWSER_PLATFORM_UTIL_H_
