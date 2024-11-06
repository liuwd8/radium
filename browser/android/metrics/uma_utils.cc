// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/android/metrics/uma_utils.h"

#include <stdint.h>

#include "components/metrics/metrics_reporting_default_state.h"
#include "third_party/jni_zero/jni_methods.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "radium/android/base_module_jni/UmaUtils_jni.h"

using jni_zero::JavaParamRef;

class PrefService;

namespace radium::android {

base::TimeTicks GetApplicationStartTime() {
  JNIEnv* env = jni_zero::AttachCurrentThread();
  return base::TimeTicks::FromUptimeMillis(
      Java_UmaUtils_getApplicationStartTime(env));
}

base::TimeTicks GetProcessStartTime() {
  JNIEnv* env = jni_zero::AttachCurrentThread();
  return base::TimeTicks::FromUptimeMillis(
      Java_UmaUtils_getProcessStartTime(env));
}

static jboolean JNI_UmaUtils_IsClientInSampleForMetrics(JNIEnv* env) {
  return false;
}

static jboolean JNI_UmaUtils_IsClientInSampleForCrashes(JNIEnv* env) {
  return false;
}

static void JNI_UmaUtils_RecordMetricsReportingDefaultOptIn(JNIEnv* env,
                                                            jboolean opt_in) {}

}  // namespace radium::android
