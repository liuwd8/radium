// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/desktop_untitled_frame_aura.h"

#include "build/build_config.h"
#include "radium/browser/ui/views/frame/native_untitled_frame_factory.h"
#include "radium/browser/ui/views/frame/untitled_desktop_window_tree_host.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "ui/aura/client/visibility_client.h"
#include "ui/wm/core/visibility_controller.h"

DesktopUntitledFrameAura::DesktopUntitledFrameAura(
    UntitledWidget* untitled_widget)
    : views::DesktopNativeWidgetAura(untitled_widget),
      untitled_widget_(untitled_widget) {
  GetNativeWindow()->SetName("UntitledFrameAura");
}

DesktopUntitledFrameAura::~DesktopUntitledFrameAura() = default;

void DesktopUntitledFrameAura::OnHostClosed() {
  aura::client::SetVisibilityClient(GetNativeView()->GetRootWindow(), nullptr);
  DesktopNativeWidgetAura::OnHostClosed();
}

void DesktopUntitledFrameAura::InitNativeWidget(
    views::Widget::InitParams params) {
  browser_desktop_window_tree_host_ =
      UntitledDesktopWindowTreeHost::CreateBrowserDesktopWindowTreeHost(
          untitled_widget_, this, untitled_widget_);
  params.desktop_window_tree_host =
      browser_desktop_window_tree_host_->AsDesktopWindowTreeHost();
  DesktopNativeWidgetAura::InitNativeWidget(std::move(params));

  visibility_controller_ = std::make_unique<wm::VisibilityController>();
  aura::client::SetVisibilityClient(GetNativeView()->GetRootWindow(),
                                    visibility_controller_.get());
  wm::SetChildWindowVisibilityChangesAnimated(GetNativeView()->GetRootWindow());
}

std::unique_ptr<views::NonClientFrameView>
DesktopUntitledFrameAura::CreateNonClientFrameView() {
  return UntitledWidgetNonClientFrameView::Create(untitled_widget_);
}

views::Widget::InitParams DesktopUntitledFrameAura::GetWidgetParams() {
  views::Widget::InitParams params(
      views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET);
  params.native_widget = this;
  return params;
}

bool DesktopUntitledFrameAura::UsesNativeSystemMenu() const {
  return browser_desktop_window_tree_host_->UsesNativeSystemMenu();
}

bool DesktopUntitledFrameAura::ShouldSaveWindowPlacement() const {
  // The placement can always be stored.
  return true;
}

void DesktopUntitledFrameAura::GetWindowPlacement(
    gfx::Rect* bounds,
    ui::mojom::WindowShowState* show_state) const {
  *bounds = GetWidget()->GetRestoredBounds();
  if (IsMaximized()) {
    *show_state = ui::mojom::WindowShowState::kMaximized;
  } else if (IsMinimized()) {
    *show_state = ui::mojom::WindowShowState::kMinimized;
  } else {
    *show_state = ui::mojom::WindowShowState::kNormal;
  }
}

#if !BUILDFLAG(IS_LINUX)
NativeUntitledFrame* NativeUntitledFrameFactory::CreateNativeUntitledFrame(
    UntitledWidget* untitled_widget) {
  return new DesktopUntitledFrameAura(untitled_widget);
}
#endif
