// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_ANDROID_H_
#define RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_ANDROID_H_

namespace content {
class PosixFileDescriptorInfo;
}  // namespace content

void GetMappedLocalePacksForChildProcess(
    content::PosixFileDescriptorInfo* mappings);

#endif  // RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_ANDROID_H_
