// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_APP_ANDROID_RADIUM_MAIN_DELEGATE_ANDROID_H_
#define RADIUM_APP_ANDROID_RADIUM_MAIN_DELEGATE_ANDROID_H_

#include "radium/app/radium_main_delegate.h"

namespace content {
class BrowserMainRunner;
}

class RadiumMainDelegateAndroid : public RadiumMainDelegate {
 public:
  static void SecureDataDirectory();  // visible for testing

  explicit RadiumMainDelegateAndroid();
  RadiumMainDelegateAndroid(const RadiumMainDelegateAndroid&) = delete;
  RadiumMainDelegateAndroid& operator=(const RadiumMainDelegateAndroid&) =
      delete;

  ~RadiumMainDelegateAndroid() override;

 private:
  void PreSandboxStartup() override;
  absl::variant<int, content::MainFunctionParams> RunProcess(
      const std::string& process_type,
      content::MainFunctionParams main_function_params) override;

 private:
  std::unique_ptr<content::BrowserMainRunner> browser_runner_;
};

#endif  // RADIUM_APP_ANDROID_RADIUM_MAIN_DELEGATE_ANDROID_H_
