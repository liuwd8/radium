// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/webui/webui_gallery/webui_gallery_ui.h"

#include "components/favicon_base/favicon_url_parser.h"
#include "components/strings/grit/components_strings.h"
#include "content/public/browser/url_data_source.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/ui/webui/favicon_source.h"
#include "radium/browser/ui/webui/webui_util.h"
#include "radium/common/webui_url_constants.h"
#include "radium/grit/webui_gallery_resources.h"
#include "radium/grit/webui_gallery_resources_map.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "ui/base/ui_base_features.h"
#include "ui/base/webui/web_ui_util.h"
#include "ui/webui/color_change_listener/color_change_handler.h"

// Special case. Otherwise we shouldn't depend on any part of //chrome
#include "chrome/grit/side_panel_shared_resources.h"
#include "chrome/grit/side_panel_shared_resources_map.h"

namespace {

void CreateAndAddWebuiGalleryUIHtmlSource(Profile* profile) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, radium::kRadiumUIWebuiGalleryHost);

  radium::webui::SetupWebUIDataSource(source, kWebuiGalleryResources,
                                      IDR_WEBUI_GALLERY_WEBUI_GALLERY_HTML);

  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameSrc, "frame-src 'self';");
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameAncestors,
      "frame-ancestors 'self';");

  // TODO(colehorvitz): Promote to a place where it can be easily registered
  // by many WebUIs.
  source->AddString("opensInNewTab", "Opens in new tab");
  source->AddLocalizedString("backButton", IDS_ACCNAME_BACK);

  // Add shared SidePanel resources so that those elements can be demonstrated
  // as well.
  source->AddResourcePaths(kSidePanelSharedResources);

  content::URLDataSource::Add(
      profile, std::make_unique<FaviconSource>(
                   profile, chrome::FaviconUrlFormat::kFavicon2));
}

}  // namespace

WebuiGalleryUI::WebuiGalleryUI(content::WebUI* web_ui)
    : ui::MojoWebUIController(web_ui, false) {
  CreateAndAddWebuiGalleryUIHtmlSource(Profile::FromWebUI(web_ui));
}

WebuiGalleryUI::~WebuiGalleryUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(WebuiGalleryUI)

void WebuiGalleryUI::BindInterface(
    mojo::PendingReceiver<color_change_listener::mojom::PageHandler>
        pending_receiver) {
  color_provider_handler_ = std::make_unique<ui::ColorChangeHandler>(
      web_ui()->GetWebContents(), std::move(pending_receiver));
}
