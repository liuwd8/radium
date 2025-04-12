// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_widget_frame_view_win.h"

#include "base/trace_event/trace_event.h"
#include "radium/browser/ui/color/radium_color_id.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "radium/browser/ui/views/frame/untitled_widget_delegate.h"
#include "radium/browser/ui/views/frame/untitled_widget_non_client_frame_view.h"
#include "ui/base/hit_test.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/win/hwnd_metrics.h"
#include "ui/display/win/dpi.h"
#include "ui/display/win/screen_win.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/win/hwnd_util.h"

UntitledWidgetFrameViewWin::UntitledWidgetFrameViewWin(
    UntitledWidget* untitled_widget)
    : UntitledWidgetNonClientFrameView(untitled_widget) {}

UntitledWidgetFrameViewWin::~UntitledWidgetFrameViewWin() = default;

gfx::Rect UntitledWidgetFrameViewWin::GetBoundsForClientView() const {
  gfx::Insets insets;
  gfx::Rect content_bounds = GetContentsBounds();
  content_bounds.Inset(insets);
  return content_bounds;
}

gfx::Rect UntitledWidgetFrameViewWin::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  gfx::Rect window_bounds(client_bounds);
  gfx::Insets insets = GetInsets();
  window_bounds.Inset(-insets);
  return window_bounds;
}

int UntitledWidgetFrameViewWin::NonClientHitTest(const gfx::Point& point) {
  int super_component =
      UntitledWidgetNonClientFrameView::NonClientHitTest(point);
  if (super_component != HTNOWHERE) {
    return super_component;
  }

  // For app windows and popups without a custom titlebar we haven't customized
  // the frame at all so Windows can figure it out.
  if (!untitled_widget()->ShouldBrowserCustomDrawTitlebar()) {
    return HTNOWHERE;
  }

  // If the point isn't within our bounds, then it's in the native portion of
  // the frame so again Windows can figure it out.
  if (!bounds().Contains(point)) {
    return HTNOWHERE;
  }

  // At the window corners the resize area is not actually bigger, but the 16
  // pixels at the end of the top and bottom edges trigger diagonal resizing.
  constexpr int kResizeCornerWidth = 16;

  const int top_border_thickness = 6;

  const int window_component = GetHTComponentForFrame(
      point, gfx::Insets::TLBR(top_border_thickness, 0, 0, 0),
      top_border_thickness, kResizeCornerWidth - FrameBorderThickness(),
      untitled_widget()->widget_delegate()->CanResize());

  const int frame_component =
      untitled_widget()->client_view()->NonClientHitTest(point);

  // // See if we're in the sysmenu region.  We still have to check the tabstrip
  // // first so that clicks in a tab don't get treated as sysmenu clicks.
  // if (frame_component != HTCLIENT &&
  // ShouldShowWindowIcon(TitlebarType::kAny)) {
  //   gfx::Rect sys_menu_region(
  //       0, display::win::ScreenWin::GetSystemMetricsInDIP(SM_CYSIZEFRAME),
  //       display::win::ScreenWin::GetSystemMetricsInDIP(SM_CXSMICON),
  //       display::win::ScreenWin::GetSystemMetricsInDIP(SM_CYSMICON));
  //   if (sys_menu_region.Contains(point)) {
  //     return HTSYSMENU;
  //   }
  // }

  if (frame_component != HTNOWHERE) {
    // If the clientview  registers a hit within it's bounds, it's still
    // possible that the hit target should be top resize since the tabstrip
    // region paints to the top of the frame. If the frame registered a hit for
    // the Top resize, override the client frame target.
    if (window_component == HTTOP && !untitled_widget()->IsMaximized()) {
      return window_component;
    }

    int delegate_component =
        untitled_widget()->delegate()->NonClientHitTest(point);
    if (delegate_component != HTNOWHERE) {
      return delegate_component;
    }

    return frame_component;
  }

  if (window_component != HTNOWHERE) {
    return window_component;
  }

  return HTCAPTION;
}

void UntitledWidgetFrameViewWin::OnPaint(gfx::Canvas* canvas) {
  TRACE_EVENT0("views.frame", "BrowserFrameViewWin::OnPaint");
  if (untitled_widget()->ShouldBrowserCustomDrawTitlebar()) {
    // This is the pixel-accurate version of WindowTopY(). Scaling the DIP
    // values here compounds precision error, which exposes unpainted client
    // area. When restored it uses the system dsf instead of the per-monitor dsf
    // to match Windows' behavior.
    const int y = untitled_widget()->IsMaximized()
                      ? ui::GetFrameThickness(MonitorFromWindow(
                            HWNDForView(this), MONITOR_DEFAULTTONEAREST))
                      : std::floor(display::win::GetDPIScale());

    // Draw the top of the accent border.
    //
    // We let the DWM do this for the other sides of the window by insetting the
    // client area to leave nonclient area available. However, along the top
    // window edge, we have to have zero nonclient area or the DWM will draw a
    // full native titlebar outside our client area. See
    // BrowserDesktopWindowTreeHostWin::GetClientAreaInsets().
    //
    // We could ask the DWM to draw the top accent border in the client area (by
    // calling DwmExtendFrameIntoClientArea() in
    // BrowserDesktopWindowTreeHostWin::UpdateDWMFrame()), but this requires
    // that we leave part of the client surface transparent. If we draw this
    // ourselves, we can make the client surface fully opaque and avoid the
    // power consumption needed for DWM to blend the window contents.
    //
    // So the accent border also has to be opaque. We can blend the titlebar
    // color with the accent border to approximate the native effect.
    const SkColor titlebar_color =
        GetFrameColor(BrowserFrameActiveState::kUseCurrent);

    gfx::ScopedCanvas scoped_canvas(canvas);
    float scale = canvas->UndoDeviceScaleFactor();
    cc::PaintFlags flags;
    flags.setColor(color_utils::GetResultingPaintColor(
        GetColorProvider()->GetColor(ShouldPaintAsActive()
                                         ? kColorAccentBorderActive
                                         : kColorAccentBorderInactive),
        titlebar_color));
    canvas->DrawRect(gfx::RectF(0, 0, width() * scale, y), flags);

    const int titlebar_height =
        untitled_widget()->GetTitleBarBackgroundHeight();
    const gfx::Rect titlebar_rect = gfx::ToEnclosingRect(
        gfx::RectF(0, y, width() * scale, titlebar_height * scale - y));
    // Paint the titlebar first so we have a background if an area isn't covered
    // by the theme image.
    flags.setColor(titlebar_color);
    canvas->DrawRect(titlebar_rect, flags);
    // const gfx::ImageSkia frame_image = GetFrameImage();
    // if (!frame_image.isNull()) {
    //   canvas->TileImageInt(frame_image, 0,
    //                        ThemeProperties::kFrameHeightAboveTabs -
    //                            GetTopInset(false) + titlebar_rect.y(),
    //                        titlebar_rect.x(), titlebar_rect.y(),
    //                        titlebar_rect.width(), titlebar_rect.height(),
    //                        scale, SkTileMode::kRepeat, SkTileMode::kMirror);
    // }
    // const gfx::ImageSkia frame_overlay_image = GetFrameOverlayImage();
    // if (!frame_overlay_image.isNull()) {
    //   canvas->DrawImageInt(
    //       frame_overlay_image, 0, 0, frame_overlay_image.width(),
    //       frame_overlay_image.height(), titlebar_rect.x(), titlebar_rect.y(),
    //       frame_overlay_image.width() * scale,
    //       frame_overlay_image.height() * scale, true);
    // }
  }
}

int UntitledWidgetFrameViewWin::FrameBorderThickness() const {
  return IsFrameCondensed()
             ? 0
             : display::win::GetScreenWin()->GetSystemMetricsInDIP(
                   SM_CXSIZEFRAME);
}

BEGIN_METADATA(UntitledWidgetFrameViewWin)
END_METADATA
