// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_BROWSER_MAIN_EXTRA_PARTS_VIEWS_LINUX_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_BROWSER_MAIN_EXTRA_PARTS_VIEWS_LINUX_H_

#include <memory>

#include "build/build_config.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views.h"
#include "ui/display/display_observer.h"

namespace ui {
class LinuxUiGetter;
#if defined(USE_DBUS)
class DarkModeManagerLinux;
#endif
}  // namespace ui

class RadiumBrowserMainExtraPartsViewsLinux
    : public RadiumBrowserMainExtraPartsViews,
      public display::DisplayObserver {
 public:
  explicit RadiumBrowserMainExtraPartsViewsLinux();
  RadiumBrowserMainExtraPartsViewsLinux(
      const RadiumBrowserMainExtraPartsViewsLinux&) = delete;
  RadiumBrowserMainExtraPartsViewsLinux& operator=(
      const RadiumBrowserMainExtraPartsViewsLinux&) = delete;

  ~RadiumBrowserMainExtraPartsViewsLinux() override;

 private:
  // RadiumBrowserMainExtraPartsViews:
  void ToolkitInitialized() override;
  void PreCreateThreads() override;

  // display::DisplayObserver:
  void OnCurrentWorkspaceChanged(const std::string& new_workspace) override;

  std::optional<display::ScopedDisplayObserver> display_observer_;

  std::unique_ptr<ui::LinuxUiGetter> linux_ui_getter_;
#if defined(USE_DBUS)
  std::unique_ptr<ui::DarkModeManagerLinux> dark_mode_manager_;
#endif
};

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_BROWSER_MAIN_EXTRA_PARTS_VIEWS_LINUX_H_
