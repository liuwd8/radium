// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/gallery/gallery_view.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/notreached.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "radium/common/webui_url_constants.h"
#include "ui/base/hit_test.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/background.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/window/frame_caption_button.h"
#include "ui/views/window/hit_test_utils.h"
#include "ui/views/window/vector_icons/vector_icons.h"

namespace {

std::unique_ptr<views::FrameCaptionButton> CreateFrameCaptionButton(
    views::CaptionButtonIcon icon_type,
    int ht_component,
    const gfx::VectorIcon& icon_image) {
  auto button = std::make_unique<views::FrameCaptionButton>(
      views::Button::PressedCallback(), icon_type, ht_component);
  button->SetImage(button->GetIcon(), views::FrameCaptionButton::Animate::kNo,
                   icon_image);
  button->GetViewAccessibility().SetName(u"haha");
  button->SetPaintAsActive(true);
  return button;
}

}  // namespace

GalleryView::GalleryView()
    : keep_alive_(KeepAliveOrigin::USER_MANAGER_VIEW,
                  KeepAliveRestartOption::DISABLED) {
  SetHasWindowSizeControls(true);
  SetTitle(u"WebUI Gallery");
  SetBackground(views::CreateThemedRoundedRectBackground(
      kColorFrameTitleBarBackground,
      RadiumLayoutProvider::Get()->GetCornerRadiusMetric(
          views::Emphasis::kHigh),
      0, 0));
  views::BoxLayout* layout =
      SetLayoutManager(std::make_unique<views::BoxLayout>());
  layout->SetOrientation(views::BoxLayout::Orientation::kVertical);

  SetEventTargeter(std::make_unique<views::ViewTargeter>(this));
  views::Builder<views::View>(this)
      .AddChildren(
          views::Builder<views::BoxLayoutView>()
              .CopyAddressTo(&title_bar_)
              .SetInsideBorderInsets(gfx::Insets::TLBR(0, 12, 0, 8))
              .SetMainAxisAlignment(views::BoxLayout::MainAxisAlignment::kEnd)
              .SetPreferredSize(gfx::Size(0, 35))
              .CustomConfigure(base::BindOnce([](views::BoxLayoutView* view) {
                views::SetHitTestComponent(view, HTCAPTION);
              }))
              .AddChildren(
                  views::Builder<views::Button>(
                      CreateFrameCaptionButton(
                          views::CaptionButtonIcon::
                              CAPTION_BUTTON_ICON_MINIMIZE,
                          HTMINBUTTON, views::kWindowControlMinimizeIcon))
                      .SetPreferredSize(
                          gfx::Size(views::GetCaptionButtonWidth(), 0))
                      .SetCallback(base::BindRepeating(
                          &GalleryView::OnButtonPressed, base::Unretained(this),
                          HTMINBUTTON)),
                  views::Builder<views::Button>(
                      CreateFrameCaptionButton(
                          views::CaptionButtonIcon::
                              CAPTION_BUTTON_ICON_MAXIMIZE_RESTORE,
                          HTMAXBUTTON, views::kWindowControlMaximizeIcon))
                      .SetPreferredSize(
                          gfx::Size(views::GetCaptionButtonWidth(), 0))
                      .SetCallback(base::BindRepeating(
                          &GalleryView::OnButtonPressed, base::Unretained(this),
                          HTMAXBUTTON)),
                  views::Builder<views::Button>(
                      CreateFrameCaptionButton(
                          views::CaptionButtonIcon::CAPTION_BUTTON_ICON_CLOSE,
                          HTCLOSE, views::kWindowControlCloseIcon))
                      .SetPreferredSize(
                          gfx::Size(views::GetCaptionButtonWidth(), 0))
                      .SetCallback(base::BindRepeating(
                          &GalleryView::OnButtonPressed, base::Unretained(this),
                          HTCLOSE))),
          views::Builder<views::WebView>().CopyAddressTo(&webview_).SetProperty(
              views::kBoxLayoutFlexKey, views::BoxLayoutFlexSpecification()))
      .BuildChildren();
}

GalleryView::~GalleryView() = default;

int GalleryView::NonClientHitTest(const gfx::Point& point) {
  return views::GetHitTestComponent(this, point);
}

gfx::Size GalleryView::GetMinimumSize() const {
  return {900, 600};
}

void GalleryView::AddedToWidget() {
  webview_->LoadInitialURL(GURL(radium::kRadiumUIWebuiGalleryURL));
}

gfx::Size GalleryView::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  gfx::Size size = GetLayoutManager()->GetPreferredSize(this, available_size);
  size.SetToMax(GetMinimumSize());
  return size;
}

void GalleryView::Show(Profile* profile) {
  auto* widget = new UntitledWidget(this, profile);
  views::Widget::InitParams params = widget->GetUntitledWidgetParams();
  params.delegate = this;
  params.name = "GalleryView";
#if BUILDFLAG(IS_LINUX)
  params.wm_class_name = "GalleryView";
  params.wm_class_class = "GalleryView";
  params.wayland_app_id = params.wm_class_class;
#endif
  widget->SetTitleBarBackgroundColor(kColorFrameTitleBarBackground);
  webview_->SetBrowserContext(profile);
  widget->Init(std::move(params));

  widget->Show();
}

void GalleryView::OnButtonPressed(int hit_component) {
  switch (hit_component) {
    case HTMINBUTTON: {
      GetWidget()->Minimize();
    } break;
    case HTMAXBUTTON: {
      GetWidget()->Maximize();
    } break;
    case HTCLOSE: {
      GetWidget()->Close();
    } break;
    default:
      NOTREACHED();
  }
}
