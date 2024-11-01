// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_views_delegate.h"

#include "content/public/browser/context_factory.h"

ui::ContextFactory* RadiumViewsDelegate::GetContextFactory() {
  return content::GetContextFactory();
}

views::NativeWidget* RadiumViewsDelegate::CreateNativeWidget(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // By returning null Widget creates the default NativeWidget implementation.
  return nullptr;
}
