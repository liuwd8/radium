// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/signin/signin_frame_view.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/location.h"
#include "base/task/single_thread_task_runner.h"
#include "base/types/expected.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "components/qr_code_generator/bitmap_generator.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/signin/signin_window.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/separator.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/view_targeter.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/frame_caption_button.h"
#include "ui/views/window/hit_test_utils.h"
#include "ui/views/window/vector_icons/vector_icons.h"

namespace {

// Calculates the height of the QR Code with padding.
constexpr gfx::Size GetQRCodeImageSize() {
  constexpr int kQRImageSizePx = 150;
  return gfx::Size(kQRImageSizePx, kQRImageSizePx);
}

#if !BUILDFLAG(IS_MAC)
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
#endif
}  // namespace

void SigninWindow::Show(Profile* profile, base::OnceClosure finish_callback) {
  auto* delegate = new SigninFrameView();
  auto* widget = new UntitledWidget(delegate, profile);

  delegate->Init(widget, std::move(finish_callback));

  views::Widget::InitParams params = widget->GetUntitledWidgetParams();
  params.delegate = delegate;
#if BUILDFLAG(IS_LINUX)
  params.wm_class_name = "radium";
  params.wm_class_class = "radium";
  params.wayland_app_id = params.wm_class_class;
#endif
  widget->SetTitleBarBackgroundColor(kColorFrameTitleBarBackground);
  widget->Init(std::move(params));
  widget->Show();
}

SigninFrameView::SigninFrameView()
    : keep_alive_(KeepAliveOrigin::USER_MANAGER_VIEW,
                  KeepAliveRestartOption::DISABLED) {}

SigninFrameView::~SigninFrameView() = default;

void SigninFrameView::Init(views::Widget* widget,
                           base::OnceClosure finish_callback) {
  views::BoxLayout* layout_manager =
      SetLayoutManager(std::make_unique<views::BoxLayout>());
  layout_manager->SetOrientation(views::BoxLayout::Orientation::kVertical);
  SetEventTargeter(std::make_unique<views::ViewTargeter>(this));

  views::Builder<views::View>(this)
      .SetPreferredSize(gfx::Size(280, 380))
      .AddChildren(
          views::Builder<views::BoxLayoutView>()
              .SetInsideBorderInsets(gfx::Insets::TLBR(0, 12, 0, 8))
              .SetPreferredSize(
                  gfx::Size(0, RadiumLayoutProvider::Get()->GetDistanceMetric(
                                   DISTANCE_UNTITLED_WIDGET_TITLE_BAR_HEIGHT)))
              .CustomConfigure(base::BindOnce([](views::BoxLayoutView* view) {
                views::SetHitTestComponent(view, HTCAPTION);
              }))
              .AddChildren(
                  views::Builder<views::Label>()
                      .SetProperty(views::kBoxLayoutFlexKey,
                                   views::BoxLayoutFlexSpecification())
                      .SetText(u"微信（开发版）")
#if !BUILDFLAG(IS_MAC)
                      .SetHorizontalAlignment(
                          gfx::HorizontalAlignment::ALIGN_LEFT),
                  views::Builder<views::Button>(
                      CreateFrameCaptionButton(
                          views::CaptionButtonIcon::CAPTION_BUTTON_ICON_MENU,
                          HTSYSMENU, views::kWindowControlMaximizeIcon))
                      .SetPreferredSize(
                          gfx::Size(views::GetCaptionButtonWidth(), 0)),
                  views::Builder<views::Button>(
                      CreateFrameCaptionButton(
                          views::CaptionButtonIcon::CAPTION_BUTTON_ICON_CLOSE,
                          HTCLOSE, views::kWindowControlCloseIcon))
                      .SetPreferredSize(
                          gfx::Size(views::GetCaptionButtonWidth(), 0))
                      .SetCallback(base::BindOnce(&views::Widget::Close,
                                                  base::Unretained(widget)))),
#else
                      .SetHorizontalAlignment(
                          gfx::HorizontalAlignment::ALIGN_CENTER)),
#endif
          views::Builder<views::BoxLayoutView>()
              .SetOrientation(views::BoxLayout::Orientation::kVertical)
              .SetInsideBorderInsets(gfx::Insets::VH(30, 12))
              .SetBetweenChildSpacing(20)
              .SetCollapseMarginsSpacing(true)
              .SetCrossAxisAlignment(
                  views::BoxLayout::CrossAxisAlignment::kCenter)
              .SetProperty(views::kBoxLayoutFlexKey,
                           views::BoxLayoutFlexSpecification())
              .SetBackground(views::CreateThemedSolidBackground(
                  kColorFrameTitleBarBackground))
              .AddChildren(
                  views::Builder<views::ImageView>()
                      .CopyAddressTo(&qr_code_image_)
                      .SetProperty(views::kBoxLayoutFlexKey,
                                   views::BoxLayoutFlexSpecification())
                      .SetImageSize(GetQRCodeImageSize()),
                  views::Builder<views::Label>().SetText(u"W").SetTextStyle(
                      views::style::STYLE_HEADLINE_4_BOLD),
                  views::Builder<views::BoxLayoutView>()
                      .SetInsideBorderInsets(gfx::Insets::TLBR(0, 28, 0, 32))
                      .AddChild(
                          views::Builder<views::LabelButton>()
                              .SetBackground(views::CreateRoundedRectBackground(
                                  0xFF49b558, 4))
                              .SetEnabledTextColorIds(
                                  kColorFrameTitleBarBackground)
                              .SetProperty(views::kBoxLayoutFlexKey,
                                           views::BoxLayoutFlexSpecification())
                              .SetHorizontalAlignment(
                                  gfx::HorizontalAlignment::ALIGN_CENTER)
                              .SetText(u"进入微信")
                              .SetCallback(std::move(finish_callback)
                                               .Then(base::BindOnce(
                                                   &views::Widget::Close,
                                                   base::Unretained(widget))))),
                  views::Builder<views::BoxLayoutView>()
                      .SetBetweenChildSpacing(8)
                      .AddChildren(
                          views::Builder<views::Label>()
                              .SetText(u"切换账号")
                              .SetTextStyle(views::style::STYLE_LINK),
                          views::Builder<views::Separator>().SetOrientation(
                              views::Separator::Orientation::kVertical),
                          views::Builder<views::Label>()
                              .SetText(u"仅传输文件")
                              .SetTextStyle(views::style::STYLE_LINK))))
      .BuildChildren();

  UpdateQRContent();
}

int SigninFrameView::NonClientHitTest(const gfx::Point& point) {
  return views::GetHitTestComponent(this, point);
}

void SigninFrameView::UpdateQRContent() {
  std::string input = base::UTF16ToUTF8(u"这是一个测试");

  base::expected<gfx::ImageSkia, qr_code_generator::Error> qr_code =
      qr_code_generator::GenerateImage(
          base::as_byte_span(input), qr_code_generator::ModuleStyle::kCircles,
          qr_code_generator::LocatorStyle::kRounded,
          qr_code_generator::CenterImage::kNoCenterImage,
          qr_code_generator::QuietZone::kIncluded);

  if (!qr_code.has_value()) {
    return;
  }

  UpdateQRImage(qr_code.value());
}

void SigninFrameView::UpdateQRImage(gfx::ImageSkia qr_image) {
  qr_code_image_->SetImage(ui::ImageModel::FromImageSkia(qr_image));
}

BEGIN_METADATA(SigninFrameView)
END_METADATA
