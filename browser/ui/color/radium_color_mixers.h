// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_MIXERS_H_
#define RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_MIXERS_H_

#include "ui/color/color_provider_key.h"

namespace ui {
class ColorProvider;
}

// Adds all radium/-side color mixers to `provider`.
void AddRadiumColorMixers(ui::ColorProvider* provider,
                          const ui::ColorProviderKey& key);

#endif  // RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_MIXERS_H_
