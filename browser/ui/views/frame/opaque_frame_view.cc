// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/opaque_frame_view.h"

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

OpaqueFrameView::OpaqueFrameView(views::Widget* widget)
    : UntitledWidgetNonClientFrameView(widget) {}

OpaqueFrameView::~OpaqueFrameView() = default;

gfx::Insets OpaqueFrameView::RestoredFrameBorderInsets() const {
  return gfx::Insets(kFrameBorderThickness);
}

gfx::Insets OpaqueFrameView::GetInsets() const {
  return IsFrameCondensed() ? gfx::Insets() : RestoredFrameBorderInsets();
}

gfx::Rect OpaqueFrameView::GetBoundsForClientView() const {
  return bounds();
}

gfx::Rect OpaqueFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  return client_bounds;
}
