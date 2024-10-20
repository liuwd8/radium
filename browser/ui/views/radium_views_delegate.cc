// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_views_delegate.h"

RadiumViewsDelegate::RadiumViewsDelegate() = default;
RadiumViewsDelegate::~RadiumViewsDelegate() = default;

void RadiumViewsDelegate::SaveWindowPlacement(
    const views::Widget* window,
    const std::string& window_name,
    const gfx::Rect& bounds,
    ui::mojom::WindowShowState show_state) {}

bool RadiumViewsDelegate::GetSavedWindowPlacement(
    const views::Widget* widget,
    const std::string& window_name,
    gfx::Rect* bounds,
    ui::mojom::WindowShowState* show_state) const {
  return false;
}
