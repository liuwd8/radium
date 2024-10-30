// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_COLOR_NATIVE_RADIUM_COLOR_MIXER_H_
#define RADIUM_BROWSER_UI_COLOR_NATIVE_RADIUM_COLOR_MIXER_H_

#include "ui/color/color_provider_key.h"

namespace ui {
class ColorProvider;
}

// Adds a color mixer to |provider| that can override the default chrome colors.
// This function should be implemented on a per-platform basis in relevant
// subdirectories.
void AddNativeRadiumColorMixer(ui::ColorProvider* provider,
                               const ui::ColorProviderKey& key);

#endif  // RADIUM_BROWSER_UI_COLOR_NATIVE_RADIUM_COLOR_MIXER_H_
