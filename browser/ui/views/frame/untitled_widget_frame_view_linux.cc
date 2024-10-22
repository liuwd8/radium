// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_frame_view_linux.h"

#include <memory>

#include "base/i18n/rtl.h"
#include "radium/browser/ui/views/frame/opaque_frame_view.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/views/widget/widget.h"

namespace {

// This is the same thickness as the resize border on ChromeOS.
constexpr unsigned int kResizeBorder = 10;

}  // namespace

UntitledWidgetFrameViewLinux::UntitledWidgetFrameViewLinux(
    views::Widget* widget)
    : OpaqueFrameView(widget) {}

UntitledWidgetFrameViewLinux::~UntitledWidgetFrameViewLinux() = default;

gfx::Insets UntitledWidgetFrameViewLinux::RestoredMirroredFrameBorderInsets()
    const {
  gfx::Insets border = GetInsets();
  return base::i18n::IsRTL() ? gfx::Insets::TLBR(border.top(), border.right(),
                                                 border.bottom(), border.left())
                             : border;
}

gfx::Insets UntitledWidgetFrameViewLinux::GetInputInsets() const {
  return ShouldDrawRestoredFrameShadow() && !IsFrameCondensed()
             ? gfx::Insets(kResizeBorder)
             : gfx::Insets();
}

SkRRect UntitledWidgetFrameViewLinux::GetRestoredClipRegion() const {
  gfx::RectF bounds_dip(GetLocalBounds());
  if (ShouldDrawRestoredFrameShadow()) {
    gfx::InsetsF border(RestoredMirroredFrameBorderInsets());
    bounds_dip.Inset(border);
  }
  float radius_dip = GetRestoredCornerRadiusDip();
  SkVector radii[4]{{radius_dip, radius_dip}, {radius_dip, radius_dip}, {}, {}};
  SkRRect clip;
  clip.setRectRadii(gfx::RectFToSkRect(bounds_dip), radii);
  return clip;
}

bool UntitledWidgetFrameViewLinux::ShouldDrawRestoredFrameShadow() const {
  return true;
}

gfx::Insets UntitledWidgetFrameViewLinux::GetInsets() const {
  if (!ShouldDrawRestoredFrameShadow()) {
    auto no_shadow_border = OpaqueFrameView::GetInsets();
    no_shadow_border.set_top(0);
    return no_shadow_border;
  }

  const UntitledWidget* untitled_widget =
      static_cast<const UntitledWidget*>(GetWidget());
  const bool tiled = untitled_widget->tiled();

  auto shadow_values =
      tiled ? gfx::ShadowValues()
            : gfx::ShadowValue::MakeMdShadowValues(
                  RadiumLayoutProvider::Get()->GetShadowElevationMetric(
                      views::Emphasis::kMaximum));

  // The border must be at least as large as the shadow.
  gfx::Rect frame_extents;
  for (const auto& shadow_value : shadow_values) {
    const auto shadow_radius = shadow_value.blur() / 4;
    const gfx::InsetsF shadow_insets(shadow_radius);
    gfx::RectF shadow_extents;
    shadow_extents.Inset(-shadow_insets);
    shadow_extents.set_origin(shadow_extents.origin() + shadow_value.offset());
    frame_extents.Union(gfx::ToEnclosingRect(shadow_extents));
  }

  // The border must be at least as large as the input region.
  const gfx::Insets insets(kResizeBorder);
  gfx::Rect input_extents;
  input_extents.Inset(-insets);
  frame_extents.Union(input_extents);

  return gfx::Insets::TLBR(-frame_extents.y(), -frame_extents.x(),
                           frame_extents.bottom(), frame_extents.right());
}

float UntitledWidgetFrameViewLinux::GetRestoredCornerRadiusDip() const {
  const UntitledWidget* untitled_widget =
      static_cast<const UntitledWidget*>(GetWidget());
  const bool tiled = untitled_widget->tiled();
  if (tiled || !views::Widget::IsWindowCompositingSupported()) {
    return 0;
  }
  return RadiumLayoutProvider::Get()->GetCornerRadiusMetric(
      views::Emphasis::kHigh);
}
