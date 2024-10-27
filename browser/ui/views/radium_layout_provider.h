// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_LAYOUT_PROVIDER_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_LAYOUT_PROVIDER_H_

#include "ui/views/layout/layout_provider.h"

enum RadiumInsetsMetric {
  // Padding around buttons on the bookmarks bar.
  INSETS_BOOKMARKS_BAR_BUTTON = views::VIEWS_INSETS_END,
  // Margins used by toasts.
  INSETS_TOAST,
  // Padding used in an omnibox pill button.
  INSETS_OMNIBOX_PILL_BUTTON,
  // Padding used in an page info hover button.
  INSETS_PAGE_INFO_HOVER_BUTTON,
};

enum RadiumDistanceMetric {
  // 默认
  DISTANCE_UNTITLED_WIDGET_TITLE_BAR_HEIGHT = views::VIEWS_DISTANCE_END,
  // Vertical spacing between a list of multiple controls in one column.
  DISTANCE_CONTROL_LIST_VERTICAL,
  // Horizontal spacing between icon and label in the rich hover button.
  DISTANCE_RICH_HOVER_BUTTON_ICON_HORIZONTAL,
};

class RadiumLayoutProvider : public views::LayoutProvider {
 public:
  static RadiumLayoutProvider* Get();
  static std::unique_ptr<views::LayoutProvider> CreateLayoutProvider();

  explicit RadiumLayoutProvider();
  RadiumLayoutProvider(const RadiumLayoutProvider&) = delete;
  RadiumLayoutProvider& operator=(const RadiumLayoutProvider&) = delete;

  ~RadiumLayoutProvider() override;

  gfx::Insets GetInsetsMetric(int metric) const override;
  int GetDistanceMetric(int metric) const override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_LAYOUT_PROVIDER_H_
