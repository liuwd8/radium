// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_COCOA_SIMPLE_MESSAGE_BOX_COCOA_H_
#define RADIUM_BROWSER_UI_COCOA_SIMPLE_MESSAGE_BOX_COCOA_H_


#include <string_view>

#include "radium/browser/ui/simple_message_box.h"

namespace radium {

MessageBoxResult ShowMessageBoxCocoa(std::u16string_view message,
                                     MessageBoxType type,
                                     std::u16string_view checkbox_text);

}  // namespace radium


#endif  // RADIUM_BROWSER_UI_COCOA_SIMPLE_MESSAGE_BOX_COCOA_H_
