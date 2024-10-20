// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_BROWSER_MAIN_EXTRA_PARTS_VIEWS_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_BROWSER_MAIN_EXTRA_PARTS_VIEWS_H_

#include <memory>

#include "build/build_config.h"
#include "radium/browser/radium_browser_main_extra_parts.h"

namespace views {
class ViewsDelegate;
class LayoutProvider;
}  // namespace views

namespace ui_devtools {
class UiDevToolsServer;
}

#if defined(USE_AURA)
namespace display {
class Screen;
}

namespace wm {
class WMState;
}
#endif

class DevtoolsProcessObserver;

class RadiumBrowserMainExtraPartsViews : public RadiumBrowserMainExtraParts {
 public:
  explicit RadiumBrowserMainExtraPartsViews();
  RadiumBrowserMainExtraPartsViews(const RadiumBrowserMainExtraPartsViews&) =
      delete;
  RadiumBrowserMainExtraPartsViews& operator=(
      const RadiumBrowserMainExtraPartsViews&) = delete;

  ~RadiumBrowserMainExtraPartsViews() override;

  // RadiumBrowserMainExtraParts:
  void ToolkitInitialized() override;
  void PreCreateThreads() override;
  void PreProfileInit() override;
  void PostBrowserStart() override;
  void PostMainMessageLoopRun() override;

 private:
  void CreateUiDevTools();
  const ui_devtools::UiDevToolsServer* GetUiDevToolsServerInstance();
  void DestroyUiDevTools();

  std::unique_ptr<views::ViewsDelegate> views_delegate_;
  std::unique_ptr<views::LayoutProvider> layout_provider_;

  // Only used when running in --enable-ui-devtools.
  std::unique_ptr<ui_devtools::UiDevToolsServer> devtools_server_;
  std::unique_ptr<DevtoolsProcessObserver> devtools_process_observer_;

#if defined(USE_AURA)
  std::unique_ptr<display::Screen> screen_;
  std::unique_ptr<wm::WMState> wm_state_;
#endif
};

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_BROWSER_MAIN_EXTRA_PARTS_VIEWS_H_
