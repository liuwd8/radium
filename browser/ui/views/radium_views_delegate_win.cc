// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_views_delegate.h"

#include "radium/browser/ui/views/native_widget_factory.h"

views::NativeWidget* RadiumViewsDelegate::CreateNativeWidget(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // Check the force_software_compositing flag only on Windows. If this flag is
  // on, it means that the widget being created wants to use the software
  // compositor which requires a top level window. We cannot have a mixture of
  // compositors active in one view hierarchy.
  NativeWidgetType native_widget_type =
      (params->parent && params->child && !params->force_software_compositing &&
       params->type != views::Widget::InitParams::TYPE_TOOLTIP)
          ? NativeWidgetType::NATIVE_WIDGET_AURA
          : NativeWidgetType::DESKTOP_NATIVE_WIDGET_AURA;

  if (params->shadow_type == views::Widget::InitParams::ShadowType::kDrop &&
      params->shadow_elevation.has_value()) {
    // If the window defines an elevation based shadow in the Widget
    // initialization parameters, force the use of a non toplevel window,
    // as the native window manager has no concept of elevation based shadows.
    // TODO: This may no longer be needed if we get proper elevation-based
    // shadows on toplevel windows. See https://crbug.com/838667.
    native_widget_type = NativeWidgetType::NATIVE_WIDGET_AURA;
  } else {
    // Otherwise, we can use a toplevel window (they get blended via
    // WS_EX_COMPOSITED, which allows for animation effects, and for exceeding
    // the bounds of the parent window).
    if (params->parent &&
        params->type != views::Widget::InitParams::TYPE_CONTROL &&
        params->type != views::Widget::InitParams::TYPE_WINDOW) {
      native_widget_type = NativeWidgetType::DESKTOP_NATIVE_WIDGET_AURA;
    }
  }
  return ::CreateNativeWidget(native_widget_type, params, delegate);
}
