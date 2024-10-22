// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/desktop_untitled_frame_aura_linux.h"

#include "radium/browser/ui/views/frame/native_untitled_frame_factory.h"
#include "ui/ozone/public/ozone_platform.h"

DesktopUntitledFrameAuraLinux::DesktopUntitledFrameAuraLinux(
    UntitledWidget* untitled_widget)
    : DesktopUntitledFrameAura(untitled_widget) {}

DesktopUntitledFrameAuraLinux::~DesktopUntitledFrameAuraLinux() = default;

views::Widget::InitParams DesktopUntitledFrameAuraLinux::GetWidgetParams() {
  views::Widget::InitParams params =
      DesktopUntitledFrameAura::GetWidgetParams();
  params.remove_standard_frame = true;
  params.opacity = views::Widget::InitParams::WindowOpacity::kTranslucent;

  return params;
}

NativeUntitledFrame* NativeUntitledFrameFactory::CreateNativeUntitledFrame(
    UntitledWidget* untitled_widget) {
  return new DesktopUntitledFrameAuraLinux(untitled_widget);
}
