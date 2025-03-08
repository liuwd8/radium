// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROCESS_SINGLETON_INTERNAL_H_
#define RADIUM_BROWSER_PROCESS_SINGLETON_INTERNAL_H_

#include "radium/browser/process_singleton.h"

namespace internal {

void SendRemoteProcessInteractionResultHistogram(
    ProcessSingleton::RemoteProcessInteractionResult result);

void SendRemoteHungProcessTerminateReasonHistogram(
    ProcessSingleton::RemoteHungProcessTerminateReason reason);

}  // namespace internal

#endif  // RADIUM_BROWSER_PROCESS_SINGLETON_INTERNAL_H_
