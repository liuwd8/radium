// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_APP_DELAY_LOAD_FAILURE_HOOK_WIN_H_
#define RADIUM_APP_DELAY_LOAD_FAILURE_HOOK_WIN_H_

namespace radium {

// This should be called early in process startup (before any delay load
// failures) to disable the delay load hooks for the main EXE file.
void DisableDelayLoadFailureHooksForMainExecutable();

}  // namespace radium

#endif  // RADIUM_APP_DELAY_LOAD_FAILURE_HOOK_WIN_H_
