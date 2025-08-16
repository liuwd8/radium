// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/browser_finder.h"

#include <stdint.h>

#include <algorithm>

#include "radium/browser/ui/browser_list.h"

namespace radium {

size_t GetTotalBrowserCount() {
  return BrowserList::GetInstance()->size();
}

}  // namespace radium
