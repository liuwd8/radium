// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_ANDROID_SHELL_MANAGER_H_
#define RADIUM_BROWSER_ANDROID_SHELL_MANAGER_H_
#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

class Shell;

namespace radium {

// Creates an Android specific shell view, which is our version of a shell
// window.  This view holds the controls and content views necessary to
// render a shell window.  Returns the java object representing the shell view.
// object.
base::android::ScopedJavaLocalRef<jobject> CreateShellView(Shell* shell);

// Removes a previously created shell view.
void RemoveShellView(const base::android::JavaRef<jobject>& shell_view);

// Destroys the ShellManager on app exit. Must not use the above functions
// after this is called.
void DestroyShellManager();

}  // namespace radium

#endif  // RADIUM_BROWSER_ANDROID_SHELL_MANAGER_H_
