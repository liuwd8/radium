// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/channel_info.h"

#include "base/version_info/channel.h"

namespace radium {

version_info::Channel GetChannel() {
  return version_info::Channel::UNKNOWN;
}

}  // namespace radium
