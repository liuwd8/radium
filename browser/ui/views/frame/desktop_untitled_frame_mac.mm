// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/desktop_untitled_frame_mac.h"

#import "base/apple/foundation_util.h"
#include "components/remote_cocoa/common/native_widget_ns_window.mojom.h"
#include "radium/browser/ui/views/frame/native_untitled_frame_factory.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#import "ui/views/cocoa/native_widget_mac_ns_window_host.h"
#include "ui/views/widget/native_widget_mac.h"

DesktopUntitledFrameMac::DesktopUntitledFrameMac(
    UntitledWidget* untitled_widget)
    : views::NativeWidgetMac(untitled_widget),
      untitled_widget_(untitled_widget) {}

DesktopUntitledFrameMac::~DesktopUntitledFrameMac() = default;

views::Widget::InitParams DesktopUntitledFrameMac::GetWidgetParams() {
  views::Widget::InitParams params(
      views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET);
  params.native_widget = this;
  return params;
}

bool DesktopUntitledFrameMac::UsesNativeSystemMenu() const {
  return false;
}

bool DesktopUntitledFrameMac::ShouldSaveWindowPlacement() const {
  return true;
}

void DesktopUntitledFrameMac::GetWindowPlacement(
    gfx::Rect* bounds,
    ui::mojom::WindowShowState* show_state) const {
  return NativeWidgetMac::GetWindowPlacement(bounds, show_state);
}

void DesktopUntitledFrameMac::PopulateCreateWindowParams(
    const views::Widget::InitParams& widget_params,
    remote_cocoa::mojom::CreateWindowParams* params) {
  params->style_mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                       NSWindowStyleMaskMiniaturizable |
                       NSWindowStyleMaskResizable;

  params->window_class = remote_cocoa::mojom::WindowClass::kBrowser;
  params->style_mask |= NSWindowStyleMaskFullSizeContentView;
  params->window_title_hidden = true;

  // Ensure tabstrip/profile button are visible.
  params->titlebar_appears_transparent = true;

  params->animation_enabled = true;
}

void DesktopUntitledFrameMac::GetWindowFrameTitlebarHeight(
    bool* override_titlebar_height,
    float* titlebar_height) {
  if (untitled_widget_ && untitled_widget_->non_client_view()->frame_view()) {
    *override_titlebar_height = true;
    *titlebar_height = untitled_widget_->GetTitleBarBackgroundHeight();
  } else {
    *override_titlebar_height = false;
    *titlebar_height = 0;
  }
}

NativeUntitledFrame* NativeUntitledFrameFactory::CreateNativeUntitledFrame(
    UntitledWidget* untitled_widget) {
  return new DesktopUntitledFrameMac(untitled_widget);
}
