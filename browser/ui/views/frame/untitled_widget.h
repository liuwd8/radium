// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_H_

#include "ui/views/widget/widget.h"

class NativeUntitledFrame;

class UntitledWidget : public views::Widget {
 public:
  explicit UntitledWidget();
  UntitledWidget(const UntitledWidget&) = delete;
  UntitledWidget& operator=(const UntitledWidget&) = delete;

  ~UntitledWidget() override;

#if BUILDFLAG(IS_LINUX)
  // Returns whether the frame is in a tiled state.
  bool tiled() const { return tiled_; }
  void set_tiled(bool tiled) { tiled_ = tiled; }
#endif

  views::Widget::InitParams GetUntitledWidgetParams();

 private:
  raw_ptr<NativeUntitledFrame> native_untitled_frame_;

#if BUILDFLAG(IS_LINUX)
  bool tiled_ = false;
#endif
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_H_
