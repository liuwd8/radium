// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_SIGNIN_SIGNIN_FRAME_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_SIGNIN_SIGNIN_FRAME_VIEW_H_

#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"
#include "ui/views/view_targeter_delegate.h"

class Profile;

namespace views {
class ImageView;
}

class SigninFrameView : public UntitledWidgetDelegateView,
                        public views::ViewTargeterDelegate {
  METADATA_HEADER(SigninFrameView, UntitledWidgetDelegateView)

 public:
  explicit SigninFrameView();
  SigninFrameView(const SigninFrameView&) = delete;
  SigninFrameView& operator=(const SigninFrameView&) = delete;

  ~SigninFrameView() override;

 private:
  friend class SigninWindow;

  // UntitledWidgetDelegateView:
  int NonClientHitTest(const gfx::Point& point) override;

  // Updates and formats QR code, text, and controls.
  void UpdateQRContent();
  void UpdateQRImage(gfx::ImageSkia qr_image);

  ScopedKeepAlive keep_alive_;

  raw_ptr<views::View> title_bar_;
  raw_ptr<views::ImageView> qr_code_image_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_SIGNIN_SIGNIN_FRAME_VIEW_H_
