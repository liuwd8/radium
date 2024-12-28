// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

#include <memory>

#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/grit/theme_resources.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/theme_provider.h"
#include "ui/gfx/image/image_skia.h"

UntitledWidgetNonClientFrameView::UntitledWidgetNonClientFrameView(
    UntitledWidget* untitled_widget)
    : untitled_widget_(untitled_widget),
      paint_as_active_subscription_(
          untitled_widget_->RegisterPaintAsActiveChangedCallback(
              base::BindRepeating(
                  &UntitledWidgetNonClientFrameView::PaintAsActiveChanged,
                  base::Unretained(this)))) {}

UntitledWidgetNonClientFrameView::~UntitledWidgetNonClientFrameView() = default;

bool UntitledWidgetNonClientFrameView::IsFrameCondensed() const {
  return untitled_widget_->IsMaximized() || untitled_widget_->IsFullscreen();
}

int UntitledWidgetNonClientFrameView::GetTopAreaHeight() const {
  return untitled_widget_->GetTitleBarBackgroundHeight();
}

bool UntitledWidgetNonClientFrameView::CaptionButtonsOnLeadingEdge() const {
  return false;
}

gfx::Insets UntitledWidgetNonClientFrameView::GetCaptionButtonInsets() const {
  return gfx::Insets();
}

SkColor UntitledWidgetNonClientFrameView::GetFrameColor(
    BrowserFrameActiveState active_state) const {
  return GetColorProvider()->GetColor(ShouldPaintAsActive(active_state)
                                          ? ui::kColorFrameActive
                                          : ui::kColorFrameInactive);
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

gfx::ImageSkia UntitledWidgetNonClientFrameView::GetFrameImage(
    BrowserFrameActiveState active_state) const {
  const ui::ThemeProvider* tp = GetThemeProvider();
  const int frame_image_id = ShouldPaintAsActive(active_state)
                                 ? IDR_THEME_FRAME
                                 : IDR_THEME_FRAME_INACTIVE;
  return (tp->HasCustomImage(frame_image_id) ||
          tp->HasCustomImage(IDR_THEME_FRAME))
             ? *tp->GetImageSkiaNamed(frame_image_id)
             : gfx::ImageSkia();
}

gfx::ImageSkia UntitledWidgetNonClientFrameView::GetFrameOverlayImage(
    BrowserFrameActiveState active_state) const {
  const ui::ThemeProvider* tp = GetThemeProvider();
  const int frame_overlay_image_id = ShouldPaintAsActive(active_state)
                                         ? IDR_THEME_FRAME_OVERLAY
                                         : IDR_THEME_FRAME_OVERLAY_INACTIVE;
  return tp->HasCustomImage(frame_overlay_image_id)
             ? *tp->GetImageSkiaNamed(frame_overlay_image_id)
             : gfx::ImageSkia();
}

gfx::Size UntitledWidgetNonClientFrameView::GetMinimumSize() const {
  auto size = untitled_widget_->client_view()->GetMinimumSize();
  if (size.IsEmpty()) {
    return size;
  }

  gfx::Rect client_bounds(size);
  return GetWindowBoundsForClientBounds(client_bounds).size();
}

gfx::Size UntitledWidgetNonClientFrameView::GetMaximumSize() const {
  auto size = untitled_widget_->client_view()->GetMaximumSize();
  if (size.IsEmpty()) {
    return size;
  }

  gfx::Rect client_bounds(size);
  return GetWindowBoundsForClientBounds(client_bounds).size();
}

void UntitledWidgetNonClientFrameView::PaintAsActiveChanged() {
  // Changing the activation state may change the visible frame color.
  SchedulePaint();
}

bool UntitledWidgetNonClientFrameView::ShouldPaintAsActive(
    BrowserFrameActiveState active_state) const {
  return (active_state == BrowserFrameActiveState::kUseCurrent)
             ? ShouldPaintAsActive()
             : (active_state == BrowserFrameActiveState::kActive);
}

BEGIN_METADATA(UntitledWidgetNonClientFrameView)
END_METADATA
