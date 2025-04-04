// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/traffic_utils.h"

#import <Cocoa/Cocoa.h>
#include <objc/NSObject.h>

#include <optional>

#include "base/i18n/rtl.h"
#include "base/scoped_observation.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/point.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"

@interface WindowButtonsProxy : NSObject {
 @private
  NSWindow* window_;

  // Current left-top margin of buttons.
  gfx::Point margin_;
  // The default left-top margin.
  gfx::Point default_margin_;
};

// Set left-top margin of the window buttons..
- (void)setMargin:(const std::optional<gfx::Point>&)margin;
- (void)redraw;

@end

namespace {
class TrafficLightsTracker : views::WidgetObserver {
 public:
  explicit TrafficLightsTracker(WindowButtonsProxy* buttons_proxy)
      : buttons_proxy_(buttons_proxy) {}

  ~TrafficLightsTracker() override = default;

  void Observe(views::Widget* widget) { scoped_observation_.Observe(widget); }

 private:
  void OnWidgetShowStateChanged(views::Widget* widget) override {
    [buttons_proxy_ redraw];
  }
  void OnWidgetDestroyed(views::Widget* widget) override { delete this; }

 private:
  WindowButtonsProxy* __strong buttons_proxy_;
  base::ScopedObservation<views::Widget, views::WidgetObserver>
      scoped_observation_{this};
};
}  // namespace

@implementation WindowButtonsProxy

- (id)initWithWindow:(NSWindow*)window {
  window_ = window;

  // Remember the default margin.
  margin_ = default_margin_ = [self getCurrentMargin];

  return self;
}

- (void)setMargin:(const std::optional<gfx::Point>&)margin {
  if (margin) {
    margin_ = *margin;
  } else {
    margin_ = default_margin_;
  }
  [self redraw];
}

- (void)redraw {
  NSView* left = [self leftButton];
  NSView* middle = [self middleButton];
  NSView* right = [self rightButton];

  float button_width = NSWidth(left.frame);
  float button_y = NSMinY(left.frame);
  float padding = NSMinX(middle.frame) - NSMaxX(left.frame);
  float start;
  if (base::i18n::IsRTL()) {
    start =
        NSWidth(window_.frame) - 3 * button_width - 2 * padding - margin_.x();
  } else {
    start = margin_.x();
  }

  [left setFrameOrigin:NSMakePoint(start, button_y)];
  start += button_width + padding;
  [middle setFrameOrigin:NSMakePoint(start, button_y)];
  start += button_width + padding;
  [right setFrameOrigin:NSMakePoint(start, button_y)];
}

// Compute margin from position of current buttons.
- (gfx::Point)getCurrentMargin {
  gfx::Point result;
  NSView* titleBarContainer = [self titleBarContainer];
  if (!titleBarContainer) {
    return result;
  }

  NSView* left = [self leftButton];
  NSView* right = [self rightButton];

  result.set_y(NSMinY(left.frame));

  if (base::i18n::IsRTL()) {
    result.set_x(NSWidth(window_.frame) - NSMaxX(right.frame));
  } else {
    result.set_x(NSMinX(left.frame));
  }
  return result;
}

// Receive the titlebar container, which might be nil if the window does not
// have the NSWindowStyleMaskTitled style.
- (NSView*)titleBarContainer {
  NSView* left = [self leftButton];
  if (!left.superview) {
    return nil;
  }
  return left.superview.superview;
}

// Receive the window buttons, note that the buttons might be removed and
// re-added on the fly so we should not cache them.
- (NSButton*)leftButton {
  if (base::i18n::IsRTL()) {
    return [window_ standardWindowButton:NSWindowZoomButton];
  } else {
    return [window_ standardWindowButton:NSWindowCloseButton];
  }
}

- (NSButton*)middleButton {
  return [window_ standardWindowButton:NSWindowMiniaturizeButton];
}

- (NSButton*)rightButton {
  if (base::i18n::IsRTL()) {
    return [window_ standardWindowButton:NSWindowCloseButton];
  } else {
    return [window_ standardWindowButton:NSWindowZoomButton];
  }
}

@end

void SetTrafficLightsMargin(views::Widget* widget, const gfx::Insets& insets) {
  NSWindow* window = widget->GetNativeWindow().GetNativeNSWindow();
  if (!window) {
    return;
  }
  WindowButtonsProxy* buttons_proxy =
      [[WindowButtonsProxy alloc] initWithWindow:window];
  gfx::Point p(insets.top(), insets.left());
  [buttons_proxy setMargin:p];
  (new TrafficLightsTracker(buttons_proxy))->Observe(widget);
}
