// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_WINDOW_H_
#define RADIUM_BROWSER_UI_BROWSER_WINDOW_H_

namespace views {
class Widget;
}

class BrowserWindow {
 public:
  virtual ~BrowserWindow() = default;

  virtual views::Widget* GetWidget() = 0;
  virtual void Show() = 0;
  virtual void Close() = 0;
};

#endif  // RADIUM_BROWSER_UI_BROWSER_WINDOW_H_
