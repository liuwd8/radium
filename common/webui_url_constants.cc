// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/webui_url_constants.h"

#include "third_party/blink/public/common/chrome_debug_urls.h"

base::span<const base::cstring_view> RadiumDebugURLs() {
  static constexpr auto kChromeDebugURLs = std::to_array<base::cstring_view>({
      blink::kChromeUICrashURL,
  });
  return base::span(kChromeDebugURLs);
}
