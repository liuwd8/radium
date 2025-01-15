// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget.h"

#include <memory>

#include "radium/browser/themes/theme_service.h"
#include "radium/browser/ui/views/frame/native_untitled_frame.h"
#include "radium/browser/ui/views/frame/native_untitled_frame_factory.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "radium/browser/ui/views/frame/untitled_window_client_view.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/widget/widget_delegate.h"

namespace {
ui::mojom::WindowShowState GetShowState(views::Widget* widget) {
  if (widget->IsMaximized()) {
    return ui::mojom::WindowShowState::kMaximized;
  } else if (widget->IsMinimized()) {
    return ui::mojom::WindowShowState::kMinimized;
  } else if (widget->IsFullscreen()) {
    return ui::mojom::WindowShowState::kFullscreen;
  }

  return ui::mojom::WindowShowState::kNormal;
}
}  // namespace

UntitledWidget::UntitledWidget(UntitledWidgetDelegate* delegate,
                               Profile* profile)
    : delegate_(delegate),
      profile_(profile),
      title_bar_background_height_(
          RadiumLayoutProvider::Get()->GetDistanceMetric(
              DISTANCE_UNTITLED_WIDGET_TITLE_BAR_HEIGHT)) {
  set_is_secondary_widget(false);
  // Don't focus anything on creation, selecting a tab will set the focus.
  set_focus_on_creation(false);
}

UntitledWidget::~UntitledWidget() = default;

gfx::Point UntitledWidget::GetThemeOffsetFromBrowserView() const {
  gfx::Point browser_view_origin;
  views::View::ConvertPointToTarget(client_view(), GetRootView(),
                                    &browser_view_origin);
  return gfx::Point(-browser_view_origin.x(), 16 - browser_view_origin.y());
}

gfx::Insets UntitledWidget::GetCaptionButtonInsets() const {
  return untitled_widget_non_client_frame_view_
             ? untitled_widget_non_client_frame_view_->GetCaptionButtonInsets()
             : gfx::Insets();
}

views::Widget::InitParams UntitledWidget::GetUntitledWidgetParams() {
  native_untitled_frame_ =
      NativeUntitledFrameFactory::CreateNativeUntitledFrame(this);
  views::Widget::InitParams params = native_untitled_frame_->GetWidgetParams();
  params.name = "UntitledWidget";
  params.headless_mode = false;
  return params;
}

std::unique_ptr<views::NonClientFrameView>
UntitledWidget::CreateNonClientFrameView() {
  std::unique_ptr<UntitledWidgetNonClientFrameView> non_client =
      UntitledWidgetNonClientFrameView::Create(this);
  untitled_widget_non_client_frame_view_ = non_client.get();
  return non_client;
}

const ui::ThemeProvider* UntitledWidget::GetThemeProvider() const {
  return &ThemeService::GetThemeProviderForProfile(profile_);
}

ui::ColorProviderKey::ThemeInitializerSupplier* UntitledWidget::GetCustomTheme()
    const {
  // return ThemeService::GetThemeSupplierForProfile(nullptr);
  return nullptr;
}

#if BUILDFLAG(IS_WIN)
void UntitledWidget::OnNativeWidgetSizeChanged(const gfx::Size& new_size) {
  views::Widget::OnNativeWidgetSizeChanged(new_size);

  ui::mojom::WindowShowState show_state = GetShowState(this);
  if (last_show_state_ != show_state) {
    OnNativeWidgetWindowShowStateChanged();
    last_show_state_ = show_state;
  }
}
#endif

BEGIN_METADATA(UntitledWidget)
ADD_PROPERTY_METADATA(int, TitleBarBackgroundHeight)
ADD_READONLY_PROPERTY_METADATA(std::optional<ui::ColorId>,
                               TitleBarBackgroundColor)
END_METADATA
