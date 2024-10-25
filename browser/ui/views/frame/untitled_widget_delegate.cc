// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"

#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"

UntitledWidgetDelegate::UntitledWidgetDelegate() = default;
UntitledWidgetDelegate::~UntitledWidgetDelegate() = default;

int UntitledWidgetDelegate::NonClientHitTest(const gfx::Point& point) {
  return HTNOWHERE;
}

////////////////////////////////////////////////////////////////////////////////
// UntitledWidgetDelegateView:

UntitledWidgetDelegateView::UntitledWidgetDelegateView() = default;

UntitledWidgetDelegateView::~UntitledWidgetDelegateView() = default;

views::Widget* UntitledWidgetDelegateView::GetWidget() {
  return View::GetWidget();
}

const views::Widget* UntitledWidgetDelegateView::GetWidget() const {
  return View::GetWidget();
}

views::View* UntitledWidgetDelegateView::GetContentsView() {
  return this;
}

BEGIN_METADATA(UntitledWidgetDelegateView)
END_METADATA
