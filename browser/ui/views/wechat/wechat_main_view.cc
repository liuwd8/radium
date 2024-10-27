// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/wechat/wechat_main_view.h"

#include <memory>

#include "base/functional/bind.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "radium/browser/ui/views/wechat/search_textfield.h"
#include "radium/browser/ui/views/wechat/wechat_contact_view.h"
#include "radium/browser/ui/views/wechat/wechat_side_menu_view.h"
#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/layout/table_layout.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/window/caption_button_layout_constants.h"
#include "ui/views/window/caption_button_types.h"
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

WechatMainView::WechatMainView()
    : keep_alive_(KeepAliveOrigin::USER_MANAGER_VIEW,
                  KeepAliveRestartOption::DISABLED) {
  SetHasWindowSizeControls(true);
  SetTitle(u"WeChat");
  SetBackground(views::CreateThemedRoundedRectBackground(
      kColorFrameTitleBarBackground,
      RadiumLayoutProvider::Get()->GetCornerRadiusMetric(
          views::Emphasis::kHigh),
      0, 0));
  views::TableLayout* layout =
      SetLayoutManager(std::make_unique<views::TableLayout>());
  layout
      ->AddColumn(views::LayoutAlignment::kStart,
                  views::LayoutAlignment::kStretch, 0,
                  views::TableLayout::ColumnSize::kFixed, 72, 0)
      .AddColumn(views::LayoutAlignment::kStretch,
                 views::LayoutAlignment::kStretch, 0,
                 views::TableLayout::ColumnSize::kFixed, 110, 0)
      .AddColumn(views::LayoutAlignment::kStretch,
                 views::LayoutAlignment::kStretch, 1.0,
                 views::TableLayout::ColumnSize::kUsePreferred, 0, 420)
      .AddRows(2, views::TableLayout::kFixedSize)
      .AddRows(1, 1.0);

  SetEventTargeter(std::make_unique<views::ViewTargeter>(this));
  views::Builder<views::View>(this)
      .AddChildren(
          views::Builder<views::BoxLayoutView>(
              std::make_unique<WechatSideMenuView>())
              .SetProperty(views::kTableColAndRowSpanKey, gfx::Size(1, 3)),
          views::Builder<views::View>()
              .SetUseDefaultFillLayout(true)
              .CustomConfigure(base::BindOnce([](views::View* view) {
                views::SetHitTestComponent(view, HTCAPTION);
              })),
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
                          HTSYSMENU, views::kWindowControlMinimizeIcon))
                      .SetPreferredSize(
                          gfx::Size(views::GetCaptionButtonWidth(), 0)),
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
                          gfx::Size(views::GetCaptionButtonWidth(), 0))),
          views::Builder<views::Textfield>(std::make_unique<SearchTextfield>())
              .SetPlaceholderText(u"搜索"),
          views::Builder<views::Label>()
              .SetText(u"文件传输助手")
              .SetBorder(views::CreateEmptyBorder(4))
              .SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT),
          views::Builder<views::ScrollView>(
              std::make_unique<views::ScrollView>(
                  views::ScrollView::ScrollWithLayers::kEnabled))
              .SetVerticalScrollBarMode(
                  views::ScrollView::ScrollBarMode::kHiddenButEnabled)
              .SetContents(views::Builder<views::View>(
                  std::make_unique<WechatContactView>(base::BindRepeating(
                      &WechatMainView::LoadURL, base::Unretained(this)))))
              .SetBorder(views::CreateThemedSolidSidedBorder(
                  gfx::Insets::TLBR(0, 0, 0, 1), kWechatSideMenuBackground)),
          views::Builder<views::WebView>().CopyAddressTo(&webview_))
      .BuildChildren();
}

WechatMainView::~WechatMainView() = default;

void WechatMainView::Show(Profile* profile) {
  auto* widget = new UntitledWidget(this, profile);
  views::Widget::InitParams params = widget->GetUntitledWidgetParams();
  params.delegate = this;
  params.name = "WeChat";
#if BUILDFLAG(IS_LINUX)
  params.wm_class_name = "WeChat";
  params.wm_class_class = "WeChat";
  params.wayland_app_id = params.wm_class_class;
#endif
  widget->SetTitleBarBackgroundColor(kColorFrameTitleBarBackground);
  widget->Init(std::move(params));

  webview_->SetBrowserContext(profile);

  widget->Show();
}

int WechatMainView::NonClientHitTest(const gfx::Point& point) {
  return views::GetHitTestComponent(this, point);
}

gfx::Size WechatMainView::GetMinimumSize() const {
  return gfx::Size(400, 400);
}

void WechatMainView::LoadURL() {
  webview_->LoadInitialURL(GURL("https://pc.weixin.qq.com"));
}

BEGIN_METADATA(WechatMainView)
END_METADATA
