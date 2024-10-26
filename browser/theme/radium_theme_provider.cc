// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/theme/radium_theme_provider.h"

#include "base/containers/flat_tree.h"
#include "radium/grit/theme_resources_map.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/image/image_skia.h"

RadiumThemeProvider::RadiumThemeProvider() = default;
RadiumThemeProvider::~RadiumThemeProvider() = default;

gfx::ImageSkia* RadiumThemeProvider::GetImageSkiaNamed(int id) const {
  gfx::Image image = GetImageNamed(id);
  if (image.IsEmpty()) {
    return nullptr;
  }

  return const_cast<gfx::ImageSkia*>(image.ToImageSkia());
}

color_utils::HSL RadiumThemeProvider::GetTint(int original_id) const {
  return {-1, -1, -1};
}

int RadiumThemeProvider::GetDisplayProperty(int id) const {
  return -1;
}

bool RadiumThemeProvider::ShouldUseNativeFrame() const {
  return false;
}

bool RadiumThemeProvider::HasCustomImage(int id) const {
  return base::ranges::find_if(
      kThemeResources,
      [id](const webui::ResourcePath& path) { return path.id == id; });
}

base::RefCountedMemory* RadiumThemeProvider::GetRawData(
    int id,
    ui::ResourceScaleFactor scale_factor) const {
  return nullptr;
}

gfx::Image RadiumThemeProvider::GetImageNamed(int id) const {
  return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(id);
}
