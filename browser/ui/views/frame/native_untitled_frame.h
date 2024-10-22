// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_NATIVE_UNTITLED_FRAME_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_NATIVE_UNTITLED_FRAME_H_

#include "ui/views/widget/widget.h"

class NativeUntitledFrame {
 public:
  virtual ~NativeUntitledFrame() = default;

  // Returns the platform specific InitParams for initializing our widget.
  virtual views::Widget::InitParams GetWidgetParams() = 0;

  // Returns true if the OS takes care of showing the system menu. Returning
  // false means BrowserFrame handles showing the system menu.
  virtual bool UsesNativeSystemMenu() const = 0;

  // Returns true when the window placement should be stored.
  virtual bool ShouldSaveWindowPlacement() const = 0;

  // Retrieves the window placement (show state and bounds) for restoring.
  virtual void GetWindowPlacement(
      gfx::Rect* bounds,
      ui::mojom::WindowShowState* show_state) const = 0;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_NATIVE_UNTITLED_FRAME_H_
