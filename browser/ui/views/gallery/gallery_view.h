// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_GALLERY_GALLERY_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_GALLERY_GALLERY_VIEW_H_

#include "radium/browser/ui/browser_window.h"
#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"
#include "ui/views/view_targeter_delegate.h"
#include "ui/views/widget/widget_observer.h"

class Browser;

namespace views {
class WebView;
}

class GalleryView : public UntitledWidgetDelegateView,
                    public views::ViewTargeterDelegate,
                    public BrowserWindow,
                    public views::WidgetObserver {
 public:
  static BrowserWindow* Show(std::unique_ptr<Browser> browser);

  explicit GalleryView(std::unique_ptr<Browser> browser);
  GalleryView(const GalleryView&) = delete;
  GalleryView& operator=(const GalleryView&) = delete;

  ~GalleryView() override;

 private:
  // UntitledWidgetDelegateView:
  int NonClientHitTest(const gfx::Point& point) override;
  gfx::Size GetMinimumSize() const override;
  void AddedToWidget() override;
  void RemovedFromWidget() override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;

  // BrowserWindow:
  views::Widget* GetWidget() override;
  void Show() override;
  void Close() override;

  // WidgetObserver:
  void OnWidgetShowStateChanged(views::Widget* widget) override;

 private:
  void OnButtonPressed(int hit_component);

  std::unique_ptr<Browser> browser_;

  raw_ptr<views::View> title_bar_;
  raw_ptr<views::View> maximize_button_;
  raw_ptr<views::View> restore_button_;
  raw_ptr<views::WebView> webview_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_GALLERY_GALLERY_VIEW_H_
