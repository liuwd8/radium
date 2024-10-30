// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_DELEGATE_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_DELEGATE_H_

#include <memory>

#include "ui/views/view.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
class Widget;
}

class UntitledWidgetDelegate : public views::WidgetDelegate {
 public:
  explicit UntitledWidgetDelegate();
  UntitledWidgetDelegate(const UntitledWidgetDelegate&) = delete;
  UntitledWidgetDelegate& operator=(const UntitledWidgetDelegate&) = delete;

  ~UntitledWidgetDelegate() override;

  virtual int NonClientHitTest(const gfx::Point& point);
};

class UntitledWidgetDelegateView : public UntitledWidgetDelegate,
                                   public views::View {
  METADATA_HEADER(UntitledWidgetDelegateView, views::View)

 public:
  UntitledWidgetDelegateView();
  UntitledWidgetDelegateView(const UntitledWidgetDelegateView&) = delete;
  UntitledWidgetDelegateView& operator=(const UntitledWidgetDelegateView&) =
      delete;
  ~UntitledWidgetDelegateView() override;

  // UntitledWidgetDelegate:
  views::Widget* GetWidget() override;
  const views::Widget* GetWidget() const override;
  View* GetContentsView() override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_DELEGATE_H_
