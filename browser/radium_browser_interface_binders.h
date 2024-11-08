// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_INTERFACE_BINDERS_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_INTERFACE_BINDERS_H_

#include "mojo/public/cpp/bindings/binder_map.h"

namespace content {
class RenderFrameHost;
class WebUIBrowserInterfaceBrokerRegistry;
}  // namespace content

namespace radium::internal {

// The mechanism implemented by the PopulateRadium*FrameBinders() functions
// below will replace interface registries and binders used for handling
// InterfaceProvider's GetInterface() calls (see crbug.com/718652).

// PopulateRadiumFrameBinders() registers BrowserInterfaceBroker's
// GetInterface() handler callbacks for radium-specific document-scoped
// interfaces.
void PopulateRadiumFrameBinders(
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map,
    content::RenderFrameHost* render_frame_host);

// PopulateRadiumWebUIFrameBinders() registers BrowserInterfaceBroker's
// GetInterface() handler callbacks for radium-specific document-scoped
// interfaces used from WebUI pages (e.g. Radium://bluetooth-internals).
void PopulateRadiumWebUIFrameBinders(
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map,
    content::RenderFrameHost* render_frame_host);

// PopulateRadiumWebUIFrameInterfaceBrokers registers BrowserInterfaceBrokers
// for each WebUI, these brokers are used to handle that WebUI's JavaScript
// Mojo.bindInterface calls.
void PopulateRadiumWebUIFrameInterfaceBrokers(
    content::WebUIBrowserInterfaceBrokerRegistry& registry);

}  // namespace radium::internal

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_INTERFACE_BINDERS_H_
