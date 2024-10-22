// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_LINUX_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_LINUX_H_

#include "radium/browser/ui/views/frame/untitled_desktop_window_tree_host.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host_linux.h"

class UntitledDesktopWindowTreeHostLinux
    : public UntitledDesktopWindowTreeHost,
      public views::DesktopWindowTreeHostLinux {
 public:
  explicit UntitledDesktopWindowTreeHostLinux(
      views::internal::NativeWidgetDelegate* native_widget_delegate,
      views::DesktopNativeWidgetAura* desktop_native_widget_aura,
      UntitledWidget* untitled_widget);
  UntitledDesktopWindowTreeHostLinux(
      const UntitledDesktopWindowTreeHostLinux&) = delete;
  UntitledDesktopWindowTreeHostLinux& operator=(
      const UntitledDesktopWindowTreeHostLinux&) = delete;

  ~UntitledDesktopWindowTreeHostLinux() override;

  // Returns true if the system supports client-drawn shadows.  We may still
  // choose not to draw a shadow eg. when the "system titlebar and borders"
  // setting is enabled, or when the window is maximized/fullscreen.
  bool SupportsClientFrameShadow() const;

 private:
  // UntitledDesktopWindowTreeHost:
  DesktopWindowTreeHost* AsDesktopWindowTreeHost() override;
  int GetMinimizeButtonOffset() const override;
  bool UsesNativeSystemMenu() const override;

  // views::DesktopWindowTreeHostLinux:
  void AddAdditionalInitProperties(
      const views::Widget::InitParams& params,
      ui::PlatformWindowInitProperties* properties) override;
  void FrameTypeChanged() override;
  void UpdateFrameHints() override;
  gfx::Insets CalculateInsetsInDIP(
      ui::PlatformWindowState window_state) const override;
  void OnWindowStateChanged(ui::PlatformWindowState old_state,
                            ui::PlatformWindowState new_state) override;
  void OnWindowTiledStateChanged(ui::WindowTiledEdges new_tiled_edges) override;

 private:
  raw_ptr<UntitledWidget> untitled_widget_ = nullptr;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_LINUX_H_
