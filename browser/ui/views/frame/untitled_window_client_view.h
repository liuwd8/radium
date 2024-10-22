// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WINDOW_CLIENT_VIEW_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WINDOW_CLIENT_VIEW_H_

#include "ui/views/window/client_view.h"

class UntitledWindowClientView : public views::ClientView {
  METADATA_HEADER(UntitledWindowClientView, views::ClientView)

 public:
  explicit UntitledWindowClientView();
  UntitledWindowClientView(const UntitledWindowClientView&) = delete;
  UntitledWindowClientView& operator=(const UntitledWindowClientView&) = delete;

  ~UntitledWindowClientView() override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_UNTITLED_WINDOW_CLIENT_VIEW_H_
