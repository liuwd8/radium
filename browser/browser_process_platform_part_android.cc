// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_process_platform_part_android.h"

#include "base/android/memory_pressure_listener_android.h"
#include "radium/browser/lifetime/application_lifetime_android.h"

BrowserProcessPlatformPart::BrowserProcessPlatformPart() {
  base::android::MemoryPressureListenerAndroid::Initialize(
      base::android::AttachCurrentThread());
}

BrowserProcessPlatformPart::~BrowserProcessPlatformPart() = default;

void BrowserProcessPlatformPart::AttemptExit(bool try_to_quit_application) {
  // Tell the Java code to finish() the Activity.
  radium::TerminateAndroid();
}
