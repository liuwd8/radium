// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

#include "build/build_config.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_frame_view_mac.h"

std::unique_ptr<UntitledWidgetNonClientFrameView>
UntitledWidgetNonClientFrameView::Create(UntitledWidget* untitled_widget) {
  return std::make_unique<UntitledWidgetFrameViewMac>(untitled_widget);
}
