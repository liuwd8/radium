// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_GALLERY_GALLERY_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_GALLERY_GALLERY_VIEW_H_

#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"
#include "ui/views/view_targeter_delegate.h"

class Profile;

namespace views {
class WebView;
}

class GalleryView : public UntitledWidgetDelegateView,
                    public views::ViewTargeterDelegate {
 public:
  explicit GalleryView();
  GalleryView(const GalleryView&) = delete;
  GalleryView& operator=(const GalleryView&) = delete;

  ~GalleryView() override;

  void Show(Profile* profile);

 private:
  // UntitledWidgetDelegateView:
  int NonClientHitTest(const gfx::Point& point) override;
  gfx::Size GetMinimumSize() const override;
  void AddedToWidget() override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;

 private:
  void OnButtonPressed(int hit_component);

  ScopedKeepAlive keep_alive_;

  raw_ptr<views::View> title_bar_;
  raw_ptr<views::WebView> webview_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_GALLERY_GALLERY_VIEW_H_