// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_AURA_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_AURA_H_

#include "radium/browser/ui/views/frame/native_untitled_frame.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"

class UntitledDesktopWindowTreeHost;
class UntitledWidget;
class UntitledWindowClientView;

class DesktopUntitledFrameAura : public views::DesktopNativeWidgetAura,
                                 public NativeUntitledFrame {
 public:
  explicit DesktopUntitledFrameAura(UntitledWidget* untitled_widget);
  DesktopUntitledFrameAura(const DesktopUntitledFrameAura&) = delete;
  DesktopUntitledFrameAura& operator=(const DesktopUntitledFrameAura&) = delete;

 protected:
  ~DesktopUntitledFrameAura() override;

  // views::DesktopNativeWidgetAura:
  void OnHostClosed() override;
  void InitNativeWidget(views::Widget::InitParams params) override;

  // NativeBrowserFrame:
  views::Widget::InitParams GetWidgetParams() override;
  bool UsesNativeSystemMenu() const override;
  bool ShouldSaveWindowPlacement() const override;
  void GetWindowPlacement(
      gfx::Rect* bounds,
      ui::mojom::WindowShowState* show_state) const override;

 private:
  raw_ptr<UntitledWidget> untitled_widget_ = nullptr;

  // Owned by the RootWindow.
  raw_ptr<UntitledDesktopWindowTreeHost> browser_desktop_window_tree_host_ =
      nullptr;

  std::unique_ptr<wm::VisibilityController> visibility_controller_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_AURA_H_
