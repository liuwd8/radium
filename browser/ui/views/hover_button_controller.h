// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_HOVER_BUTTON_CONTROLLER_H_
#define RADIUM_BROWSER_UI_VIEWS_HOVER_BUTTON_CONTROLLER_H_

#include <memory>

#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/button_controller.h"

namespace views {
class ButtonControllerDelegate;
}  // namespace views

class HoverButton;

// A controller that contains the logic for a button that's the full width of
// its parent.
class HoverButtonController : public views::ButtonController {
 public:
  HoverButtonController(
      HoverButton* button,
      std::unique_ptr<views::ButtonControllerDelegate> delegate);

  HoverButtonController(const HoverButtonController&) = delete;
  HoverButtonController& operator=(const HoverButtonController&) = delete;

  ~HoverButtonController() override;

  // views::ButtonController:
  bool OnMousePressed(const ui::MouseEvent& event) override;
  void OnMouseReleased(const ui::MouseEvent& event) override;
  bool OnKeyPressed(const ui::KeyEvent& event) override;
  void OnGestureEvent(ui::GestureEvent* event) override;

 private:
  views::Button::PressedCallback& callback();
};

#endif  // RADIUM_BROWSER_UI_VIEWS_HOVER_BUTTON_CONTROLLER_H_
