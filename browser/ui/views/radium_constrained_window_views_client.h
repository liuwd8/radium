// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_CONSTRAINED_WINDOW_VIEWS_CLIENT_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_CONSTRAINED_WINDOW_VIEWS_CLIENT_H_

#include <memory>

#include "components/constrained_window/constrained_window_views_client.h"

// Creates a ConstrainedWindowViewsClient for the Chrome environment.
std::unique_ptr<constrained_window::ConstrainedWindowViewsClient>
CreateRadiumConstrainedWindowViewsClient();

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_CONSTRAINED_WINDOW_VIEWS_CLIENT_H_
