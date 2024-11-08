// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_WEBUI_WEBUI_GALLERY_WEBUI_GALLERY_UI_H_
#define RADIUM_BROWSER_UI_WEBUI_WEBUI_GALLERY_WEBUI_GALLERY_UI_H_

#include "radium/browser/ui/webui/radium_webui_config.h"
#include "radium/common/webui_url_constants.h"
#include "ui/webui/mojo_web_ui_controller.h"
#include "ui/webui/resources/cr_components/color_change_listener/color_change_listener.mojom.h"

namespace content {
class WebUI;
}

namespace ui {
class ColorChangeHandler;
}

class WebuiGalleryUI;

class WebuiGalleryUIConfig : public DefaultRadiumWebUIConfig<WebuiGalleryUI> {
 public:
  WebuiGalleryUIConfig()
      : DefaultRadiumWebUIConfig(radium::kRadiumUIScheme,
                                 radium::kRadiumUIWebuiGalleryHost) {}
};

// The Web UI controller for the chrome://webui-gallery page.
class WebuiGalleryUI : public ui::MojoWebUIController {
 public:
  explicit WebuiGalleryUI(content::WebUI* web_ui);
  ~WebuiGalleryUI() override;

  WebuiGalleryUI(const WebuiGalleryUI&) = delete;
  WebuiGalleryUI& operator=(const WebuiGalleryUI&) = delete;

  void BindInterface(
      mojo::PendingReceiver<color_change_listener::mojom::PageHandler>
          pending_receiver);

  static constexpr std::string GetWebUIName() { return "WebUIGallery"; }

 private:
  std::unique_ptr<ui::ColorChangeHandler> color_provider_handler_;

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // RADIUM_BROWSER_UI_WEBUI_WEBUI_GALLERY_WEBUI_GALLERY_UI_H_
