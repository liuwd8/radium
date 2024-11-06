// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts_android.h"

#include "base/task/thread_pool.h"
#include "base/trace_event/trace_event.h"
#include "components/crash/content/browser/child_exit_observer_android.h"
#include "components/crash/content/browser/child_process_crash_observer_android.h"
#include "content/public/browser/android/compositor.h"
#include "radium/browser/android/mojo/radium_interface_registrar_android.h"

RadiumBrowserMainPartsAndroid::RadiumBrowserMainPartsAndroid(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : RadiumBrowserMainParts(radium_feature_list_creator) {}

RadiumBrowserMainPartsAndroid::~RadiumBrowserMainPartsAndroid() = default;

int RadiumBrowserMainPartsAndroid::PreCreateThreads() {
  TRACE_EVENT0("startup", "RadiumBrowserMainPartsAndroid::PreCreateThreads");

  int result_code = RadiumBrowserMainParts::PreCreateThreads();

  // The ChildExitObserver needs to be created before any child process is
  // created because it needs to be notified during process creation.
  child_exit_observer_ = std::make_unique<crash_reporter::ChildExitObserver>();
  child_exit_observer_->RegisterClient(
      std::make_unique<crash_reporter::ChildProcessCrashObserver>());

  return result_code;
}

void RadiumBrowserMainPartsAndroid::PostProfileInit(Profile* profile,
                                                    bool is_initial_profile) {
  DCHECK(is_initial_profile);  // No multiprofile on Android, only the initial
                               // call should happen.

  RadiumBrowserMainParts::PostProfileInit(profile, is_initial_profile);
}

int RadiumBrowserMainPartsAndroid::PreEarlyInitialization() {
  TRACE_EVENT0("startup",
               "ChromeBrowserMainPartsAndroid::PreEarlyInitialization");
  content::Compositor::Initialize();

  CHECK(base::CurrentThread::IsSet());

  return RadiumBrowserMainParts::PreEarlyInitialization();
}

void RadiumBrowserMainPartsAndroid::PostBrowserStart() {
  RadiumBrowserMainParts::PostBrowserStart();

  RegisterRadiumJavaMojoInterfaces();
}
