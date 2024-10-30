// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_WEBUI_URL_CONSTANTS_H_
#define RADIUM_COMMON_WEBUI_URL_CONSTANTS_H_

#include <string_view>

#include "base/containers/span.h"
#include "base/strings/cstring_view.h"

namespace radium {

inline constexpr char kRadiumUIScheme[] = "weixin";

// Gets the URL strings of "debug" pages which are dangerous and not for general
// consumption.
base::span<const base::cstring_view> RadiumDebugURLs();

}  // namespace radium

#endif  // RADIUM_COMMON_WEBUI_URL_CONSTANTS_H_
