// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_SIMPLE_MESSAGE_BOX_INTERNAL_H_
#define RADIUM_BROWSER_UI_SIMPLE_MESSAGE_BOX_INTERNAL_H_

namespace radium::internal {

// If true: skips prompting a blocking message box in tests in situations where
// such UI would freeze the test, but behaves as though such a box had come up
// and the user had clicked "OK". Defaults to false.
extern bool g_should_skip_message_box_for_test;

}  // namespace radium::internal

#endif  // RADIUM_BROWSER_UI_SIMPLE_MESSAGE_BOX_INTERNAL_H_
