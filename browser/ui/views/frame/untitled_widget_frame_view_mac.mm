// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_frame_view_mac.h"

#include <AppKit/AppKit.h>

#include "base/i18n/rtl.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/canvas.h"

namespace {

// Empirical measurements of the traffic lights.
constexpr int kCaptionButtonsWidth = 52;
constexpr int kCaptionButtonsLeadingPadding = 20;

}  // namespace

UntitledWidgetFrameViewMac::UntitledWidgetFrameViewMac(
    UntitledWidget* untitled_widget)
    : UntitledWidgetNonClientFrameView(untitled_widget) {}

UntitledWidgetFrameViewMac::~UntitledWidgetFrameViewMac() = default;

bool UntitledWidgetFrameViewMac::CaptionButtonsOnLeadingEdge() const {
  // In "partial" RTL mode (where the OS is in LTR mode while Chrome is in RTL
  // mode, or vice versa), the traffic lights are on the trailing edge rather
  // than the leading edge.
  return base::i18n::IsRTL() == (NSApp.userInterfaceLayoutDirection ==
                                 NSUserInterfaceLayoutDirectionRightToLeft);
}

gfx::Insets UntitledWidgetFrameViewMac::GetCaptionButtonInsets() const {
  const int kCaptionButtonInset =
      kCaptionButtonsWidth + kCaptionButtonsLeadingPadding * 2;
  if (CaptionButtonsOnLeadingEdge()) {
    return gfx::Insets::TLBR(0, kCaptionButtonInset, 0, 0);
  } else {
    return gfx::Insets::TLBR(0, 0, 0, kCaptionButtonInset);
  }
}

gfx::Rect UntitledWidgetFrameViewMac::GetBoundsForClientView() const {
  return bounds();
}

gfx::Rect UntitledWidgetFrameViewMac::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  // If the operating system is handling drawing the window titlebar then the
  // titlebar height will not be included in |GetTopInset|, so we have to
  // explicitly add it. If a custom titlebar is being drawn, this calculation
  // will be zero.
  NSWindow* window = GetWidget()->GetNativeWindow().GetNativeNSWindow();
  int top_inset = window.frame.size.height -
                  [window contentRectForFrameRect:window.frame].size.height;

  return gfx::Rect(client_bounds.x(), client_bounds.y() - top_inset,
                   client_bounds.width(), client_bounds.height() + top_inset);
}

int UntitledWidgetFrameViewMac::NonClientHitTest(const gfx::Point& point) {
  int super_component =
      UntitledWidgetNonClientFrameView::NonClientHitTest(point);
  if (super_component != HTNOWHERE) {
    return super_component;
  }

  int delegate_component =
      untitled_widget()->delegate()->NonClientHitTest(point);
  if (delegate_component != HTNOWHERE) {
    return delegate_component;
  }

  // BrowserView::NonClientHitTest will return HTNOWHERE for points that hit
  // the native title bar. On Mac, we need to explicitly return HTCAPTION for
  // those points.
  const int component =
      untitled_widget()->client_view()->NonClientHitTest(point);
  return (component == HTNOWHERE && bounds().Contains(point)) ? HTCAPTION
                                                              : component;
}

void UntitledWidgetFrameViewMac::OnPaint(gfx::Canvas* canvas) {
  SkColor frame_color = GetFrameColor(BrowserFrameActiveState::kUseCurrent);
  canvas->DrawColor(frame_color);
}

BEGIN_METADATA(UntitledWidgetFrameViewMac)
END_METADATA
