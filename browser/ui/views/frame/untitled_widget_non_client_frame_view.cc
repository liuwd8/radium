// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

#include <memory>

#include "third_party/skia/include/core/SkRRect.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/widget/widget.h"

UntitledWidgetNonClientFrameView::UntitledWidgetNonClientFrameView(
    views::Widget* widget)
    : widget_(widget) {}

UntitledWidgetNonClientFrameView::~UntitledWidgetNonClientFrameView() = default;

bool UntitledWidgetNonClientFrameView::IsFrameCondensed() const {
  return widget_->IsMaximized() || widget_->IsFullscreen();
}

gfx::Insets
UntitledWidgetNonClientFrameView::RestoredMirroredFrameBorderInsets() const {
  NOTREACHED();
}

gfx::Insets UntitledWidgetNonClientFrameView::GetInputInsets() const {
  NOTREACHED();
}

SkRRect UntitledWidgetNonClientFrameView::GetRestoredClipRegion() const {
  NOTREACHED();
}

int UntitledWidgetNonClientFrameView::GetTranslucentTopAreaHeight() const {
  return 0;
}

gfx::Size UntitledWidgetNonClientFrameView::GetMinimumSize() const {
  return widget_->client_view()->GetMinimumSize();
}

gfx::Size UntitledWidgetNonClientFrameView::GetMaximumSize() const {
  return widget_->client_view()->GetMaximumSize();
}

BEGIN_METADATA(UntitledWidgetNonClientFrameView)
END_METADATA
