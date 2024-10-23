// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/opaque_frame_view.h"

#include <memory>

#include "base/i18n/rtl.h"
#include "base/trace_event/trace_event.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/theme_provider.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/views/resources/grit/views_resources.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/window/frame_background.h"

namespace {

static constexpr int kNonClientExtraTopThickness = 1;
}  // namespace

OpaqueFrameView::OpaqueFrameView(UntitledWidget* untitled_widget)
    : UntitledWidgetNonClientFrameView(untitled_widget),
      frame_background_(std::make_unique<views::FrameBackground>()) {}

OpaqueFrameView::~OpaqueFrameView() = default;

int OpaqueFrameView::GetTopBackgroundHeight() const {
  int thickness = GetTopAreaHeight();
  if (thickness > 0) {
    thickness += kNonClientExtraTopThickness;
  }
  return thickness;
}

gfx::Insets OpaqueFrameView::GetInsets() const {
  return IsFrameCondensed() ? gfx::Insets() : RestoredFrameBorderInsets();
}

gfx::Rect OpaqueFrameView::GetBoundsForClientView() const {
  gfx::Insets insets;
  gfx::Rect content_bounds = GetContentsBounds();
  content_bounds.Inset(insets);
  return content_bounds;
}

gfx::Rect OpaqueFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  gfx::Rect window_bounds(client_bounds);
  gfx::Insets insets = GetInsets();
  window_bounds.Inset(-insets);
  return window_bounds;
}

int OpaqueFrameView::NonClientHitTest(const gfx::Point& point) {
  int super_component =
      UntitledWidgetNonClientFrameView::NonClientHitTest(point);
  if (super_component != HTNOWHERE) {
    return super_component;
  }

  if (!bounds().Contains(point)) {
    return HTNOWHERE;
  }

  int frame_component = GetWidget()->client_view()->NonClientHitTest(point);
  if (frame_component != HTNOWHERE) {
    return frame_component;
  }

  views::WidgetDelegate* delegate = GetWidget()->widget_delegate();
  if (!delegate) {
    LOG(WARNING) << "delegate is null, returning safe default.";
    return HTCAPTION;
  }

  // In the window corners, the resize areas don't actually expand bigger, but
  // the 16 px at the end of each edge triggers diagonal resizing.
  constexpr int kResizeAreaCornerSize = 16;
  auto resize_border = GetInsets();
  if (base::i18n::IsRTL()) {
    resize_border =
        gfx::Insets::TLBR(resize_border.top(), resize_border.right(),
                          resize_border.bottom(), resize_border.left());
  }
  // The top resize border has extra thickness.
  resize_border.set_top(GetTopAreaHeight());
  int window_component =
      GetHTComponentForFrame(point, resize_border, kResizeAreaCornerSize,
                             kResizeAreaCornerSize, delegate->CanResize());
  // Fall back to the caption if no other component matches.
  return (window_component == HTNOWHERE) ? HTCAPTION : window_component;
}

gfx::Size OpaqueFrameView::GetMinimumSize() const {
  gfx::Rect client_bounds(gfx::Point(),
                          UntitledWidgetNonClientFrameView::GetMinimumSize());
  return GetWindowBoundsForClientBounds(client_bounds).size();
}

void OpaqueFrameView::OnPaint(gfx::Canvas* canvas) {
  TRACE_EVENT0("views.frame", "OpaqueFrameView::OnPaint");
  if (untitled_widget()->IsFullscreen()) {
    return;  // Nothing is visible, so don't bother to paint.
  }

  std::optional<ui::ColorId> custom_background_color =
      untitled_widget()->GetTitleBarBackgroundColor();
  SkColor frame_color =
      custom_background_color
          ? GetColorProvider()->GetColor(custom_background_color.value())
          : GetFrameColor(BrowserFrameActiveState::kUseCurrent);
  frame_background_->set_frame_color(frame_color);
  frame_background_->set_use_custom_frame(true);
  frame_background_->set_is_active(ShouldPaintAsActive());
  frame_background_->set_theme_image(GetFrameImage());
  frame_background_->set_theme_image_inset(
      untitled_widget()->GetThemeOffsetFromBrowserView());
  frame_background_->set_theme_overlay_image(GetFrameOverlayImage());
  frame_background_->set_top_area_height(GetTopBackgroundHeight());

  // if (GetFrameButtonStyle() == FrameButtonStyle::kMdButton) {
  //   for (views::Button* button :
  //        {minimize_button_, maximize_button_, restore_button_,
  //        close_button_}) {
  //     DCHECK_EQ(std::string(views::FrameCaptionButton::kViewClassName),
  //               button->GetClassName());
  //     views::FrameCaptionButton* frame_caption_button =
  //         static_cast<views::FrameCaptionButton*>(button);
  //     frame_caption_button->SetPaintAsActive(active);
  //     frame_caption_button->SetBackgroundColor(frame_color);
  //   }
  // }

  PaintRestoredFrameBorder(canvas);
}

void OpaqueFrameView::PaintRestoredFrameBorder(gfx::Canvas* canvas) const {
  const ui::ThemeProvider* tp = GetThemeProvider();
  frame_background_->SetSideImages(
      tp->GetImageSkiaNamed(IDR_WINDOW_LEFT_SIDE),
      tp->GetImageSkiaNamed(IDR_WINDOW_TOP_CENTER),
      tp->GetImageSkiaNamed(IDR_WINDOW_RIGHT_SIDE),
      tp->GetImageSkiaNamed(IDR_WINDOW_BOTTOM_CENTER));
  frame_background_->SetCornerImages(
      tp->GetImageSkiaNamed(IDR_WINDOW_TOP_LEFT_CORNER),
      tp->GetImageSkiaNamed(IDR_WINDOW_TOP_RIGHT_CORNER),
      tp->GetImageSkiaNamed(IDR_WINDOW_BOTTOM_LEFT_CORNER),
      tp->GetImageSkiaNamed(IDR_WINDOW_BOTTOM_RIGHT_CORNER));
  frame_background_->PaintRestored(canvas, this);

  // Note: When we don't have a toolbar, we need to draw some kind of bottom
  // edge here.  Because the App Window graphics we use for this have an
  // attached client edge and their sizing algorithm is a little involved, we do
  // all this in PaintRestoredClientEdge().
}

BEGIN_METADATA(OpaqueFrameView)
END_METADATA
