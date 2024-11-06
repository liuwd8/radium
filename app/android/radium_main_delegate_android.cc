// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/app/android/radium_main_delegate_android.h"

#include "base/base_paths_android.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/sampling_heap_profiler/poisson_allocation_sampler.h"
#include "base/trace_event/trace_event.h"
#include "components/startup_metric_utils/common/startup_metric_utils.h"
#include "content/public/browser/browser_main_runner.h"
#include "radium/app/radium_main_delegate.h"
#include "radium/browser/android/metrics/uma_utils.h"

void RadiumMainDelegateAndroid::SecureDataDirectory() {
  // By default, Android creates the directory accessible by others.
  // We'd like to tighten security and make it accessible only by
  // the browser process.
  // TODO(crbug.com/41382891): Remove this once minsdk >= 21,
  // at which point this will be handled by PathUtils.java.
  base::FilePath data_path;
  bool ok = base::PathService::Get(base::DIR_ANDROID_APP_DATA, &data_path);
  if (ok) {
    ok = base::SetPosixFilePermissions(data_path,
                                       base::FILE_PERMISSION_USER_MASK);
  }

  LOG_IF(ERROR, !ok) << "Failed to set data directory permissions";
}

RadiumMainDelegateAndroid::RadiumMainDelegateAndroid() = default;
RadiumMainDelegateAndroid::~RadiumMainDelegateAndroid() = default;

void RadiumMainDelegateAndroid::PreSandboxStartup() {
  RadiumMainDelegate::PreSandboxStartup();

  // PoissonAllocationSampler's TLS slots need to be set up before
  // MainThreadStackSamplingProfiler, which can allocate TLS slots of its own.
  // On some platforms pthreads can malloc internally to access higher-numbered
  // TLS slots, which can cause reentry in the heap profiler. (See the comment
  // on ReentryGuard::InitTLSSlot().)
  // TODO(crbug.com/40062835): Clean up other paths that call this Init()
  // function, which are now redundant.
  base::PoissonAllocationSampler::Init();
}

absl::variant<int, content::MainFunctionParams>
RadiumMainDelegateAndroid::RunProcess(
    const std::string& process_type,
    content::MainFunctionParams main_function_params) {
  TRACE_EVENT0("startup", "RadiumMainDelegateAndroid::RunProcess");
  // Defer to the default main method outside the browser process.
  if (!process_type.empty()) {
    return std::move(main_function_params);
  }

  SecureDataDirectory();

  // Because the browser process can be started asynchronously as a series of
  // UI thread tasks a second request to start it can come in while the
  // first request is still being processed. Chrome must keep the same
  // browser runner for the second request.
  // Also only record the start time the first time round, since this is the
  // start time of the application, and will be same for all requests.
  if (!browser_runner_) {
    base::TimeTicks process_start_time = radium::android::GetProcessStartTime();
    base::TimeTicks application_start_time =
        radium::android::GetApplicationStartTime();
    if (!process_start_time.is_null()) {
      startup_metric_utils::GetCommon().RecordStartupProcessCreationTime(
          process_start_time);
    }
    startup_metric_utils::GetCommon().RecordApplicationStartTime(
        application_start_time);
    browser_runner_ = content::BrowserMainRunner::Create();
  }

  int exit_code = browser_runner_->Initialize(std::move(main_function_params));
  // On Android we do not run BrowserMain(), so the above initialization of a
  // BrowserMainRunner is all we want to occur. Preserve any error codes > 0.
  if (exit_code > 0) {
    return exit_code;
  }
  return 0;
}
