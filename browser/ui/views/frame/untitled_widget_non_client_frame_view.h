// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_NON_CLIENT_FRAME_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_NON_CLIENT_FRAME_VIEW_H_

#include "ui/views/layout/delegating_layout_manager.h"
#include "ui/views/window/non_client_view.h"

class UntitledWidget;
class UntitledWindowClientView;

// Type used for functions whose return values depend on the active state of
// the frame.
enum class BrowserFrameActiveState {
  kUseCurrent,  // Use current frame active state.
  kActive,      // Treat frame as active regardless of current state.
  kInactive,    // Treat frame as inactive regardless of current state.
};

class UntitledWidgetNonClientFrameView : public views::NonClientFrameView {
  METADATA_HEADER(UntitledWidgetNonClientFrameView, views::NonClientFrameView)

 public:
  static std::unique_ptr<UntitledWidgetNonClientFrameView> Create(
      UntitledWidget* untitled_widget);

  explicit UntitledWidgetNonClientFrameView(UntitledWidget* untitled_widget);
  UntitledWidgetNonClientFrameView(const UntitledWidgetNonClientFrameView&) =
      delete;
  UntitledWidgetNonClientFrameView& operator=(
      const UntitledWidgetNonClientFrameView&) = delete;

  ~UntitledWidgetNonClientFrameView() override;

  using views::NonClientFrameView::ShouldPaintAsActive;

  UntitledWidget* untitled_widget() const { return untitled_widget_; }

  bool IsFrameCondensed() const;

  // Computes the height of the top area of the frame.
  virtual int GetTopAreaHeight() const;

  // Returns whether the caption buttons are drawn at the leading edge (i.e. the
  // left in LTR mode, or the right in RTL mode).
  virtual bool CaptionButtonsOnLeadingEdge() const;

  // Creates an inset from the caption button size which controls for which edge
  // the captions buttons exists on. Used to position the tab strip region view
  // and the caption button placeholder container. Returns the distance from the
  // leading edge of the frame to the first tab in the tabstrip not including
  // the corner radius.
  virtual gfx::Insets GetCaptionButtonInsets() const;

  // Returns the color of the browser frame, which is also the color of the
  // tabstrip background.
  virtual SkColor GetFrameColor(BrowserFrameActiveState active_state) const;

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

  // Compute aspects of the frame needed to paint the frame background.
  gfx::ImageSkia GetFrameImage(BrowserFrameActiveState active_state =
                                   BrowserFrameActiveState::kUseCurrent) const;
  gfx::ImageSkia GetFrameOverlayImage(
      BrowserFrameActiveState active_state =
          BrowserFrameActiveState::kUseCurrent) const;

 protected:
  // View:
  gfx::Size GetMinimumSize() const override;
  gfx::Size GetMaximumSize() const override;

  // Called when |frame_|'s "paint as active" state has changed.
  virtual void PaintAsActiveChanged();

  // Converts an ActiveState to a bool representing whether the frame should be
  // treated as active.
  bool ShouldPaintAsActive(BrowserFrameActiveState active_state) const;

 private:
  raw_ptr<UntitledWidget> untitled_widget_ = nullptr;

  base::CallbackListSubscription paint_as_active_subscription_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_NON_CLIENT_FRAME_VIEW_H_
