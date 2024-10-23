// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/signin/signin_frame_view.h"

#include "components/keep_alive_registry/keep_alive_types.h"
#include "include/core/SkColor.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/signin/signin_window.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/background.h"
#include "ui/views/widget/widget.h"

void SigninWindow::Show() {
  auto* widget = new UntitledWidget();
  views::Widget::InitParams params = widget->GetUntitledWidgetParams();
  params.delegate = new SigninFrameView();
#if BUILDFLAG(IS_LINUX)
  params.wm_class_name = "radium";
  params.wm_class_class = "radium";
  params.wayland_app_id = params.wm_class_class;
#endif
  widget->SetTitleBarBackgroundColor(kColorFrameTitleBarBackground);
  widget->SetTitleBarBackgroundHeight(100);
  widget->Init(std::move(params));
  widget->Show();
}

SigninFrameView::SigninFrameView()
    : keep_alive_(KeepAliveOrigin::USER_MANAGER_VIEW,
                  KeepAliveRestartOption::DISABLED) {
  SetHasWindowSizeControls(true);
}

SigninFrameView::~SigninFrameView() = default;

gfx::Size SigninFrameView::GetMinimumSize() const {
  return {300, 300};
}

BEGIN_METADATA(SigninFrameView)
END_METADATA
