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
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/compositor/clip_recorder.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/gfx/skia_paint_util.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/frame_background.h"

namespace {

// This is the same thickness as the resize border on ChromeOS.
constexpr unsigned int kResizeBorder = 10;
constexpr int kBorderAlpha = 0x26;

}  // namespace

gfx::ShadowValues UntitledWidgetFrameViewLinux::GetShadowValues(bool active) {
  int elevation = RadiumLayoutProvider::Get()->GetShadowElevationMetric(
      active ? views::Emphasis::kMaximum : views::Emphasis::kMedium);
  return gfx::ShadowValue::MakeMdShadowValues(elevation);
}

UntitledWidgetFrameViewLinux::UntitledWidgetFrameViewLinux(
    UntitledWidget* untitled_widget)
    : OpaqueFrameView(untitled_widget) {}

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

void UntitledWidgetFrameViewLinux::PaintChildren(const views::PaintInfo& info) {
  // Clip children based on the tab's fill path.  This has no effect except when
  // the tab is too narrow to completely show even one icon, at which point this
  // serves to clip the favicon.
  ui::ClipRecorder clip_recorder(info.context());
  // The paint recording scale for tabs is consistent along the x and y axis.
  const float paint_recording_scale = info.paint_recording_scale_x();
  const bool tiled = untitled_widget()->tiled();
  bool showing_shadow = ShouldDrawRestoredFrameShadow();
  SkRRect clip = GetRestoredClipRegion();

  // If rendering shadows, draw a 1px exterior border, otherwise draw a 1px
  // interior border.
  const SkScalar one_pixel = SkFloatToScalar(1 / paint_recording_scale);
  if (!tiled && !showing_shadow) {
    clip.inset(one_pixel, one_pixel);
  }

  SkPath clip_path;
  clip_path.addRRect(clip);
  clip_recorder.ClipPathWithAntiAliasing(clip_path);
  View::PaintChildren(info);
}

void UntitledWidgetFrameViewLinux::PaintRestoredFrameBorder(
    gfx::Canvas* canvas) const {
  const bool tiled = untitled_widget()->tiled();
  const bool is_active = ShouldPaintAsActive();
  auto shadow_values = tiled ? gfx::ShadowValues() : GetShadowValues(is_active);

  const auto* color_provider = GetColorProvider();
  SkRRect clip = GetRestoredClipRegion();
  bool showing_shadow = ShouldDrawRestoredFrameShadow();
  gfx::Insets border = GetInsets();
  if (frame_background()) {
    gfx::ScopedCanvas scoped_canvas(canvas);
    canvas->sk_canvas()->clipRRect(clip, SkClipOp::kIntersect, true);
    auto shadow_inset = showing_shadow ? border : gfx::Insets();
    frame_background()->PaintMaximized(canvas, GetNativeTheme(), color_provider,
                                       shadow_inset.left(), shadow_inset.top(),
                                       width() - shadow_inset.width());
    if (!showing_shadow) {
      frame_background()->FillFrameBorders(canvas, this, border.left(),
                                           border.right(), border.bottom());
    }
  }

  // If rendering shadows, draw a 1px exterior border, otherwise draw a 1px
  // interior border.
  const SkScalar one_pixel = SkFloatToScalar(1 / canvas->image_scale());
  SkRRect outset_rect = clip;
  SkRRect inset_rect = clip;
  if (tiled) {
    outset_rect.outset(1, 1);
  } else if (showing_shadow) {
    outset_rect.outset(one_pixel, one_pixel);
  } else {
    inset_rect.inset(one_pixel, one_pixel);
  }

  cc::PaintFlags flags;
  const SkColor frame_color = color_provider->GetColor(
      is_active ? ui::kColorFrameActive : ui::kColorFrameInactive);
  const SkColor border_color =
      showing_shadow ? SK_ColorBLACK
                     : color_utils::PickContrastingColor(
                           SK_ColorBLACK, SK_ColorWHITE, frame_color);
  flags.setColor(SkColorSetA(border_color, kBorderAlpha));
  flags.setAntiAlias(true);
  if (showing_shadow) {
    flags.setLooper(gfx::CreateShadowDrawLooper(shadow_values));
  }

  gfx::ScopedCanvas scoped_canvas(canvas);
  canvas->sk_canvas()->clipRRect(inset_rect, SkClipOp::kDifference, true);
  canvas->sk_canvas()->drawRRect(outset_rect, flags);
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

  auto shadow_values = tiled ? gfx::ShadowValues() : GetShadowValues(true);

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
  const bool tiled = untitled_widget()->tiled();
  if (tiled || !views::Widget::IsWindowCompositingSupported()) {
    return 0;
  }
  return RadiumLayoutProvider::Get()->GetCornerRadiusMetric(
      views::Emphasis::kHigh);
}

BEGIN_METADATA(UntitledWidgetFrameViewLinux)
END_METADATA
