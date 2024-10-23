// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/color/radium_color_mixers.h"

#include "base/containers/fixed_flat_map.h"
#include "base/no_destructor.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/color/radium_color_mixer.h"
#include "ui/color/color_provider_utils.h"

namespace {

class RadiumColorProviderUtilsCallbacks
    : public ui::ColorProviderUtilsCallbacks {
 public:
  bool ColorIdName(ui::ColorId color_id, std::string_view* color_name) override;
};

#include "ui/color/color_id_map_macros.inc"

bool RadiumColorProviderUtilsCallbacks::ColorIdName(
    ui::ColorId color_id,
    std::string_view* color_name) {
  static constexpr const auto radium_color_id_map =
      base::MakeFixedFlatMap<ui::ColorId, const char*>({RADIUM_COLOR_IDS});
  auto i = radium_color_id_map.find(color_id);
  if (i != radium_color_id_map.cend()) {
    *color_name = i->second;
    return true;
  }
  return false;
}

// Note that this second include is not redundant. The second inclusion of the
// .inc file serves to undefine the macros the first inclusion defined.
#include "ui/color/color_id_map_macros.inc"

}  // namespace

void AddRadiumColorMixers(ui::ColorProvider* provider,
                          const ui::ColorProviderKey& key) {
  static base::NoDestructor<RadiumColorProviderUtilsCallbacks>
      radium_color_provider_utils_callbacks;
  ui::SetColorProviderUtilsCallbacks(
      radium_color_provider_utils_callbacks.get());

  AddRadiumColorMixer(provider, key);
}
