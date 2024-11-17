// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_TAB_TAB_ANDROID_H_
#define RADIUM_BROWSER_TAB_TAB_ANDROID_H_

#include <jni.h>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/supports_user_data.h"

class TabWebContentsDelegateAndroid;
class Profile;

namespace content {
class DevToolsAgentHost;
class WebContents;
}  // namespace content

class TabAndroid : public base::SupportsUserData {
 public:
  explicit TabAndroid(JNIEnv* env,
                      const base::android::JavaRef<jobject>& obj,
                      jint id);
  TabAndroid(const TabAndroid&) = delete;
  TabAndroid& operator=(const TabAndroid&) = delete;

  ~TabAndroid() override;

  // Convenience method to retrieve the Tab associated with the passed
  // WebContents.  Can return NULL.
  static TabAndroid* FromWebContents(const content::WebContents* web_contents);

  // Returns the native TabAndroid stored in the Java Tab represented by
  // |obj|.
  static TabAndroid* GetNativeTab(JNIEnv* env,
                                  const base::android::JavaRef<jobject>& obj);

  base::android::ScopedJavaLocalRef<jobject> GetJavaObject();

 private:
  JavaObjectWeakGlobalRef weak_java_tab_;
};

#endif  // RADIUM_BROWSER_TAB_TAB_ANDROID_H_
