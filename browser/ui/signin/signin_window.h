// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_SIGNIN_SIGNIN_WINDOW_H_
#define RADIUM_BROWSER_UI_SIGNIN_SIGNIN_WINDOW_H_

class Profile;

class SigninWindow {
 public:
  explicit SigninWindow();
  SigninWindow(const SigninWindow&) = delete;
  SigninWindow& operator=(const SigninWindow&) = delete;

  ~SigninWindow();

  static void Show(Profile* profile);
};

#endif  // RADIUM_BROWSER_UI_SIGNIN_SIGNIN_WINDOW_H_
