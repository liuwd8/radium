// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/wechat/search_textfield.h"

#include "radium/browser/ui/views/radium_layout_provider.h"
#include "ui/views/controls/textfield/textfield.h"

SearchTextfield::SearchTextfield() {
  SetExtraInsets(
      gfx::Insets::VH(-RadiumLayoutProvider::Get()->GetDistanceMetric(
                          views::DISTANCE_CONTROL_VERTICAL_TEXT_PADDING),
                      4));
}

SearchTextfield::~SearchTextfield() = default;

gfx::Size SearchTextfield::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  gfx::Size size = views::Textfield::CalculatePreferredSize(available_size);
  gfx::Insets extra_insets =
      gfx::Insets::VH(-RadiumLayoutProvider::Get()->GetDistanceMetric(
                          views::DISTANCE_CONTROL_VERTICAL_TEXT_PADDING),
                      4);
  size.Enlarge(extra_insets.width(), extra_insets.height());

  return size;
}
