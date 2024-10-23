// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_LINUX_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_LINUX_H_

#include "radium/browser/ui/views/frame/opaque_frame_view.h"

class UntitledWidgetFrameViewLinux : public OpaqueFrameView {
 public:
  explicit UntitledWidgetFrameViewLinux(UntitledWidget* untitled_widget);
  UntitledWidgetFrameViewLinux(const UntitledWidgetFrameViewLinux&) = delete;
  UntitledWidgetFrameViewLinux& operator=(const UntitledWidgetFrameViewLinux&) =
      delete;
  ~UntitledWidgetFrameViewLinux() override;

  // Gets the shadow metrics (radius, offset, and number of shadows).  This will
  // always return shadow values, even if shadows are not actually drawn.
  // `active` indicates if the shadow will be drawn on a focused browser window.
  static gfx::ShadowValues GetShadowValues(bool active);

  // UntitledWidgetNonClientFrameView:
  gfx::Insets RestoredMirroredFrameBorderInsets() const override;
  gfx::Insets GetInputInsets() const override;
  SkRRect GetRestoredClipRegion() const override;
  void PaintRestoredFrameBorder(gfx::Canvas* canvas) const override;

  bool ShouldDrawRestoredFrameShadow() const;

 private:
  // View:
  gfx::Insets GetInsets() const override;

  // Gets the radius of the top corners when the window is restored.  The
  // returned value is in DIPs.  The result will be 0 if rounded corners are
  // disabled (eg. if the compositor doesn't support translucency.)
  virtual float GetRestoredCornerRadiusDip() const;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_FRAME_VIEW_LINUX_H_
