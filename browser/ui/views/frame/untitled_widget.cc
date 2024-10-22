// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget.h"

#include "radium/browser/ui/views/frame/native_untitled_frame.h"
#include "radium/browser/ui/views/frame/native_untitled_frame_factory.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "radium/browser/ui/views/frame/untitled_window_client_view.h"
#include "ui/views/widget/widget_delegate.h"

UntitledWidget::UntitledWidget() {
  set_is_secondary_widget(false);
  // Don't focus anything on creation, selecting a tab will set the focus.
  set_focus_on_creation(false);
}

UntitledWidget::~UntitledWidget() = default;

views::Widget::InitParams UntitledWidget::GetUntitledWidgetParams() {
  native_untitled_frame_ =
      NativeUntitledFrameFactory::CreateNativeUntitledFrame(this);
  views::Widget::InitParams params = native_untitled_frame_->GetWidgetParams();
  params.name = "UntitledWidget";
  params.headless_mode = false;
  return params;
}
