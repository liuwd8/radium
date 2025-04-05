// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/browser_window.h"

#include "ui/views/widget/widget.h"

DragContext* BrowserWindow::GetDragContext() {
  return nullptr;
}

void BrowserWindow::SetFocusToLocationBar() {}

bool BrowserWindow::ShouldFocusLocationBarByDefault() {
  return true;
}

void BrowserWindow::AddBrowserWindowKeyToWidget() {
  GetWidget()->SetNativeWindowProperty(BrowserWindow::kBrowserWindow, this);
}
