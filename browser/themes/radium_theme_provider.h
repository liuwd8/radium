// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_THEMES_RADIUM_THEME_PROVIDER_H_
#define RADIUM_BROWSER_THEMES_RADIUM_THEME_PROVIDER_H_

#include "ui/base/theme_provider.h"

namespace gfx {
class ImageSkia;
class Image;
}  // namespace gfx

class RadiumThemeProvider : public ui::ThemeProvider {
 public:
  explicit RadiumThemeProvider(bool incognito);
  RadiumThemeProvider(const RadiumThemeProvider&) = delete;
  RadiumThemeProvider& operator=(const RadiumThemeProvider&) = delete;

  ~RadiumThemeProvider() override;

  // ui::ThemeProvider:
  gfx::ImageSkia* GetImageSkiaNamed(int id) const override;
  color_utils::HSL GetTint(int original_id) const override;
  int GetDisplayProperty(int id) const override;
  bool ShouldUseNativeFrame() const override;
  bool HasCustomImage(int id) const override;
  base::RefCountedMemory* GetRawData(
      int id,
      ui::ResourceScaleFactor scale_factor) const override;

 private:
  // Returns a cross platform image for an id.
  gfx::Image GetImageNamed(int id) const;

  bool incognito_;
};

#endif  // RADIUM_BROWSER_THEMES_RADIUM_THEME_PROVIDER_H_
