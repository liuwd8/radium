// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_views_delegate.h"

#include "components/keep_alive_registry/keep_alive_registry.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "components/keep_alive_registry/scoped_keep_alive.h"

RadiumViewsDelegate::RadiumViewsDelegate() = default;
RadiumViewsDelegate::~RadiumViewsDelegate() = default;

void RadiumViewsDelegate::SaveWindowPlacement(
    const views::Widget* window,
    const std::string& window_name,
    const gfx::Rect& bounds,
    ui::mojom::WindowShowState show_state) {}

bool RadiumViewsDelegate::GetSavedWindowPlacement(
    const views::Widget* widget,
    const std::string& window_name,
    gfx::Rect* bounds,
    ui::mojom::WindowShowState* show_state) const {
  return false;
}

void RadiumViewsDelegate::AddRef() {
  if (ref_count_ == 0u) {
    keep_alive_ = std::make_unique<ScopedKeepAlive>(
        KeepAliveOrigin::CHROME_VIEWS_DELEGATE,
        KeepAliveRestartOption::DISABLED);
  }

  ++ref_count_;
}

void RadiumViewsDelegate::ReleaseRef() {
  DCHECK_NE(0u, ref_count_);

  if (--ref_count_ == 0u) {
    keep_alive_.reset();
  }
}

bool RadiumViewsDelegate::IsShuttingDown() const {
  return KeepAliveRegistry::GetInstance()->IsShuttingDown();
}

void RadiumViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // We need to determine opacity if it's not already specified.
  if (params->opacity == views::Widget::InitParams::WindowOpacity::kInferred) {
    params->opacity = views::Widget::InitParams::WindowOpacity::kOpaque;
  }

  // If we already have a native_widget, we don't have to try to come
  // up with one.
  if (params->native_widget) {
    return;
  }

  if (!native_widget_factory().is_null()) {
    params->native_widget = native_widget_factory().Run(*params, delegate);
    if (params->native_widget) {
      return;
    }
  }

  params->native_widget = CreateNativeWidget(params, delegate);
}
