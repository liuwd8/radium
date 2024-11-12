// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_H_
#define RADIUM_BROWSER_UI_BROWSER_H_

#include "content/public/browser/web_contents_delegate.h"

class Browser {
 public:
  explicit Browser();
  Browser(const Browser&) = delete;
  Browser& operator=(const Browser&) = delete;

  ~Browser();

 private:
};

#endif  // RADIUM_BROWSER_UI_BROWSER_H_
