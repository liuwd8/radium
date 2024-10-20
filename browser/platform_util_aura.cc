// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/check.h"
#include "radium/browser/platform_util.h"

namespace platform_util {

gfx::NativeView GetViewForWindow(gfx::NativeWindow window) {
  DCHECK(window);
  return window;
}

}  // namespace platform_util
