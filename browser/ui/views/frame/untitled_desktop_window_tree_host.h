// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_H_

class UntitledWidget;

namespace views {
class DesktopNativeWidgetAura;
class DesktopWindowTreeHost;
namespace internal {
class NativeWidgetDelegate;
}
}  // namespace views

class UntitledDesktopWindowTreeHost {
 public:
  virtual ~UntitledDesktopWindowTreeHost() = default;

  // BDRWH is owned by the RootWindow.
  static UntitledDesktopWindowTreeHost* CreateBrowserDesktopWindowTreeHost(
      views::internal::NativeWidgetDelegate* native_widget_delegate,
      views::DesktopNativeWidgetAura* desktop_native_widget_aura,
      UntitledWidget* untitled_widget);

  virtual views::DesktopWindowTreeHost* AsDesktopWindowTreeHost() = 0;

  virtual int GetMinimizeButtonOffset() const = 0;

  // Returns true if the OS takes care of showing the system menu. Returning
  // false means BrowserFrame handles showing the system menu.
  virtual bool UsesNativeSystemMenu() const = 0;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_DESKTOP_WINDOW_TREE_HOST_H_
