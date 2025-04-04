// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/tab_contents/radium_web_contents_view_delegate_views.h"

#include <memory>
#include <utility>

// #include "chrome/browser/defaults.h"
// #include
// "chrome/browser/ui/aura/tab_contents/web_drag_bookmark_handler_aura.h"
// #include "chrome/browser/ui/browser_finder.h"
// #include "chrome/browser/ui/browser_window.h"
// #include "chrome/browser/ui/sad_tab_helper.h"
// #include
// "chrome/browser/ui/views/renderer_context_menu/render_view_context_menu_views.h"
// #include "radium/browser/ui/tab_contents/radium_web_contents_menu_helper.h"
// #include
// "radium/browser/ui/tab_contents/radium_web_contents_view_handle_drop.h"
// #include "chrome/browser/ui/views/sad_tab_view.h"
#include "components/renderer_context_menu/context_menu_delegate.h"
#include "components/renderer_context_menu/render_view_context_menu_base.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_drag_dest_delegate.h"
#include "content/public/common/drop_data.h"
#include "radium/browser/ui/browser.h"
#include "radium/browser/ui/views/tab_contents/radium_web_contents_view_focus_helper.h"
#include "ui/views/widget/widget.h"

RadiumWebContentsViewDelegateViews::RadiumWebContentsViewDelegateViews(
    content::WebContents* web_contents)
    : ContextMenuDelegate(web_contents), web_contents_(web_contents) {
  RadiumWebContentsViewFocusHelper::CreateForWebContents(web_contents);
}

RadiumWebContentsViewDelegateViews::~RadiumWebContentsViewDelegateViews() =
    default;

gfx::NativeWindow RadiumWebContentsViewDelegateViews::GetNativeWindow() {
  // Browser* browser = chrome::FindBrowserWithTab(web_contents_);
  // return browser ? browser->window()->GetNativeWindow() : nullptr;
  return nullptr;
}

content::WebDragDestDelegate*
RadiumWebContentsViewDelegateViews::GetDragDestDelegate() {
  // // We install a chrome specific handler to intercept bookmark drags for the
  // // bookmark manager/extension API.
  // bookmark_handler_ = std::make_unique<WebDragBookmarkHandlerAura>();
  // return bookmark_handler_.get();
  return nullptr;
}

RadiumWebContentsViewFocusHelper*
RadiumWebContentsViewDelegateViews::GetFocusHelper() const {
  RadiumWebContentsViewFocusHelper* helper =
      RadiumWebContentsViewFocusHelper::FromWebContents(web_contents_);
  DCHECK(helper);
  return helper;
}

bool RadiumWebContentsViewDelegateViews::Focus() {
  return GetFocusHelper()->Focus();
}

bool RadiumWebContentsViewDelegateViews::TakeFocus(bool reverse) {
  return GetFocusHelper()->TakeFocus(reverse);
}

void RadiumWebContentsViewDelegateViews::StoreFocus() {
  GetFocusHelper()->StoreFocus();
}

bool RadiumWebContentsViewDelegateViews::RestoreFocus() {
  return GetFocusHelper()->RestoreFocus();
}

void RadiumWebContentsViewDelegateViews::ResetStoredFocus() {
  GetFocusHelper()->ResetStoredFocus();
}

std::unique_ptr<RenderViewContextMenuBase>
RadiumWebContentsViewDelegateViews::BuildMenu(
    content::RenderFrameHost& render_frame_host,
    const content::ContextMenuParams& params) {
  // std::unique_ptr<RenderViewContextMenuBase> menu(
  //     RenderViewContextMenuViews::Create(render_frame_host, params));
  // menu->Init();
  // return menu;
  return nullptr;
}

void RadiumWebContentsViewDelegateViews::ShowMenu(
    std::unique_ptr<RenderViewContextMenuBase> menu) {
  context_menu_ = std::move(menu);
  if (!context_menu_) {
    return;
  }

  context_menu_->Show();
}

void RadiumWebContentsViewDelegateViews::ShowContextMenu(
    content::RenderFrameHost& render_frame_host,
    const content::ContextMenuParams& params) {
  // ShowMenu(BuildMenu(
  //     render_frame_host,
  //     AddContextMenuParamsPropertiesFromPreferences(web_contents_, params)));
}

void RadiumWebContentsViewDelegateViews::ExecuteCommandForTesting(
    int command_id,
    int event_flags) {
  DCHECK(context_menu_);
  // context_menu_->ExecuteCommand(command_id, event_flags);
  // context_menu_.reset();
}

void RadiumWebContentsViewDelegateViews::OnPerformingDrop(
    const content::DropData& drop_data,
    DropCompletionCallback callback) {
  // HandleOnPerformingDrop(web_contents_, drop_data, std::move(callback));
}

std::unique_ptr<content::WebContentsViewDelegate> CreateWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return std::make_unique<RadiumWebContentsViewDelegateViews>(web_contents);
}
