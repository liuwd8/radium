// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/tab_contents/radium_web_contents_view_focus_helper.h"

#include "base/memory/ptr_util.h"
#include "components/web_modal/web_contents_modal_dialog_manager.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/widget/widget.h"

// #include "chrome/browser/ui/sad_tab_helper.h"
// #include "chrome/browser/ui/views/sad_tab_view.h"

RadiumWebContentsViewFocusHelper::RadiumWebContentsViewFocusHelper(
    content::WebContents* web_contents)
    : content::WebContentsUserData<RadiumWebContentsViewFocusHelper>(
          *web_contents) {}

bool RadiumWebContentsViewFocusHelper::Focus() {
  // SadTabHelper* sad_tab_helper =
  //     SadTabHelper::FromWebContents(&GetWebContents());
  // if (sad_tab_helper) {
  //   SadTabView* sad_tab =
  //   static_cast<SadTabView*>(sad_tab_helper->sad_tab()); if (sad_tab) {
  //     sad_tab->RequestFocus();
  //     return true;
  //   }
  // }

  const web_modal::WebContentsModalDialogManager* manager =
      web_modal::WebContentsModalDialogManager::FromWebContents(
          &GetWebContents());
  if (manager && manager->IsDialogActive()) {
    manager->FocusTopmostDialog();
    return true;
  }

  return false;
}

bool RadiumWebContentsViewFocusHelper::TakeFocus(bool reverse) {
  views::FocusManager* focus_manager = GetFocusManager();
  if (focus_manager) {
    focus_manager->AdvanceFocus(reverse);
    return true;
  }
  return false;
}

void RadiumWebContentsViewFocusHelper::StoreFocus() {
  last_focused_view_tracker_.SetView(nullptr);
  if (GetFocusManager()) {
    last_focused_view_tracker_.SetView(GetFocusManager()->GetFocusedView());
  }
}

bool RadiumWebContentsViewFocusHelper::RestoreFocus() {
  views::View* view_to_focus = GetStoredFocus();
  last_focused_view_tracker_.SetView(nullptr);
  if (view_to_focus) {
    view_to_focus->RequestFocus();
    return true;
  }
  return false;
}

void RadiumWebContentsViewFocusHelper::ResetStoredFocus() {
  last_focused_view_tracker_.SetView(nullptr);
}

views::View* RadiumWebContentsViewFocusHelper::GetStoredFocus() {
  views::View* last_focused_view = last_focused_view_tracker_.view();
  views::FocusManager* focus_manager = GetFocusManager();
  if (last_focused_view && focus_manager && last_focused_view->IsFocusable() &&
      focus_manager->ContainsView(last_focused_view)) {
    return last_focused_view;
  }
  return nullptr;
}

gfx::NativeView RadiumWebContentsViewFocusHelper::GetActiveNativeView() {
  return GetWebContents().GetNativeView();
}

views::Widget* RadiumWebContentsViewFocusHelper::GetTopLevelWidget() {
  return views::Widget::GetTopLevelWidgetForNativeView(GetActiveNativeView());
}

views::FocusManager* RadiumWebContentsViewFocusHelper::GetFocusManager() {
  views::Widget* toplevel_widget = GetTopLevelWidget();
  return toplevel_widget ? toplevel_widget->GetFocusManager() : nullptr;
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(RadiumWebContentsViewFocusHelper);
