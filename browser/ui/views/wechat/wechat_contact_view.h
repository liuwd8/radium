// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_WECHAT_WECHAT_CONTACT_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_WECHAT_WECHAT_CONTACT_VIEW_H_

#include "base/functional/callback_forward.h"
#include "ui/views/layout/box_layout_view.h"

class WechatContactView : public views::BoxLayoutView {
 public:
  explicit WechatContactView(base::RepeatingClosure callback);
  WechatContactView(const WechatContactView&) = delete;
  WechatContactView& operator=(const WechatContactView&) = delete;

  ~WechatContactView() override;

 private:
  void OnClick();

  base::RepeatingClosure callback_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_WECHAT_WECHAT_CONTACT_VIEW_H_
