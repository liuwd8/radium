// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/wechat/wechat_side_menu_view.h"

#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/models/image_model.h"
#include "ui/gfx/geometry/rounded_corners_f.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/gfx/vector_icon_types.h"
#include "ui/views/background.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/window/vector_icons/vector_icons.h"

WechatSideMenuView::WechatSideMenuView() {
  auto builder = views::Builder<views::BoxLayoutView>(this)
                     .SetBackground(views::CreateThemedRoundedRectBackground(
                         kWechatSideMenuBackground,
                         gfx::RoundedCornersF(
                             RadiumLayoutProvider::Get()->GetCornerRadiusMetric(
                                 views::Emphasis::kHigh),
                             0, 0, 0)))
                     .SetOrientation(views::BoxLayout::Orientation::kVertical)
                     .SetBetweenChildSpacing(25)
                     .SetInsideBorderInsets(gfx::Insets::VH(40, 13));

  for (auto* i :
       {&views::kWindowControlMaximizeIcon, &views::kWindowControlMaximizeIcon,
        &views::kWindowControlMaximizeIcon, &views::kWindowControlMaximizeIcon,
        &views::kWindowControlMaximizeIcon}) {
    builder.AddChild(views::Builder<views::ImageView>()
                         .SetImage(ui::ImageModel::FromVectorIcon(*i))
                         .SetImageSize(gfx::Size(38, 38)));
  }

  std::move(builder)
      .AddChildren(
          views::Builder<views::View>().SetProperty(
              views::kBoxLayoutFlexKey, views::BoxLayoutFlexSpecification()),
          views::Builder<views::ImageView>()
              .SetImage(ui::ImageModel::FromVectorIcon(
                  views::kWindowControlMaximizeIcon))
              .SetImageSize(gfx::Size(38, 38)))
      .BuildChildren();
}

WechatSideMenuView::~WechatSideMenuView() = default;

BEGIN_METADATA(WechatSideMenuView)
END_METADATA
