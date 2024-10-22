// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_SIGNIN_SIGNIN_FRAME_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_SIGNIN_SIGNIN_FRAME_VIEW_H_

#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "ui/views/widget/widget_delegate.h"

class SigninFrameView : public views::WidgetDelegateView {
  METADATA_HEADER(SigninFrameView, views::WidgetDelegateView)

 public:
  explicit SigninFrameView();
  SigninFrameView(const SigninFrameView&) = delete;
  SigninFrameView& operator=(const SigninFrameView&) = delete;

  ~SigninFrameView() override;

 private:
  // views::WidgetDelegateView:
  gfx::Size GetMinimumSize() const override;

  ScopedKeepAlive keep_alive_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_SIGNIN_SIGNIN_FRAME_VIEW_H_
