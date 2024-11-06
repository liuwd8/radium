// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_ANDROID_METRICS_UMA_UTILS_H_
#define RADIUM_BROWSER_ANDROID_METRICS_UMA_UTILS_H_

#include <jni.h>

#include "base/time/time.h"

namespace radium::android {

base::TimeTicks GetApplicationStartTime();
base::TimeTicks GetProcessStartTime();

}  // namespace radium::android

#endif  // RADIUM_BROWSER_ANDROID_METRICS_UMA_UTILS_H_
