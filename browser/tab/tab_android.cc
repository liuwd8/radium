// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/tab/tab_android.h"

#include "base/trace_event/trace_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "radium/browser/tab/android/jni_headers/TabImpl_jni.h"

using base::android::AttachCurrentThread;
using base::android::JavaParamRef;
using base::android::JavaRef;

namespace {

class TabAndroidHelper : public content::WebContentsUserData<TabAndroidHelper> {
 public:
  static void SetTabForWebContents(content::WebContents* contents,
                                   TabAndroid* tab_android) {
    content::WebContentsUserData<TabAndroidHelper>::CreateForWebContents(
        contents);
    content::WebContentsUserData<TabAndroidHelper>::FromWebContents(contents)
        ->tab_android_ = tab_android;
  }

  static TabAndroid* FromWebContents(const content::WebContents* contents) {
    TabAndroidHelper* helper =
        static_cast<TabAndroidHelper*>(contents->GetUserData(UserDataKey()));
    return helper ? helper->tab_android_.get() : nullptr;
  }

  explicit TabAndroidHelper(content::WebContents* web_contents)
      : content::WebContentsUserData<TabAndroidHelper>(*web_contents) {}

 private:
  friend class content::WebContentsUserData<TabAndroidHelper>;

  raw_ptr<TabAndroid> tab_android_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

WEB_CONTENTS_USER_DATA_KEY_IMPL(TabAndroidHelper);

}  // namespace

TabAndroid* TabAndroid::FromWebContents(
    const content::WebContents* web_contents) {
  return TabAndroidHelper::FromWebContents(web_contents);
}

TabAndroid* TabAndroid::GetNativeTab(
    JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  return reinterpret_cast<TabAndroid*>(Java_TabImpl_getNativePtr(env, obj));
}

TabAndroid::TabAndroid(JNIEnv* env,
                       const base::android::JavaRef<jobject>& obj,
                       jint id)
    : weak_java_tab_(env, obj) {
  Java_TabImpl_setNativePtr(env, obj, reinterpret_cast<intptr_t>(this));
}

TabAndroid::~TabAndroid() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_TabImpl_clearNativePtr(env, weak_java_tab_.get(env));
}

base::android::ScopedJavaLocalRef<jobject> TabAndroid::GetJavaObject() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return weak_java_tab_.get(env);
}

base::android::ScopedJavaLocalRef<jobject> JNI_TabImpl_FromWebContents(
    JNIEnv* env,
    const JavaParamRef<jobject>& jweb_contents) {
  base::android::ScopedJavaLocalRef<jobject> jtab;

  content::WebContents* web_contents =
      content::WebContents::FromJavaWebContents(jweb_contents);
  TabAndroid* tab =
      web_contents ? TabAndroid::FromWebContents(web_contents) : nullptr;
  if (tab) {
    jtab = tab->GetJavaObject();
  }
  return jtab;
}

static void JNI_TabImpl_Init(JNIEnv* env,
                             const JavaParamRef<jobject>& obj,
                             jint id) {
  TRACE_EVENT0("native", "TabAndroid::Init");
  // This will automatically bind to the Java object and pass ownership there.
  new TabAndroid(env, obj, id);
}
