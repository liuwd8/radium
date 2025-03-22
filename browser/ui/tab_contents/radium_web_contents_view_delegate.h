// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_DELEGATE_H_
#define RADIUM_BROWSER_UI_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_DELEGATE_H_

#include <memory>

namespace content {
class WebContents;
class WebContentsViewDelegate;
}  // namespace content

std::unique_ptr<content::WebContentsViewDelegate> CreateWebContentsViewDelegate(
    content::WebContents* web_contents);

#endif  // RADIUM_BROWSER_UI_TAB_CONTENTS_RADIUM_WEB_CONTENTS_VIEW_DELEGATE_H_
