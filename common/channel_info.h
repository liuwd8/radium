// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_CHANNEL_INFO_H_
#define RADIUM_COMMON_CHANNEL_INFO_H_

namespace version_info {
enum class Channel;
}

namespace radium {

// Returns the channel for the installation. In branded builds, this will be
// version_info::Channel::{STABLE,BETA,DEV,CANARY}. In unbranded builds, or
// in branded builds when the channel cannot be determined, this will be
// version_info::Channel::UNKNOWN.
version_info::Channel GetChannel();

}  // namespace radium

#endif  // RADIUM_COMMON_CHANNEL_INFO_H_
