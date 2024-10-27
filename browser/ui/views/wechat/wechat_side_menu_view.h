// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_WECHAT_WECHAT_SIDE_MENU_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_WECHAT_WECHAT_SIDE_MENU_VIEW_H_

#include "ui/views/layout/box_layout_view.h"

class WechatSideMenuView : public views::BoxLayoutView {
  METADATA_HEADER(WechatSideMenuView, views::BoxLayoutView)

 public:
  explicit WechatSideMenuView();
  WechatSideMenuView(const WechatSideMenuView&) = delete;
  WechatSideMenuView& operator=(const WechatSideMenuView&) = delete;

  ~WechatSideMenuView() override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_WECHAT_WECHAT_SIDE_MENU_VIEW_H_
