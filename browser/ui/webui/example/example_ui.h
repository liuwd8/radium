// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_WEBUI_EXAMPLE_EXAMPLE_UI_H_
#define RADIUM_BROWSER_UI_WEBUI_EXAMPLE_EXAMPLE_UI_H_

#include "radium/browser/ui/webui/radium_webui_config.h"
#include "radium/common/webui_url_constants.h"
#include "ui/webui/mojo_web_ui_controller.h"

class ExampleUI;

class ExampleUIConfig : public DefaultRadiumWebUIConfig<ExampleUI> {
 public:
  ExampleUIConfig()
      : DefaultRadiumWebUIConfig(radium::kRadiumUIScheme,
                                 radium::kRadiumUIExampleHost) {}
};

class ExampleUI : public ui::MojoWebUIController {
 public:
  explicit ExampleUI(content::WebUI* web_ui);
  ExampleUI(const ExampleUI&) = delete;
  ExampleUI& operator=(const ExampleUI&) = delete;

  ~ExampleUI() override;

  static constexpr std::string GetWebUIName() { return "Example"; }
};

#endif  // RADIUM_BROWSER_UI_WEBUI_EXAMPLE_EXAMPLE_UI_H_
