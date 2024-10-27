// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_layout_provider.h"

#include "base/notreached.h"

namespace {

RadiumLayoutProvider* g_radium_layout_provider = nullptr;

}  // namespace

// static
RadiumLayoutProvider* RadiumLayoutProvider::Get() {
  // Check to avoid downcasting a base LayoutProvider.
  DCHECK_EQ(g_radium_layout_provider, views::LayoutProvider::Get());
  return static_cast<RadiumLayoutProvider*>(views::LayoutProvider::Get());
}

// static
std::unique_ptr<views::LayoutProvider>
RadiumLayoutProvider::CreateLayoutProvider() {
  return std::make_unique<RadiumLayoutProvider>();
}

RadiumLayoutProvider::RadiumLayoutProvider() {
  DCHECK_EQ(nullptr, g_radium_layout_provider);
  g_radium_layout_provider = this;
}

RadiumLayoutProvider::~RadiumLayoutProvider() {
  DCHECK_EQ(this, g_radium_layout_provider);
  g_radium_layout_provider = nullptr;
}

gfx::Insets RadiumLayoutProvider::GetInsetsMetric(int metric) const {
  DCHECK_LT(metric, views::VIEWS_INSETS_MAX);
  switch (metric) {
    case views::INSETS_DIALOG:
    case views::INSETS_DIALOG_SUBSECTION: {
      return gfx::Insets::VH(20, 20);
    }
    case views::INSETS_DIALOG_FOOTNOTE: {
      return gfx::Insets::TLBR(10, 20, 15, 20);
    }
    case views::INSETS_CHECKBOX_RADIO_BUTTON: {
      gfx::Insets insets = LayoutProvider::GetInsetsMetric(metric);
      // Checkboxes and radio buttons should be aligned flush to the left edge.
      return gfx::Insets::TLBR(insets.top(), 0, insets.bottom(),
                               insets.right());
    }
    case views::INSETS_VECTOR_IMAGE_BUTTON:
      return gfx::Insets(4);
    case views::InsetsMetric::INSETS_LABEL_BUTTON:
      return LayoutProvider::GetInsetsMetric(metric);
    case INSETS_BOOKMARKS_BAR_BUTTON:
      return gfx::Insets(6);
    case INSETS_TOAST:
      return gfx::Insets::VH(0, 16);
    case INSETS_OMNIBOX_PILL_BUTTON:
      return gfx::Insets::VH(4, 8);
    case INSETS_PAGE_INFO_HOVER_BUTTON: {
      const gfx::Insets insets =
          LayoutProvider::GetInsetsMetric(views::INSETS_LABEL_BUTTON);
      const int horizontal_padding = 20;
      // Hover button in page info requires double the height compared to the
      // label button because it behaves like a menu control.
      return gfx::Insets::VH(insets.height(), horizontal_padding);
    }
    default:
      return LayoutProvider::GetInsetsMetric(metric);
  }
}

int RadiumLayoutProvider::GetDistanceMetric(int metric) const {
  DCHECK_GE(metric, views::VIEWS_DISTANCE_START);
  DCHECK_LT(metric, views::VIEWS_DISTANCE_MAX);

  if (metric < views::VIEWS_DISTANCE_END) {
    return LayoutProvider::GetDistanceMetric(metric);
  }

  switch (static_cast<RadiumDistanceMetric>(metric)) {
    case DISTANCE_UNTITLED_WIDGET_TITLE_BAR_HEIGHT:
      return 41;
    case DISTANCE_CONTROL_LIST_VERTICAL:
      return 12;
    case DISTANCE_RICH_HOVER_BUTTON_ICON_HORIZONTAL:
      return 8;
  }
  NOTREACHED();
}
