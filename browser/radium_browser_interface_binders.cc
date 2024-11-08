// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_interface_binders.h"

#include "content/public/browser/web_ui_browser_interface_broker_registry.h"
#include "content/public/browser/web_ui_controller_interface_binder.h"

#if !BUILDFLAG(IS_ANDROID)
#include "radium/browser/ui/webui/webui_gallery/webui_gallery_ui.h"
#include "ui/webui/resources/cr_components/color_change_listener/color_change_listener.mojom.h"
#endif  // BUILDFLAG(IS_ANDROID)

namespace radium::internal {
using content::RegisterWebUIControllerInterfaceBinder;

void PopulateRadiumFrameBinders(
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map,
    content::RenderFrameHost* render_frame_host) {}

void PopulateRadiumWebUIFrameBinders(
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map,
    content::RenderFrameHost* render_frame_host) {}

void PopulateRadiumWebUIFrameInterfaceBrokers(
    content::WebUIBrowserInterfaceBrokerRegistry& registry) {
#if !BUILDFLAG(IS_ANDROID)
  registry.ForWebUI<WebuiGalleryUI>()
      .Add<color_change_listener::mojom::PageHandler>();
#endif  // BUILDFLAG(IS_ANDROID)
}

}  // namespace radium::internal
