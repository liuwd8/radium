// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_MAC_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_MAC_H_

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

class UntitledWidgetFrameViewMac : public UntitledWidgetNonClientFrameView {
  METADATA_HEADER(UntitledWidgetFrameViewMac, UntitledWidgetNonClientFrameView)
 public:
  explicit UntitledWidgetFrameViewMac(UntitledWidget* untitled_widget);
  UntitledWidgetFrameViewMac(const UntitledWidgetFrameViewMac&) = delete;
  UntitledWidgetFrameViewMac& operator=(const UntitledWidgetFrameViewMac&) =
      delete;

  ~UntitledWidgetFrameViewMac() override;

 private:
  // UntitledWidgetNonClientFrameView:
  bool CaptionButtonsOnLeadingEdge() const override;
  gfx::Insets GetCaptionButtonInsets() const override;

  // views::NonClientFrameView:
  gfx::Rect GetBoundsForClientView() const override;
  gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const override;
  int NonClientHitTest(const gfx::Point& point) override;
  void OnPaint(gfx::Canvas* canvas) override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_MAC_H_
