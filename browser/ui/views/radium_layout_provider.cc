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

int RadiumLayoutProvider::GetDistanceMetric(int metric) const {
  DCHECK_GE(metric, views::VIEWS_DISTANCE_START);
  DCHECK_LT(metric, views::VIEWS_DISTANCE_MAX);

  if (metric < views::VIEWS_DISTANCE_END) {
    return LayoutProvider::GetDistanceMetric(metric);
  }

  switch (static_cast<RadiumDistanceMetric>(metric)) {
    case DISTANCE_UNTITLED_WIDGET_TITLE_BAR_HEIGHT:
      return 41;
  }
  NOTREACHED();
}