// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_WECHAT_SEARCH_TEXTFIELD_H_
#define RADIUM_BROWSER_UI_VIEWS_WECHAT_SEARCH_TEXTFIELD_H_

#include "ui/views/controls/textfield/textfield.h"

class SearchTextfield : public views::Textfield {
 public:
  explicit SearchTextfield();
  SearchTextfield(const SearchTextfield&) = delete;
  SearchTextfield& operator=(const SearchTextfield&) = delete;

  ~SearchTextfield() override;

 private:
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_WECHAT_SEARCH_TEXTFIELD_H_
