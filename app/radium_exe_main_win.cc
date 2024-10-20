// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#if !defined(WIN_CONSOLE_APP)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE prev, wchar_t*, int) {
#else   // !defined(WIN_CONSOLE_APP)
int main() {
  HINSTANCE instance = GetModuleHandle(nullptr);
#endif  // !defined(WIN_CONSOLE_APP)
}
