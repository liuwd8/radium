// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_OBSERVER_H_
#define RADIUM_BROWSER_UI_BROWSER_OBSERVER_H_

#include "base/observer_list_types.h"
#include "build/build_config.h"

class Browser;

namespace content {
class WebContents;
}  // namespace content

class BrowserObserver : public base::CheckedObserver {
 public:
  virtual void OnWebContentsAdded(content::WebContents*) {}
  virtual void OnWebContentsRemoved(content::WebContents*) {}
  virtual void OnWebContentsEmpty() {}
};

#endif  // RADIUM_BROWSER_UI_BROWSER_OBSERVER_H_
