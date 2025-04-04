// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_MAC_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_MAC_H_

#include "radium/browser/ui/views/frame/native_untitled_frame.h"
#include "ui/views/widget/native_widget_mac.h"

class UntitledWidget;

class DesktopUntitledFrameMac : public views::NativeWidgetMac,
                                public NativeUntitledFrame {
 public:
  explicit DesktopUntitledFrameMac(UntitledWidget* untitled_widget);
  DesktopUntitledFrameMac(const DesktopUntitledFrameMac&) = delete;
  DesktopUntitledFrameMac& operator=(const DesktopUntitledFrameMac&) = delete;

  ~DesktopUntitledFrameMac() override;

 private:
  // NativeUntitledFrame:
  views::Widget::InitParams GetWidgetParams() override;
  bool UsesNativeSystemMenu() const override;
  bool ShouldSaveWindowPlacement() const override;
  void GetWindowPlacement(
      gfx::Rect* bounds,
      ui::mojom::WindowShowState* show_state) const override;

  // views::NativeWidgetMac:
  void PopulateCreateWindowParams(
      const views::Widget::InitParams& widget_params,
      remote_cocoa::mojom::CreateWindowParams* params) override;
  void GetWindowFrameTitlebarHeight(bool* override_titlebar_height,
                                    float* titlebar_height) override;
  void OnWindowFullscreenTransitionComplete() override;

 private:
  raw_ptr<UntitledWidget> untitled_widget_;  // Weak. Our ClientView.
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_MAC_H_
