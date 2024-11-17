// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/tab/tab_web_contents_delegate_android.h"

#include <stddef.h>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/metrics/histogram_macros.h"
#include "base/rand_util.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "radium/browser/tab/android/jni_headers/TabWebContentsDelegateAndroidImpl_jni.h"

using base::android::AttachCurrentThread;
using base::android::JavaParamRef;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;
using blink::mojom::FileChooserParams;
using content::WebContents;

TabWebContentsDelegateAndroid::TabWebContentsDelegateAndroid(
    JNIEnv* env,
    const jni_zero::JavaRef<jobject>& obj)
    : WebContentsDelegateAndroid(env, obj) {}

TabWebContentsDelegateAndroid::~TabWebContentsDelegateAndroid() = default;

bool TabWebContentsDelegateAndroid::IsNightModeEnabled() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null()) {
    return false;
  }
  return Java_TabWebContentsDelegateAndroidImpl_isNightModeEnabled(env, obj);
}

bool TabWebContentsDelegateAndroid::IsForceDarkWebContentEnabled() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null()) {
    return false;
  }
  return Java_TabWebContentsDelegateAndroidImpl_isForceDarkWebContentEnabled(
      env, obj);
}
