// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/wechat/wechat_contact_view.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "ui/base/models/image_model.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/layout/table_layout_view.h"
#include "ui/views/metadata/view_factory_internal.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/window/vector_icons/vector_icons.h"

namespace {
class WechatContactItemView : public views::Button {
 public:
  WechatContactItemView() {
    SetLayoutManager(std::make_unique<views::TableLayout>())
        ->AddColumn(views::LayoutAlignment::kStretch,
                    views::LayoutAlignment::kCenter,
                    views::TableLayout::kFixedSize,
                    views::TableLayout::ColumnSize::kUsePreferred, 0, 0)
        .AddPaddingColumn(views::TableLayout::kFixedSize, 8)
        .AddColumn(views::LayoutAlignment::kStretch,
                   views::LayoutAlignment::kCenter, 1.0f,
                   views::TableLayout::ColumnSize::kUsePreferred, 0, 0)
        .AddRows(2, views::TableLayout::kFixedSize, 0);
  }

  ~WechatContactItemView() override = default;
};

}  // namespace

WechatContactView::WechatContactView(base::RepeatingClosure callback)
    : callback_(callback) {
  auto builder = views::Builder<views::BoxLayoutView>(this)
                     .SetOrientation(views::BoxLayout::Orientation::kVertical)
                     .SetBetweenChildSpacing(8)
                     .SetInsideBorderInsets(gfx::Insets(8));

  for (auto* i : {
           u"文件传输助手",
           u"订阅号",
           u"哈哈哈哈",
           u"哈哈哈哈2",
           u"哈哈哈哈3",
           u"哈哈哈哈4",
           u"哈哈哈哈5",
           u"哈哈哈哈6",
           u"哈哈哈哈7",
           u"哈哈哈哈8",
           u"哈哈哈哈9",
           u"哈哈哈哈10",
           u"哈哈哈哈11",
           u"哈哈哈哈12",
           u"哈哈哈哈13",
           u"哈哈哈哈14",
       }) {
    builder.AddChild(
        views::Builder<views::Button>(std::make_unique<WechatContactItemView>())
            .SetAccessibleName(i)
            .SetCallback(base::BindRepeating(&WechatContactView::OnClick,
                                             base::Unretained(this)))
            .AddChildren(views::Builder<views::ImageView>()
                             .SetImageSize(gfx::Size(25, 25))
                             .SetImage(ui::ImageModel::FromVectorIcon(
                                 views::kWindowControlMaximizeIcon))
                             .SetProperty(views::kTableColAndRowSpanKey,
                                          gfx::Size(1, 2)),
                         views::Builder<views::Label>()
                             .SetText(i)
                             .SetHorizontalAlignment(
                                 gfx::HorizontalAlignment::ALIGN_LEFT),
                         views::Builder<views::Label>()
                             .SetText(u"哈哈哈哈")
                             .SetTextContext(views::style::CONTEXT_BADGE)
                             .SetHorizontalAlignment(
                                 gfx::HorizontalAlignment::ALIGN_LEFT)));
  }

  std::move(builder).BuildChildren();
}

WechatContactView::~WechatContactView() = default;

void WechatContactView::OnClick() {
  callback_.Run();
}
