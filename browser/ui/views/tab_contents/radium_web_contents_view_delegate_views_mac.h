// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_DELEGATE_VIEWS_MAC_H_
#define RADIUM_BROWSER_UI_VIEWS_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_DELEGATE_VIEWS_MAC_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "components/renderer_context_menu/context_menu_delegate.h"
#include "content/public/browser/web_contents_view_delegate.h"

class RadiumWebContentsViewFocusHelper;
class RenderViewContextMenuBase;
class WebDragBookmarkHandlerMac;

namespace content {
class RenderWidgetHostView;
class WebContents;
}  // namespace content

class RadiumWebContentsViewDelegateViewsMac
    : public content::WebContentsViewDelegate,
      public ContextMenuDelegate {
 public:
  explicit RadiumWebContentsViewDelegateViewsMac(
      content::WebContents* web_contents);

  RadiumWebContentsViewDelegateViewsMac(
      const RadiumWebContentsViewDelegateViewsMac&) = delete;
  RadiumWebContentsViewDelegateViewsMac& operator=(
      const RadiumWebContentsViewDelegateViewsMac&) = delete;

  ~RadiumWebContentsViewDelegateViewsMac() override;

  // WebContentsViewDelegate:
  gfx::NativeWindow GetNativeWindow() override;
  NSObject<RenderWidgetHostViewMacDelegate>* GetDelegateForHost(
      content::RenderWidgetHost* render_widget_host,
      bool is_popup) override;
  content::WebDragDestDelegate* GetDragDestDelegate() override;
  void ShowContextMenu(content::RenderFrameHost& render_frame_host,
                       const content::ContextMenuParams& params) override;
  void StoreFocus() override;
  bool RestoreFocus() override;
  void ResetStoredFocus() override;
  bool Focus() override;
  bool TakeFocus(bool reverse) override;
  void OnPerformingDrop(const content::DropData& drop_data,
                        DropCompletionCallback callback) override;

  // ContextMenuDelegate:
  std::unique_ptr<RenderViewContextMenuBase> BuildMenu(
      content::RenderFrameHost& render_frame_host,
      const content::ContextMenuParams& params) override;
  void ShowMenu(std::unique_ptr<RenderViewContextMenuBase> menu) override;

 private:
  content::RenderWidgetHostView* GetActiveRenderWidgetHostView() const;
  RadiumWebContentsViewFocusHelper* GetFocusHelper() const;

  // The WebContents that owns the view.
  raw_ptr<content::WebContents> web_contents_;

  // The context menu. Callbacks are asynchronous so we need to keep it around.
  std::unique_ptr<RenderViewContextMenuBase> context_menu_;

  // // The chrome specific delegate that receives events from WebDragDestMac.
  // std::unique_ptr<WebDragBookmarkHandlerMac> bookmark_handler_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_DELEGATE_VIEWS_MAC_H_
