// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_desktop_window_tree_host_linux.h"

#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/platform_window/platform_window_init_properties.h"
#include "ui/views/window/non_client_view.h"

namespace {
bool IsShowingFrame(bool use_custom_frame,
                    ui::PlatformWindowState window_state) {
  return use_custom_frame &&
         window_state != ui::PlatformWindowState::kMaximized &&
         window_state != ui::PlatformWindowState::kMinimized &&
         !ui::IsPlatformWindowStateFullscreen(window_state);
}

}  // namespace

UntitledDesktopWindowTreeHostLinux::UntitledDesktopWindowTreeHostLinux(
    views::internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura,
    UntitledWidget* untitled_widget)
    : DesktopWindowTreeHostLinux(native_widget_delegate,
                                 desktop_native_widget_aura),
      untitled_widget_(untitled_widget) {
  theme_observation_.Observe(ui::NativeTheme::GetInstanceForNativeUi());
  if (auto* linux_ui = ui::LinuxUi::instance()) {
    scale_observation_.Observe(linux_ui);
  }
}

UntitledDesktopWindowTreeHostLinux::~UntitledDesktopWindowTreeHostLinux() =
    default;

bool UntitledDesktopWindowTreeHostLinux::SupportsClientFrameShadow() const {
  return platform_window()->CanSetDecorationInsets() &&
         views::Widget::IsWindowCompositingSupported();
}

views::DesktopWindowTreeHost*
UntitledDesktopWindowTreeHostLinux::AsDesktopWindowTreeHost() {
  return this;
}

int UntitledDesktopWindowTreeHostLinux::GetMinimizeButtonOffset() const {
  return 0;
}

bool UntitledDesktopWindowTreeHostLinux::UsesNativeSystemMenu() const {
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// UntitledDesktopWindowTreeHostLinux,
//     DesktopWindowTreeHostLinux implementation:

void UntitledDesktopWindowTreeHostLinux::OnWidgetInitDone() {
  DesktopWindowTreeHostLinux::OnWidgetInitDone();

  UpdateFrameHints();
}

void UntitledDesktopWindowTreeHostLinux::AddAdditionalInitProperties(
    const views::Widget::InitParams& params,
    ui::PlatformWindowInitProperties* properties) {
  views::DesktopWindowTreeHostLinux::AddAdditionalInitProperties(params,
                                                                 properties);

  // auto* profile = browser_view_->browser()->profile();
  // const auto* linux_ui_theme = ui::LinuxUiTheme::GetForProfile(profile);
  // properties->prefer_dark_theme =
  //     linux_ui_theme && linux_ui_theme->PreferDarkTheme();
}

void UntitledDesktopWindowTreeHostLinux::FrameTypeChanged() {
  DesktopWindowTreeHostPlatform::FrameTypeChanged();
  UpdateFrameHints();
}

void UntitledDesktopWindowTreeHostLinux::UpdateFrameHints() {
  auto* window = platform_window();
  auto window_state = window->GetPlatformWindowState();
  float scale = device_scale_factor();
  auto* view = static_cast<UntitledWidgetNonClientFrameView*>(
      untitled_widget_->non_client_view()->frame_view());
  const gfx::Size widget_size =
      untitled_widget_->GetWindowBoundsInScreen().size();

  if (SupportsClientFrameShadow()) {
    auto insets = CalculateInsetsInDIP(window_state);
    if (insets.IsEmpty()) {
      window->SetInputRegion(std::nullopt);
    } else {
      gfx::Rect input_bounds(widget_size);
      input_bounds.Inset(insets - view->GetInputInsets());
      input_bounds = gfx::ScaleToEnclosingRect(input_bounds, scale);
      window->SetInputRegion(
          std::optional<std::vector<gfx::Rect>>({input_bounds}));
    }
  }

  if (ui::OzonePlatform::GetInstance()->IsWindowCompositingSupported()) {
    // Set the opaque region.
    std::vector<gfx::Rect> opaque_region;
    if (IsShowingFrame(!untitled_widget_->ShouldUseNativeFrame(),
                       window_state)) {
      // The opaque region is a list of rectangles that contain only fully
      // opaque pixels of the window.  We need to convert the clipping
      // rounded-rect into this format.
      SkRRect rrect = view->GetRestoredClipRegion();
      gfx::RectF rectf = gfx::SkRectToRectF(rrect.rect());
      rectf.Scale(scale);
      // It is acceptable to omit some pixels that are opaque, but the region
      // must not include any translucent pixels.  Therefore, we must
      // conservatively scale to the enclosed rectangle.
      gfx::Rect rect = gfx::ToEnclosedRect(rectf);

      // Create the initial region from the clipping rectangle without rounded
      // corners.
      SkRegion region(gfx::RectToSkIRect(rect));

      // Now subtract out the small rectangles that cover the corners.
      struct {
        SkRRect::Corner corner;
        bool left;
        bool upper;
      } kCorners[] = {
          {SkRRect::kUpperLeft_Corner, true, true},
          {SkRRect::kUpperRight_Corner, false, true},
          {SkRRect::kLowerLeft_Corner, true, false},
          {SkRRect::kLowerRight_Corner, false, false},
      };
      for (const auto& corner : kCorners) {
        auto radii = rrect.radii(corner.corner);
        auto rx = std::ceil(scale * radii.x());
        auto ry = std::ceil(scale * radii.y());
        auto corner_rect = SkIRect::MakeXYWH(
            corner.left ? rect.x() : rect.right() - rx,
            corner.upper ? rect.y() : rect.bottom() - ry, rx, ry);
        region.op(corner_rect, SkRegion::kDifference_Op);
      }

      auto translucent_top_area_rect = SkIRect::MakeXYWH(
          rect.x(), rect.y(), rect.width(),
          std::ceil(view->GetTranslucentTopAreaHeight() * scale - rect.y()));
      region.op(translucent_top_area_rect, SkRegion::kDifference_Op);

      // Convert the region to a list of rectangles.
      for (SkRegion::Iterator i(region); !i.done(); i.next()) {
        opaque_region.push_back(gfx::SkIRectToRect(i.rect()));
      }
    } else {
      // The entire window except for the translucent top is opaque.
      gfx::Rect opaque_region_dip(widget_size);
      gfx::Insets insets;
      insets.set_top(view->GetTranslucentTopAreaHeight());
      opaque_region_dip.Inset(insets);
      opaque_region.push_back(
          gfx::ScaleToEnclosingRect(opaque_region_dip, scale));
    }
    window->SetOpaqueRegion(opaque_region);
  }

  SizeConstraintsChanged();
}

gfx::Insets UntitledDesktopWindowTreeHostLinux::CalculateInsetsInDIP(
    ui::PlatformWindowState window_state) const {
  // If we are not showing frame, the insets should be zero.
  if (!IsShowingFrame(!untitled_widget_->ShouldUseNativeFrame(),
                      window_state)) {
    return gfx::Insets();
  }

  return untitled_widget_->non_client_view()->frame_view()->GetInsets();
}

void UntitledDesktopWindowTreeHostLinux::OnWindowStateChanged(
    ui::PlatformWindowState old_state,
    ui::PlatformWindowState new_state) {
  DesktopWindowTreeHostLinux::OnWindowStateChanged(old_state, new_state);
  UpdateFrameHints();
}

void UntitledDesktopWindowTreeHostLinux::OnWindowTiledStateChanged(
    ui::WindowTiledEdges new_tiled_edges) {
  bool maximized = new_tiled_edges.top && new_tiled_edges.left &&
                   new_tiled_edges.bottom && new_tiled_edges.right;
  bool tiled = new_tiled_edges.top || new_tiled_edges.left ||
               new_tiled_edges.bottom || new_tiled_edges.right;
  untitled_widget_->set_tiled(tiled && !maximized);
  UpdateFrameHints();
  if (SupportsClientFrameShadow()) {
    // Trigger a re-layout as the insets will change even if the bounds don't.
    ScheduleRelayout();
    if (GetWidget()->non_client_view()) {
      // This is needed for the decorated regions, borders etc. to be repainted.
      GetWidget()->non_client_view()->SchedulePaint();
    }
  }
}

void UntitledDesktopWindowTreeHostLinux::OnNativeThemeUpdated(
    ui::NativeTheme* observed_theme) {
  UpdateFrameHints();
}

void UntitledDesktopWindowTreeHostLinux::OnDeviceScaleFactorChanged() {
  UpdateFrameHints();
}

////////////////////////////////////////////////////////////////////////////////
// UntitledDesktopWindowTreeHost, public:

// static
UntitledDesktopWindowTreeHost*
UntitledDesktopWindowTreeHost::CreateBrowserDesktopWindowTreeHost(
    views::internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura,
    UntitledWidget* untitled_widget) {
  return new UntitledDesktopWindowTreeHostLinux(
      native_widget_delegate, desktop_native_widget_aura, untitled_widget);
}
