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
  /* Window control button background colors. */ \
  E_CPONLY(kColorWindowControlButtonBackgroundActive) \
  E_CPONLY(kColorWindowControlButtonBackgroundInactive) \

#if BUILDFLAG(IS_WIN)
#define RADIUM_PLATFORM_SPECIFIC_COLOR_IDS \
    /* The colors of the 1px border around the window on Windows 10. */ \
    E_CPONLY(kColorAccentBorderActive) \
    E_CPONLY(kColorAccentBorderInactive) \
    /* Caption colors. */ \
    E_CPONLY(kColorCaptionButtonForegroundActive) \
    E_CPONLY(kColorCaptionButtonForegroundInactive) \
    E_CPONLY(kColorCaptionCloseButtonBackgroundHovered) \
    E_CPONLY(kColorCaptionCloseButtonForegroundHovered) \
    E_CPONLY(kColorCaptionForegroundActive) \
    E_CPONLY(kColorCaptionForegroundInactive) \
    /* Tab search caption button colors. */ \
    E_CPONLY(kColorTabSearchCaptionButtonFocusRing)
#else
#define RADIUM_PLATFORM_SPECIFIC_COLOR_IDS
#endif  // BUILDFLAG(IS_WIN)

#define RADIUM_COLOR_IDS \
    COMMON_RADIUM_COLOR_IDS RADIUM_PLATFORM_SPECIFIC_COLOR_IDS

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
