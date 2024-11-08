// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/webui/radium_webui_config.h"

RadiumWebUIConfig::RadiumWebUIConfig(std::string_view scheme,
                                     std::string_view host)
    : WebUIConfig(scheme, host) {}

RadiumWebUIConfig::~RadiumWebUIConfig() = default;
