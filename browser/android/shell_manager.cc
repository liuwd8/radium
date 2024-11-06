// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/android/shell_manager.h"

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/no_destructor.h"
#include "radium/browser/android/shell.h"
#include "radium/browser/profiles/profile_manager.h"
#include "url/gurl.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "radium/android/radium_jni_headers/ShellManager_jni.h"

using base::android::JavaParamRef;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;

namespace radium {

namespace {
struct GlobalState {
  GlobalState() {}
  base::android::ScopedJavaGlobalRef<jobject> j_shell_manager;
};

GlobalState* GetGlobalState() {
  static base::NoDestructor<GlobalState> g_global_state;
  return g_global_state.get();
}
}  // namespace

ScopedJavaLocalRef<jobject> CreateShellView(Shell* shell) {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_ShellManager_createShell(env, GetGlobalState()->j_shell_manager,
                                       reinterpret_cast<intptr_t>(shell));
}

void RemoveShellView(const JavaRef<jobject>& shell_view) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ShellManager_removeShell(env, GetGlobalState()->j_shell_manager,
                                shell_view);
}

void JNI_ShellManager_Init(JNIEnv* env,
                           const base::android::JavaParamRef<jobject>& obj) {
  GetGlobalState()->j_shell_manager.Reset(obj);
}

void JNI_ShellManager_LaunchShell(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& jurl) {
  Profile* profile = ProfileManager::GetLastUsedProfile();
  GURL url(base::android::ConvertJavaStringToUTF8(env, jurl));
  Shell::CreateNewWindow(profile, url, nullptr, gfx::Size());
}

void DestroyShellManager() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ShellManager_destroy(env, GetGlobalState()->j_shell_manager);
}

}  // namespace radium
