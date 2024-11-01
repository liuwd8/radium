// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/platform_util.h"

#import <Cocoa/Cocoa.h>

#include "base/check.h"

namespace platform_util {

gfx::NativeView GetViewForWindow(gfx::NativeWindow native_window) {
  NSWindow* window = native_window.GetNativeNSWindow();
  DCHECK(window);
  DCHECK([window contentView]);
  return gfx::NativeView([window contentView]);
}

}  // namespace platform_util
