// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/startup/startup_browser_creator.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "radium/browser/ui/signin/signin_window.h"
#include "radium/browser/ui/wechat/wechat_main_window_factory.h"

StartupBrowserCreator::StartupBrowserCreator() = default;
StartupBrowserCreator::~StartupBrowserCreator() = default;

bool StartupBrowserCreator::Start(const base::CommandLine& cmd_line,
                                  const base::FilePath& cur_dir,
                                  StartupProfileInfo profile_info,
                                  const Profiles& last_opened_profiles) {
  SigninWindow::Show(profile_info.profile, base::BindOnce(&ShowWechatMainView));
  return true;
}
