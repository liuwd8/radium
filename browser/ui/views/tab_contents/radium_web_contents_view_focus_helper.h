// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_FOCUS_HELPER_H_
#define RADIUM_BROWSER_UI_VIEWS_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_FOCUS_HELPER_H_

#include "content/public/browser/web_contents_user_data.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/views/view_tracker.h"

namespace content {
class WebContents;
}

namespace views {
class FocusManager;
class Widget;
class View;
}  // namespace views

// A chrome specific helper class that handles focus management.
class RadiumWebContentsViewFocusHelper
    : public content::WebContentsUserData<RadiumWebContentsViewFocusHelper> {
 public:
  RadiumWebContentsViewFocusHelper(const RadiumWebContentsViewFocusHelper&) =
      delete;
  RadiumWebContentsViewFocusHelper& operator=(
      const RadiumWebContentsViewFocusHelper&) = delete;

  void StoreFocus();
  bool RestoreFocus();
  void ResetStoredFocus();
  bool Focus();
  bool TakeFocus(bool reverse);
  // Returns the View that will be focused if RestoreFocus() is called.
  views::View* GetStoredFocus();

 private:
  explicit RadiumWebContentsViewFocusHelper(content::WebContents* web_contents);
  friend class content::WebContentsUserData<RadiumWebContentsViewFocusHelper>;
  gfx::NativeView GetActiveNativeView();
  views::Widget* GetTopLevelWidget();
  views::FocusManager* GetFocusManager();

  // Used to store the last focused view.
  views::ViewTracker last_focused_view_tracker_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // RADIUM_BROWSER_UI_VIEWS_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_FOCUS_HELPER_H_
