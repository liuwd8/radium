// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_WIN_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_WIN_H_

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

class UntitledWidgetFrameViewWin : public UntitledWidgetNonClientFrameView {
  METADATA_HEADER(UntitledWidgetFrameViewWin, UntitledWidgetNonClientFrameView)
 public:
  explicit UntitledWidgetFrameViewWin(UntitledWidget* untitled_widget);
  UntitledWidgetFrameViewWin(const UntitledWidgetFrameViewWin&) = delete;
  UntitledWidgetFrameViewWin& operator=(const UntitledWidgetFrameViewWin&) =
      delete;

  ~UntitledWidgetFrameViewWin() override;

  // views::NonClientFrameView:
  gfx::Rect GetBoundsForClientView() const override;
  gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const override;
  int NonClientHitTest(const gfx::Point& point) override;

 private:
  // views::View:
  void OnPaint(gfx::Canvas* canvas) override;

  // Returns the thickness of the window border for the left, right, and bottom
  // edges of the frame. On Windows 10 this is a mostly-transparent handle that
  // allows you to resize the window.
  int FrameBorderThickness() const;

  gfx::Insets RestoredFrameBorderInsets() const;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_WIN_H_
