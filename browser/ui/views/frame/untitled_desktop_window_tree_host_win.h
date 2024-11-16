// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_WIN_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_WIN_H_

#include "radium/browser/ui/views/frame/untitled_desktop_window_tree_host.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host_win.h"

class VirtualDesktopHelper;

class UntitledDesktopWindowTreeHostWin
    : public UntitledDesktopWindowTreeHost,
      public views::DesktopWindowTreeHostWin {
 public:
  explicit UntitledDesktopWindowTreeHostWin(
      views::internal::NativeWidgetDelegate* native_widget_delegate,
      views::DesktopNativeWidgetAura* desktop_native_widget_aura,
      UntitledWidget* untitled_widget);
  UntitledDesktopWindowTreeHostWin(const UntitledDesktopWindowTreeHostWin&) =
      delete;
  UntitledDesktopWindowTreeHostWin& operator=(
      const UntitledDesktopWindowTreeHostWin&) = delete;

  ~UntitledDesktopWindowTreeHostWin() override;

 private:
  // UntitledDesktopWindowTreeHost:
  DesktopWindowTreeHost* AsDesktopWindowTreeHost() override;
  int GetMinimizeButtonOffset() const override;
  bool UsesNativeSystemMenu() const override;

  // views::DesktopWindowTreeHostWin:
  void Init(const views::Widget::InitParams& params) override;
  void Show(ui::mojom::WindowShowState show_state,
            const gfx::Rect& restore_bounds) override;
  std::string GetWorkspace() const override;
  int GetInitialShowState() const override;
  bool GetClientAreaInsets(gfx::Insets* insets,
                           HMONITOR monitor) const override;
  bool GetDwmFrameInsetsInPixels(gfx::Insets* insets) const override;
  bool PreHandleMSG(UINT message,
                    WPARAM w_param,
                    LPARAM l_param,
                    LRESULT* result) override;
  void PostHandleMSG(UINT message, WPARAM w_param, LPARAM l_param) override;

  // Kicks off an asynchronous update of |workspace_|, and notifies
  // WindowTreeHost of its value.
  void UpdateWorkspace();

 private:
  raw_ptr<UntitledWidget> untitled_widget_ = nullptr;

  // This will be null pre Win10.
  scoped_refptr<VirtualDesktopHelper> virtual_desktop_helper_;

  base::WeakPtrFactory<UntitledDesktopWindowTreeHostWin> weak_factory_{this};
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_WIN_H_
