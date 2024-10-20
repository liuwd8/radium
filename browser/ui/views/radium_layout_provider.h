// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_LAYOUT_PROVIDER_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_LAYOUT_PROVIDER_H_

#include "ui/views/layout/layout_provider.h"

class RadiumLayoutProvider : public views::LayoutProvider {
 public:
  static RadiumLayoutProvider* Get();
  static std::unique_ptr<views::LayoutProvider> CreateLayoutProvider();

  explicit RadiumLayoutProvider();
  RadiumLayoutProvider(const RadiumLayoutProvider&) = delete;
  RadiumLayoutProvider& operator=(const RadiumLayoutProvider&) = delete;

  ~RadiumLayoutProvider() override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_LAYOUT_PROVIDER_H_
