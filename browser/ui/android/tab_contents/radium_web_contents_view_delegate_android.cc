// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/android/tab_contents/radium_web_contents_view_delegate_android.h"

#include <memory>

// #include "chrome/browser/ui/android/context_menu_helper.h"
#include "base/notreached.h"
#include "content/public/browser/context_menu_params.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"

RadiumWebContentsViewDelegateAndroid::RadiumWebContentsViewDelegateAndroid(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {}

RadiumWebContentsViewDelegateAndroid::~RadiumWebContentsViewDelegateAndroid() =
    default;

content::WebDragDestDelegate*
RadiumWebContentsViewDelegateAndroid::GetDragDestDelegate() {
  // GetDragDestDelegate is a pure virtual method from WebContentsViewDelegate
  // and must have an implementation although android doesn't use it.
  NOTREACHED();
}

void RadiumWebContentsViewDelegateAndroid::ShowContextMenu(
    content::RenderFrameHost& render_frame_host,
    const content::ContextMenuParams& params) {
  // // TODO(dtrainor, kouhei): Give WebView a Populator/delegate so it can use
  // // the same context menu code.
  // ContextMenuHelper* helper =
  // ContextMenuHelper::FromWebContents(web_contents_); if (helper) {
  //   helper->ShowContextMenu(render_frame_host, params);
  // }
}

void RadiumWebContentsViewDelegateAndroid::DismissContextMenu() {
  // // ContextMenuHelper is a WebContentsUserData, so it will be the same obj
  // used
  // // in #ShowContextMenu().
  // ContextMenuHelper* helper =
  // ContextMenuHelper::FromWebContents(web_contents_); if (helper) {
  //   helper->DismissContextMenu();
  // }
}

std::unique_ptr<content::WebContentsViewDelegate> CreateWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return std::make_unique<RadiumWebContentsViewDelegateAndroid>(web_contents);
}
