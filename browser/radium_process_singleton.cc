// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_process_singleton.h"

#include <utility>

#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>

#include "base/metrics/histogram_functions.h"
#include "components/app_launch_prefetch/app_launch_prefetch.h"
#include "radium/common/radium_features.h"
#endif

namespace {

RadiumProcessSingleton* g_radium_process_singleton_ = nullptr;

}  // namespace

RadiumProcessSingleton::RadiumProcessSingleton(
    const base::FilePath& user_data_dir)
    : startup_lock_(
          base::BindRepeating(&RadiumProcessSingleton::NotificationCallback,
                              base::Unretained(this))),
      process_singleton_(user_data_dir,
                         startup_lock_.AsNotificationCallback()) {}

RadiumProcessSingleton::~RadiumProcessSingleton() = default;

ProcessSingleton::NotifyResult
RadiumProcessSingleton::NotifyOtherProcessOrCreate() {
  CHECK(!is_singleton_instance_);
  ProcessSingleton::NotifyResult result =
      process_singleton_.NotifyOtherProcessOrCreate();
  if (result == ProcessSingleton::PROCESS_NONE) {
    is_singleton_instance_ = true;
  }
  return result;
}

void RadiumProcessSingleton::StartWatching() {
  process_singleton_.StartWatching();
}

void RadiumProcessSingleton::Cleanup() {
  if (is_singleton_instance_) {
    process_singleton_.Cleanup();
  }
}

void RadiumProcessSingleton::Unlock(
    const ProcessSingleton::NotificationCallback& notification_callback) {
  notification_callback_ = notification_callback;
  startup_lock_.Unlock();
}

// static
void RadiumProcessSingleton::CreateInstance(
    const base::FilePath& user_data_dir) {
  DCHECK(!g_radium_process_singleton_);
  DCHECK(!user_data_dir.empty());
  g_radium_process_singleton_ = new RadiumProcessSingleton(user_data_dir);
}

// static
void RadiumProcessSingleton::DeleteInstance() {
  if (g_radium_process_singleton_) {
    delete g_radium_process_singleton_;
    g_radium_process_singleton_ = nullptr;
  }
}

// static
RadiumProcessSingleton* RadiumProcessSingleton::GetInstance() {
  CHECK(g_radium_process_singleton_);
  return g_radium_process_singleton_;
}

// static
bool RadiumProcessSingleton::IsSingletonInstance() {
  return g_radium_process_singleton_ &&
         g_radium_process_singleton_->is_singleton_instance_;
}

bool RadiumProcessSingleton::NotificationCallback(
    base::CommandLine command_line,
    const base::FilePath& current_directory) {
  DCHECK(notification_callback_);
  return notification_callback_.Run(std::move(command_line), current_directory);
}
