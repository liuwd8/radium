// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_views_delegate.h"

#include "base/environment.h"
#include "base/nix/xdg_util.h"
#include "base/version_info/channel.h"
#include "radium/browser/ui/views/native_widget_factory.h"
#include "radium/common/channel_info.h"
#include "ui/base/ui_base_features.h"
#include "ui/ozone/public/ozone_platform.h"

namespace {

bool IsDesktopEnvironmentUnity() {
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  base::nix::DesktopEnvironment desktop_env =
      base::nix::GetDesktopEnvironment(env.get());
  return desktop_env == base::nix::DESKTOP_ENVIRONMENT_UNITY;
}

NativeWidgetType GetNativeWidgetTypeForInitParams(
    const views::Widget::InitParams& params) {
  // If this is a security surface, always use a toplevel window,
  // otherwise it's possible for things like menus to obscure the view.
  if (params.z_order &&
      params.z_order.value() == ui::ZOrderLevel::kSecuritySurface) {
    return NativeWidgetType::DESKTOP_NATIVE_WIDGET_AURA;
  }

  const bool default_desktop_bubble =
      (params.type == views::Widget::InitParams::TYPE_BUBBLE ||
       params.type == views::Widget::InitParams::TYPE_POPUP) &&
      base::FeatureList::IsEnabled(features::kOzoneBubblesUsePlatformWidgets) &&
      ui::OzonePlatform::GetInstance()
          ->GetPlatformRuntimeProperties()
          .supports_subwindows_as_accelerated_widgets;

  if (!params.child &&
      params.use_accelerated_widget_override.value_or(default_desktop_bubble)) {
    return NativeWidgetType::DESKTOP_NATIVE_WIDGET_AURA;
  }

  return (params.parent &&
          params.type != views::Widget::InitParams::TYPE_MENU &&
          params.type != views::Widget::InitParams::TYPE_TOOLTIP)
             ? NativeWidgetType::NATIVE_WIDGET_AURA
             : NativeWidgetType::DESKTOP_NATIVE_WIDGET_AURA;
}

}  // namespace

views::NativeWidget* RadiumViewsDelegate::CreateNativeWidget(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  return ::CreateNativeWidget(GetNativeWidgetTypeForInitParams(*params), params,
                              delegate);
}

bool RadiumViewsDelegate::WindowManagerProvidesTitleBar(bool maximized) {
  // On Ubuntu Unity, the system always provides a title bar for
  // maximized windows.
  //
  // TODO(thomasanderson,crbug.com/784010): Consider using the
  // _UNITY_SHELL wm hint when support for Ubuntu Trusty is dropped.
  if (!maximized) {
    return false;
  }
  static bool is_desktop_environment_unity = IsDesktopEnvironmentUnity();
  return is_desktop_environment_unity;
}
