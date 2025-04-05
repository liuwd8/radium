// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/window_finder.h"

#include "ui/display/screen.h"

gfx::NativeWindow WindowFinder::GetLocalProcessWindowAtPoint(
    const gfx::Point& screen_point,
    const std::set<gfx::NativeWindow>& ignore) {
  return display::Screen::GetScreen()->GetLocalProcessWindowAtPoint(
      screen_point, ignore);
}
