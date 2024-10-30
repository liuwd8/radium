// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "radium/browser/ui/views/frame/opaque_frame_view.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"

#if BUILDFLAG(IS_WIN)
#include "radium/browser/ui/views/frame/untitled_widget_frame_view_win.h"
#endif

#if BUILDFLAG(IS_LINUX)
#include "radium/browser/ui/views/frame/untitled_widget_frame_view_linux.h"
#endif

namespace {

std::unique_ptr<OpaqueFrameView> CreateOpaqueFrameView(
    UntitledWidget* untitled_widget) {
#if BUILDFLAG(IS_LINUX)
  return std::make_unique<UntitledWidgetFrameViewLinux>(untitled_widget);
#else
  return std::make_unique<OpaqueFrameView>(untitled_widget);
#endif
}

}  // namespace

std::unique_ptr<UntitledWidgetNonClientFrameView>
UntitledWidgetNonClientFrameView::Create(UntitledWidget* untitled_widget) {
#if BUILDFLAG(IS_WIN)
  if (untitled_widget->ShouldUseNativeFrame()) {
    return std::make_unique<UntitledWidgetFrameViewWin>(untitled_widget);
  }
#endif
  auto view = CreateOpaqueFrameView(untitled_widget);
  // view->InitViews();
  return view;
}
