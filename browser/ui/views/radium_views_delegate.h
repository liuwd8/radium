// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_VIEWS_DELEGATE_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_VIEWS_DELEGATE_H_

#include "ui/views/views_delegate.h"

class RadiumViewsDelegate : public views::ViewsDelegate {
 public:
  explicit RadiumViewsDelegate();
  RadiumViewsDelegate(const RadiumViewsDelegate&) = delete;
  RadiumViewsDelegate& operator=(const RadiumViewsDelegate&) = delete;

  ~RadiumViewsDelegate() override;

  // views::ViewsDelegate:
  void SaveWindowPlacement(const views::Widget* window,
                           const std::string& window_name,
                           const gfx::Rect& bounds,
                           ui::mojom::WindowShowState show_state) override;
  bool GetSavedWindowPlacement(
      const views::Widget* widget,
      const std::string& window_name,
      gfx::Rect* bounds,
      ui::mojom::WindowShowState* show_state) const override;
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
  bool WindowManagerProvidesTitleBar(bool maximized) override;
#endif
  void OnBeforeWidgetInit(
      views::Widget::InitParams* params,
      views::internal::NativeWidgetDelegate* delegate) override;

 private:
  views::NativeWidget* CreateNativeWidget(
      views::Widget::InitParams* params,
      views::internal::NativeWidgetDelegate* delegate);
};

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_VIEWS_DELEGATE_H_
