// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_WEBUI_URL_CONSTANTS_H_
#define RADIUM_COMMON_WEBUI_URL_CONSTANTS_H_

#include <string_view>

#include "base/containers/span.h"
#include "base/strings/cstring_view.h"

namespace radium {

inline constexpr char kRadiumUIScheme[] = "radium";

inline constexpr char kRadiumUIExampleHost[] = "example";
inline constexpr char kRadiumUIExampleURL[] = "radium://example";
inline constexpr char kRadiumUIFavicon2Host[] = "favicon2";
inline constexpr char kRadiumUIFaviconHost[] = "favicon";
inline constexpr char kRadiumUIResourceHost[] = "resources";
inline constexpr char kRadiumUIResourceURL[] = "radium://resources";
inline constexpr char kRadiumUIWebuiGalleryHost[] = "webui-gallery";
inline constexpr char kRadiumUIWebuiGalleryURL[] = "radium://webui-gallery";

// Gets the URL strings of "debug" pages which are dangerous and not for general
// consumption.
base::span<const base::cstring_view> RadiumDebugURLs();

}  // namespace radium

#endif  // RADIUM_COMMON_WEBUI_URL_CONSTANTS_H_
