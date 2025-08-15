// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/browser_process_platform_part.h"
#include "radium/browser/lifetime/application_lifetime.h"
#include "radium/common/radium_switches.h"
#include "ui/aura/client/capture_client.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/views/buildflags.h"
#include "ui/views/widget/widget.h"

#if BUILDFLAG(ENABLE_DESKTOP_AURA)
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host.h"
#endif

#if BUILDFLAG(ENABLE_DESKTOP_AURA) && BUILDFLAG(IS_OZONE)
#include "ui/views/widget/desktop_aura/desktop_window_tree_host_platform.h"
#endif

#if BUILDFLAG(IS_WIN)
#include <windows.h>

#include "ui/views/widget/desktop_aura/desktop_window_tree_host_win.h"
#endif

namespace radium {

namespace {
#if BUILDFLAG(ENABLE_DESKTOP_AURA) && (BUILDFLAG(IS_WIN) || BUILDFLAG(IS_OZONE))
void CloseWindow(aura::Window* window) {
  if (window) {
    views::Widget* widget = views::Widget::GetWidgetForNativeView(window);
    if (widget && !widget->is_secondary_widget()) {
      widget->Close();
    }
  }
}
#endif

#if BUILDFLAG(IS_WIN)
BOOL CALLBACK WindowCallbackProc(HWND hwnd, LPARAM lParam) {
  aura::Window* root_window =
      views::DesktopWindowTreeHostWin::GetContentWindowForHWND(hwnd);
  CloseWindow(root_window);
  return TRUE;
}
#endif
}  // namespace

void CloseAllTopWidgetForPlatform() {
#if BUILDFLAG(IS_WIN)
  EnumThreadWindows(GetCurrentThreadId(), WindowCallbackProc, 0);
#endif

#if BUILDFLAG(ENABLE_DESKTOP_AURA) && BUILDFLAG(IS_OZONE)
  views::DesktopWindowTreeHostPlatform::CleanUpWindowList(CloseWindow);
#endif
}

void HandleAppExitingForPlatform() {
  // Close all non browser windows now. Those includes notifications
  // and windows created by Ash (launcher, background, etc).

  views::Widget::CloseAllWidgets();
}

}  // namespace radium
