// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_MIXER_H_
#define RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_MIXER_H_

#include "third_party/skia/include/core/SkColor.h"
#include "ui/color/color_provider_key.h"

namespace ui {
class ColorProvider;
}

// Adds a color mixer to |provider| that supplies default values for various
// chrome/ colors before taking into account any custom themes.
void AddRadiumColorMixer(ui::ColorProvider* provider,
                         const ui::ColorProviderKey& key);

#endif  // RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_MIXER_H_
