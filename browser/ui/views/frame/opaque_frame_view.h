// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_OPAQUE_FRAME_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_OPAQUE_FRAME_VIEW_H_

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "ui/views/layout/delegating_layout_manager.h"

namespace views {
class FrameBackground;
}  // namespace views

class OpaqueFrameView : public UntitledWidgetNonClientFrameView {
  METADATA_HEADER(OpaqueFrameView, UntitledWidgetNonClientFrameView)

 public:
  // The frame border is only visible in restored mode and is hardcoded to 4 px
  // on each side regardless of the system window border size.  This is
  // overridable by subclasses, so RestoredFrameBorderInsets() should be used
  // instead of using this constant directly.
  static constexpr int kFrameBorderThickness = 4;

  explicit OpaqueFrameView(UntitledWidget* untitled_widget);
  OpaqueFrameView(const OpaqueFrameView&) = delete;
  OpaqueFrameView& operator=(const OpaqueFrameView&) = delete;

  ~OpaqueFrameView() override;

  static constexpr gfx::Insets RestoredFrameBorderInsets() {
    return gfx::Insets(kFrameBorderThickness);
  }

  // 注意和GetInsets() 的区别,
  // 它主要是用于顶部圆角边框的保留。避免内容覆盖圆角区域
  int GetTopBackgroundHeight() const;

 protected:
  // views::View:
  // GetInsets() 是阴影边框区域
  gfx::Insets GetInsets() const override;
  void OnPaint(gfx::Canvas* canvas) override;

  // views::NonClientFrameView:
  gfx::Rect GetBoundsForClientView() const override;
  gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const override;
  int NonClientHitTest(const gfx::Point& point) override;

  // Paint various sub-components of this view.  The *FrameBorder() functions
  // also paint the background of the titlebar area, since the top frame border
  // and titlebar background are a contiguous component.
  virtual void PaintRestoredFrameBorder(gfx::Canvas* canvas) const;

  views::FrameBackground* frame_background() const {
    return frame_background_.get();
  }

 private:
  // Background painter for the window frame.
  std::unique_ptr<views::FrameBackground> frame_background_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_OPAQUE_FRAME_VIEW_H_
