// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/lifetime/application_lifetime_android.h"

#include <jni.h>

#include "base/android/jni_android.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "radium/browser/lifetime/android/jni_headers/ApplicationLifetime_jni.h"

namespace radium {

void TerminateAndroid() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ApplicationLifetime_terminate(env, false);
}

}  // namespace radium
