// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_ID_H_
#define RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_ID_H_

#include "components/color/color_id.h"
#include "ui/color/color_id.h"

// clang-format off
#define COMMON_RADIUM_COLOR_IDS \
  /* App menu colors. */ \
  /* The kColorAppMenuHighlightSeverityLow color id is used in */ \
  /* color_provider_css_colors_test.ts. If changing the variable name, the */ \
  /* variable name in the test needs to be changed as well. */ \
  E_CPONLY(kColorFrameTitleBarBackground, kRadiumColorsStart, \
           kRadiumColorsStart) \
  E_CPONLY(kWechatSideMenuBackground) \
    /* Hover Button colors */ \
  E_CPONLY(kColorHoverButtonBackgroundHovered) \

#define RADIUM_COLOR_IDS COMMON_RADIUM_COLOR_IDS

#include "ui/color/color_id_macros.inc"

enum RadiumColorIds : ui::ColorId {
  kRadiumColorsStart = color::kComponentsColorsEnd,

  RADIUM_COLOR_IDS

  kChromeColorsEnd,
};

// Note that this second include is not redundant. The second inclusion of the
// .inc file serves to undefine the macros the first inclusion defined.
#include "ui/color/color_id_macros.inc"

// clang-format on

#endif  // RADIUM_BROWSER_UI_COLOR_RADIUM_COLOR_ID_H_
