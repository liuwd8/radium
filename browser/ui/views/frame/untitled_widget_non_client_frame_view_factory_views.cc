// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/opaque_frame_view.h"
#include "radium/browser/ui/views/frame/untitled_widget_frame_view_linux.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

namespace {

std::unique_ptr<OpaqueFrameView> CreateOpaqueFrameView(
    UntitledWidget* untitled_widget) {
#if BUILDFLAG(IS_LINUX)
  return std::make_unique<UntitledWidgetFrameViewLinux>(untitled_widget);
#else
  return std::make_unique<OpaqueFrameView>(frame, browser_view,
                                           new OpaqueFrameViewLayout());
#endif
}

}  // namespace

std::unique_ptr<UntitledWidgetNonClientFrameView>
UntitledWidgetNonClientFrameView::Create(UntitledWidget* untitled_widget) {
#if BUILDFLAG(IS_WIN)
  if (frame->ShouldUseNativeFrame()) {
    return std::make_unique<BrowserFrameViewWin>(frame, browser_view);
  }
#endif
  auto view = CreateOpaqueFrameView(untitled_widget);
  // view->InitViews();
  return view;
}
