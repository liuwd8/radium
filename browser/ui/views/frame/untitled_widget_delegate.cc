// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"

#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/window/client_view.h"

namespace {
class UntitledWidgetClientView : public views::ClientView {
  METADATA_HEADER(UntitledWidgetClientView, ClientView)

 public:
  UntitledWidgetClientView(views::Widget* widget, views::View* contents_view)
      : ClientView(widget, contents_view) {}

  UntitledWidgetClientView(const UntitledWidgetClientView&) = delete;
  UntitledWidgetClientView& operator=(const UntitledWidgetClientView&) = delete;

  ~UntitledWidgetClientView() override = default;

  views::CloseRequestResult OnWindowCloseRequested() override {
    bool can_close = GetWidget()->widget_delegate()->OnCloseRequested(
        views::Widget::ClosedReason::kUnspecified);
    return can_close ? views::CloseRequestResult::kCanClose
                     : views::CloseRequestResult::kCannotClose;
  }
};

BEGIN_METADATA(UntitledWidgetClientView)
END_METADATA

}  // namespace

UntitledWidgetDelegate::UntitledWidgetDelegate() = default;
UntitledWidgetDelegate::~UntitledWidgetDelegate() = default;

int UntitledWidgetDelegate::NonClientHitTest(const gfx::Point& point) {
  return HTNOWHERE;
}

views::ClientView* UntitledWidgetDelegate::CreateClientView(
    views::Widget* widget) {
  return new UntitledWidgetClientView(widget,
                                      TransferOwnershipOfContentsView());
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
