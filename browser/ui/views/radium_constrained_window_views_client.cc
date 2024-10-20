// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/platform_util.h"
#include "radium/browser/ui/views/radium_constrained_window_views_client.h"

namespace {

class RadiumConstrainedWindowViewsClient
    : public constrained_window::ConstrainedWindowViewsClient {
 public:
  RadiumConstrainedWindowViewsClient() = default;

  RadiumConstrainedWindowViewsClient(
      const RadiumConstrainedWindowViewsClient&) = delete;
  RadiumConstrainedWindowViewsClient& operator=(
      const RadiumConstrainedWindowViewsClient&) = delete;

  ~RadiumConstrainedWindowViewsClient() override = default;

 private:
  // ConstrainedWindowViewsClient:
  web_modal::ModalDialogHost* GetModalDialogHost(
      gfx::NativeWindow parent) override {
    // TODO:
    return nullptr;
  }
  gfx::NativeView GetDialogHostView(gfx::NativeWindow parent) override {
    return platform_util::GetViewForWindow(parent);
  }
};

}  // namespace

std::unique_ptr<constrained_window::ConstrainedWindowViewsClient>
CreateRadiumConstrainedWindowViewsClient() {
  return std::make_unique<RadiumConstrainedWindowViewsClient>();
}
