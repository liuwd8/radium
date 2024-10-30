// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_CHILD_V8_CRASHPAD_SUPPORT_WIN_H_
#define RADIUM_CHILD_V8_CRASHPAD_SUPPORT_WIN_H_

namespace v8_crashpad_support {

// Hook up V8 to crashpad.
void SetUp();

}  // namespace v8_crashpad_support

#endif  // RADIUM_CHILD_V8_CRASHPAD_SUPPORT_WIN_H_
