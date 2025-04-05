// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_WINDOW_FINDER_H_
#define RADIUM_BROWSER_UI_WINDOW_FINDER_H_

#include <set>

#include "ui/gfx/native_widget_types.h"

namespace gfx {
class Point;
}

// Class used by the tabstrip to find chrome windows (through display::Screen)
// that we can attach tabs to. We don't call
// display::Screen::GetLocalProcessWindowAtPoint, but use this class just for
// the sake of simplicity in unittests. That is, some of them want to set own
// implementation of WindowFinder and emulate mouse movements. However, if they
// set another screen, assertion that screen has been changed may be hit.
class WindowFinder {
 public:
  WindowFinder() = default;
  WindowFinder(const WindowFinder&) = delete;
  WindowFinder& operator=(const WindowFinder&) = delete;
  virtual ~WindowFinder() = default;

  // See comment at display::Screen::GetLocalProcessWindowAtPoint().
  virtual gfx::NativeWindow GetLocalProcessWindowAtPoint(
      const gfx::Point& screen_point,
      const std::set<gfx::NativeWindow>& ignore);
};

#endif  // RADIUM_BROWSER_UI_WINDOW_FINDER_H_
