// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_OPAQUE_FRAME_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_OPAQUE_FRAME_VIEW_H_

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

class OpaqueFrameView : public UntitledWidgetNonClientFrameView {
 public:
  // The frame border is only visible in restored mode and is hardcoded to 4 px
  // on each side regardless of the system window border size.  This is
  // overridable by subclasses, so RestoredFrameBorderInsets() should be used
  // instead of using this constant directly.
  static constexpr int kFrameBorderThickness = 4;

  explicit OpaqueFrameView(views::Widget* widget);
  OpaqueFrameView(const OpaqueFrameView&) = delete;
  OpaqueFrameView& operator=(const OpaqueFrameView&) = delete;

  ~OpaqueFrameView() override;

  gfx::Insets RestoredFrameBorderInsets() const;

 protected:
  // View:
  gfx::Insets GetInsets() const override;

  // views::NonClientFrameView:
  gfx::Rect GetBoundsForClientView() const override;
  gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_OPAQUE_FRAME_VIEW_H_
