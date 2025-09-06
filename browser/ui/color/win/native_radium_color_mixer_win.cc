// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/color/native_radium_color_mixer.h"

#include <optional>

#include "base/callback_list.h"
#include "base/functional/bind.h"
#include "base/no_destructor.h"
#include "base/win/windows_version.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/grit/theme_resources.h"
#include "ui/color/color_id.h"
#include "ui/color/color_mixer.h"
#include "ui/color/color_provider.h"
#include "ui/color/color_provider_key.h"
#include "ui/color/color_provider_utils.h"
#include "ui/color/color_recipe.h"
#include "ui/color/color_transform.h"
#include "ui/color/win/accent_color_observer.h"
#include "ui/gfx/color_utils.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/views_features.h"

namespace {

// Updates the NativeTheme's user_color to reflect the system accent color.
// TODO(crbug.com/40280436): Explore moving logic into NativeThemeWin.
void UpdateUserColor() {
  const auto accent_color = ui::AccentColorObserver::Get()->accent_color();
  ui::NativeTheme::GetInstanceForNativeUi()->set_user_color(accent_color);
  ui::NativeTheme::GetInstanceForWeb()->set_user_color(accent_color);
}

void OnAccentColorUpdated() {
  UpdateUserColor();
  ui::NativeTheme::GetInstanceForNativeUi()->NotifyOnNativeThemeUpdated();
  ui::NativeTheme::GetInstanceForWeb()->NotifyOnNativeThemeUpdated();
}

void UpdateUserColorWhenAccentColorStateChanges() {
  UpdateUserColor();
  static base::NoDestructor<base::CallbackListSubscription> subscription(
      ui::AccentColorObserver::Get()->Subscribe(
          base::BindRepeating(&OnAccentColorUpdated)));
}

SkColor GetAccentBorderColor() {
  if (const std::optional<SkColor> accent_border_color =
          ui::AccentColorObserver::Get()->accent_border_color()) {
    return accent_border_color.value();
  }

  // Windows 10 pre-version 1809 native active borders default to white, while
  // in version 1809 and onwards they default to #262626 with 66% alpha.
  const bool pre_1809 = base::win::GetVersion() < base::win::Version::WIN10_RS5;
  return pre_1809 ? SK_ColorWHITE : SkColorSetARGB(0xa8, 0x26, 0x26, 0x26);
}

ui::ColorTransform GetCaptionForegroundColor(
    ui::ColorTransform input_transform) {
  const auto generator = [](ui::ColorTransform input_transform,
                            SkColor input_color, const ui::ColorMixer& mixer) {
    const SkColor background_color = input_transform.Run(input_color, mixer);
    const float windows_luma = 0.25f * SkColorGetR(background_color) +
                               0.625f * SkColorGetG(background_color) +
                               0.125f * SkColorGetB(background_color);
    const SkColor result_color =
        (windows_luma <= 128.0f) ? SK_ColorWHITE : SK_ColorBLACK;
    DVLOG(2) << "ColorTransform GetCaptionForegroundColor:"
             << " Background Color: " << ui::SkColorName(background_color)
             << " Result Color: " << ui::SkColorName(result_color);
    return result_color;
  };
  return base::BindRepeating(generator, std::move(input_transform));
}

}  // namespace

void AddNativeChromeColorMixer(ui::ColorProvider* provider,
                               const ui::ColorProviderKey& key) {
  UpdateUserColorWhenAccentColorStateChanges();

  ui::ColorMixer& mixer = provider->AddMixer();

  mixer[kColorAccentBorderActive] = {GetAccentBorderColor()};
  mixer[kColorAccentBorderInactive] = {SkColorSetARGB(0x80, 0x55, 0x55, 0x55)};
  mixer[kColorCaptionButtonForegroundActive] =
      GetCaptionForegroundColor(kColorWindowControlButtonBackgroundActive);
  mixer[kColorCaptionButtonForegroundInactive] =
      GetCaptionForegroundColor(kColorWindowControlButtonBackgroundInactive);
  mixer[kColorCaptionCloseButtonBackgroundHovered] = {
      SkColorSetRGB(0xE8, 0x11, 0x23)};
  mixer[kColorCaptionCloseButtonForegroundHovered] = {SK_ColorWHITE};
  mixer[kColorCaptionForegroundActive] =
      GetCaptionForegroundColor(ui::kColorFrameActive);
  mixer[kColorCaptionForegroundInactive] =
      SetAlpha(GetCaptionForegroundColor(ui::kColorFrameInactive), 0x66);
  mixer[kColorTabSearchCaptionButtonFocusRing] = ui::PickGoogleColor(
      ui::kColorFocusableBorderFocused, ui::kColorFrameActive,
      color_utils::kMinimumVisibleContrastRatio);
}
