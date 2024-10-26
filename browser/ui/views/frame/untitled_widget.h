// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_H_

#include <optional>

#include "ui/color/color_id.h"
#include "ui/views/widget/widget.h"

class NativeUntitledFrame;
class Profile;
class UntitledWidgetDelegate;

class UntitledWidget : public views::Widget {
  METADATA_HEADER(UntitledWidget, views::Widget)

 public:
  explicit UntitledWidget(UntitledWidgetDelegate* delegate, Profile* profile);
  UntitledWidget(const UntitledWidget&) = delete;
  UntitledWidget& operator=(const UntitledWidget&) = delete;

  ~UntitledWidget() override;

#if BUILDFLAG(IS_LINUX)
  // Returns whether the frame is in a tiled state.
  bool tiled() const { return tiled_; }
  void set_tiled(bool tiled) { tiled_ = tiled; }
#endif

  UntitledWidgetDelegate* delegate() const { return delegate_; }

  // Set the background color and height of the title bar. Note that you cannot
  // set the background color directly in the external view. Otherwise, it will
  // cause abnormal rounded corners.
  void SetTitleBarBackgroundHeight(int title_bar_background_height) {
    title_bar_background_height_ = title_bar_background_height;
  }
  int GetTitleBarBackgroundHeight() const {
    return title_bar_background_height_;
  }
  void SetTitleBarBackgroundColor(
      std::optional<ui::ColorId> title_bar_background_color) {
    title_bar_background_color_ = title_bar_background_color;
  }
  std::optional<ui::ColorId> GetTitleBarBackgroundColor() const {
    return title_bar_background_color_;
  }

  gfx::Point GetThemeOffsetFromBrowserView() const;

  views::Widget::InitParams GetUntitledWidgetParams();

 protected:
  // views::Widget:
  const ui::ThemeProvider* GetThemeProvider() const override;
  ui::ColorProviderKey::ThemeInitializerSupplier* GetCustomTheme()
      const override;
  void OnNativeThemeUpdated(ui::NativeTheme* observed_theme) override;

 private:
  raw_ptr<UntitledWidgetDelegate> delegate_;
  raw_ptr<Profile> profile_;
  raw_ptr<NativeUntitledFrame> native_untitled_frame_;

#if BUILDFLAG(IS_LINUX)
  bool tiled_ = false;
#endif
  std::optional<ui::ColorId> title_bar_background_color_;
  int title_bar_background_height_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WIDGET_H_
