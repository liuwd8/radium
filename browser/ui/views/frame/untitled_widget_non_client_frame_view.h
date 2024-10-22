// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_NON_CLIENT_FRAME_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_NON_CLIENT_FRAME_VIEW_H_

#include "ui/views/layout/delegating_layout_manager.h"
#include "ui/views/window/non_client_view.h"

class UntitledWidget;
class UntitledWindowClientView;

class UntitledWidgetNonClientFrameView : public views::NonClientFrameView {
  METADATA_HEADER(UntitledWidgetNonClientFrameView, views::NonClientFrameView)

 public:
  static std::unique_ptr<UntitledWidgetNonClientFrameView> Create(
      views::Widget* widget);

  explicit UntitledWidgetNonClientFrameView(views::Widget* widget);
  UntitledWidgetNonClientFrameView(const UntitledWidgetNonClientFrameView&) =
      delete;
  UntitledWidgetNonClientFrameView& operator=(
      const UntitledWidgetNonClientFrameView&) = delete;

  ~UntitledWidgetNonClientFrameView() override;

  bool IsFrameCondensed() const;

  // Returns the insets from the edge of the native window to the client view in
  // DIPs. The value is left-to-right even on RTL locales. That is,
  // insets.left() will be on the left in screen coordinates.
  virtual gfx::Insets RestoredMirroredFrameBorderInsets() const;

  // Returns the insets from the client view to the input region. The returned
  // insets will be negative, such that view_rect.Inset(GetInputInsets()) will
  // be the input region.
  virtual gfx::Insets GetInputInsets() const;

  // Gets the rounded-rect that will be used to clip the window frame when
  // drawing. The region will be as if the window was restored, and will be in
  // view coordinates.
  virtual SkRRect GetRestoredClipRegion() const;

  // Returns the height of the top frame.  This value will be 0 if the
  // compositor doesn't support translucency, if the top frame is not
  // translucent, or if the window is in full screen mode.
  virtual int GetTranslucentTopAreaHeight() const;

 protected:
  // View:
  gfx::Size GetMinimumSize() const override;
  gfx::Size GetMaximumSize() const override;

 private:
  raw_ptr<views::Widget> widget_ = nullptr;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_NON_CLIENT_FRAME_VIEW_H_
