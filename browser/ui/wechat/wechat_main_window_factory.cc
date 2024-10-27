// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/wechat/wechat_main_window_factory.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/hash/md5.h"
#include "base/location.h"
#include "base/task/task_traits.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/global_features.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/profiles/profile_manager.h"
#include "radium/browser/ui/views/wechat/wechat_main_view.h"

void ShowWechatMainView() {
  ProfileManager* profile_manager =
      BrowserProcess::Get()->GetFeatures()->profile_manager();
  base::FilePath path =
      profile_manager->user_data_dir().AppendASCII(base::MD5String("W"));
  Profile* profile = profile_manager->GetProfile(path);
  if (profile) {
    (new WechatMainView())->Show(profile);
    return;
  }

  profile_manager->CreateProfileAsync(
      path, base::BindOnce(&WechatMainView::Show,
                           base::Unretained(new WechatMainView())));
}
