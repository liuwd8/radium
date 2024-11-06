// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/android/mojo/radium_interface_registrar_android.h"

#include <jni.h>

#include "base/android/jni_android.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "radium/android/radium_jni_headers/RadiumInterfaceRegistrar_jni.h"

void RegisterRadiumJavaMojoInterfaces() {
  Java_RadiumInterfaceRegistrar_registerMojoInterfaces(
      base::android::AttachCurrentThread());
}
