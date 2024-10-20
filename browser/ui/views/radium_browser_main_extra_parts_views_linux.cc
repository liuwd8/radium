// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_browser_main_extra_parts_views_linux.h"
#include "ui/base/cursor/cursor_factory.h"
#include "ui/linux/linux_ui.h"
#include "ui/linux/linux_ui_factory.h"
#include "ui/linux/linux_ui_getter.h"

namespace {

class LinuxUiGetterImpl : public ui::LinuxUiGetter {
 public:
  LinuxUiGetterImpl() = default;
  ~LinuxUiGetterImpl() override = default;
  ui::LinuxUiTheme* GetForWindow(aura::Window* window) override {
    // return window ? GetForProfile(GetThemeProfileForWindow(window)) :
    // nullptr;
    // TODO:
    return nullptr;
  }
  ui::LinuxUiTheme* GetForProfile(Profile* profile) override {
    // return ui::GetLinuxUiTheme(
    //     ThemeServiceAuraLinux::GetSystemThemeForProfile(profile));
    // TODO:
    return nullptr;
  }
};

}  // namespace

RadiumBrowserMainExtraPartsViewsLinux::RadiumBrowserMainExtraPartsViewsLinux() =
    default;
RadiumBrowserMainExtraPartsViewsLinux::
    ~RadiumBrowserMainExtraPartsViewsLinux() = default;

void RadiumBrowserMainExtraPartsViewsLinux::ToolkitInitialized() {
  RadiumBrowserMainExtraPartsViews::ToolkitInitialized();

  if (auto* linux_ui = ui::GetDefaultLinuxUi()) {
    linux_ui_getter_ = std::make_unique<LinuxUiGetterImpl>();
    ui::LinuxUi::SetInstance(linux_ui);

    // Cursor theme changes are tracked by LinuxUI (via a CursorThemeManager
    // implementation). Start observing them once it's initialized.
    ui::CursorFactory::GetInstance()->ObserveThemeChanges();
  }
#if defined(USE_DBUS)
  dark_mode_manager_ = std::make_unique<ui::DarkModeManagerLinux>();
#endif
}

void RadiumBrowserMainExtraPartsViewsLinux::PreCreateThreads() {
  RadiumBrowserMainExtraPartsViews::PreCreateThreads();
  // We could do that during the ToolkitInitialized call, which is called before
  // this method, but the display::Screen is only created after PreCreateThreads
  // is called. Thus, do that here instead.
  display_observer_.emplace(this);
}

void RadiumBrowserMainExtraPartsViewsLinux::OnCurrentWorkspaceChanged(
    const std::string& new_workspace) {
  // BrowserList::MoveBrowsersInWorkspaceToFront(new_workspace);
}
