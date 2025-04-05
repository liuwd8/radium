// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_WINDOW_H_
#define RADIUM_BROWSER_UI_BROWSER_WINDOW_H_

class Browser;
class DragContext;

namespace views {
class Widget;
}  // namespace views

class BrowserWindow {
 public:
  static constexpr const char kBrowserWindow[] = "__BROWSER_WINDOW__";

  virtual ~BrowserWindow() = default;

  virtual void Close() = 0;
  virtual Browser* GetBrowser() = 0;
  virtual DragContext* GetDragContext();
  virtual views::Widget* GetWidget() = 0;
  virtual void SetFocusToLocationBar();
  virtual bool ShouldFocusLocationBarByDefault();
  virtual void Show() = 0;

 protected:
  void AddBrowserWindowKeyToWidget();
};

#endif  // RADIUM_BROWSER_UI_BROWSER_WINDOW_H_
