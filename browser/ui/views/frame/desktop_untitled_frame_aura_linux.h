// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_AURA_LINUX_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_AURA_LINUX_H_

#include "radium/browser/ui/views/frame/desktop_untitled_frame_aura.h"

class DesktopUntitledFrameAuraLinux : public DesktopUntitledFrameAura {
 public:
  explicit DesktopUntitledFrameAuraLinux(UntitledWidget* untitled_widget);
  DesktopUntitledFrameAuraLinux(const DesktopUntitledFrameAuraLinux&) = delete;
  DesktopUntitledFrameAuraLinux& operator=(
      const DesktopUntitledFrameAuraLinux&) = delete;

 protected:
  ~DesktopUntitledFrameAuraLinux() override;

  // NativeBrowserFrame:
  views::Widget::InitParams GetWidgetParams() override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_DESKTOP_UNTITLED_FRAME_AURA_LINUX_H_
