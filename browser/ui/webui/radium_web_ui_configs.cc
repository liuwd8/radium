// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/webui/radium_web_ui_configs.h"

#include "build/build_config.h"
#include "radium/browser/ui/webui/example/example_ui.h"
#include "radium/browser/ui/webui/radium_webui_config_map.h"

#if !BUILDFLAG(IS_ANDROID)
#include "radium/browser/ui/webui/webui_gallery/webui_gallery_ui.h"
#endif  // BUILDFLAG(IS_ANDROID)

void RegisterRadiumWebUIConfigs() {
  auto& map = RadiumWebUIConfigMap::GetInstance();
  map.AddWebUIConfig(std::make_unique<ExampleUIConfig>());

#if !BUILDFLAG(IS_ANDROID)
  map.AddWebUIConfig(std::make_unique<WebuiGalleryUIConfig>());
#endif  // BUILDFLAG(IS_ANDROID)
}
