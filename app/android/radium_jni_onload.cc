// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/app/android/radium_jni_onload.h"

#include "content/public/app/content_jni_onload.h"
#include "content/public/app/content_main.h"
#include "radium/app/android/radium_main_delegate_android.h"

namespace android {

bool OnJNIOnLoadInit() {
  if (!content::android::OnJNIOnLoadInit()) {
    return false;
  }

  content::SetContentMainDelegate(new RadiumMainDelegateAndroid());
  return true;
}

}  // namespace android
