// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_TAB_TAB_WEB_CONTENTS_DELEGATE_ANDROID_H_
#define RADIUM_BROWSER_TAB_TAB_WEB_CONTENTS_DELEGATE_ANDROID_H_

#include <memory>

#include "components/embedder_support/android/delegate/web_contents_delegate_android.h"

namespace content {
class WebContents;
}  // namespace content

// Chromium Android specific WebContentsDelegate.
// Should contain any WebContentsDelegate implementations required by
// the Chromium Android port but not to be shared with WebView.
class TabWebContentsDelegateAndroid
    : public web_contents_delegate_android::WebContentsDelegateAndroid {
 public:
  TabWebContentsDelegateAndroid(JNIEnv* env,
                                const jni_zero::JavaRef<jobject>& obj);

  TabWebContentsDelegateAndroid(const TabWebContentsDelegateAndroid&) = delete;
  TabWebContentsDelegateAndroid& operator=(
      const TabWebContentsDelegateAndroid&) = delete;

  ~TabWebContentsDelegateAndroid() override;

  bool IsNightModeEnabled() const;
  bool IsForceDarkWebContentEnabled() const;
};

#endif  // RADIUM_BROWSER_TAB_TAB_WEB_CONTENTS_DELEGATE_ANDROID_H_
