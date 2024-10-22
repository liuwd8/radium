// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts_linux.h"

#include "base/command_line.h"
#include "base/linux_util.h"
#include "base/task/thread_pool.h"
#include "components/os_crypt/sync/key_storage_config_linux.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "components/password_manager/core/browser/password_manager_switches.h"
#include "device/bluetooth/dbus/bluez_dbus_manager.h"
#include "device/bluetooth/dbus/bluez_dbus_thread_manager.h"
#include "radium/browser/radium_browser_main_parts_posix.h"
#include "radium/common/radium_paths_internal.h"
#include "radium/grit/radium_strings.h"
#include "ui/base/l10n/l10n_util.h"

#if BUILDFLAG(IS_LINUX)
#include "ui/ozone/public/ozone_platform.h"
#endif

RadiumBrowserMainPartsLinux::RadiumBrowserMainPartsLinux(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : RadiumBrowserMainPartsPosix(radium_feature_list_creator) {}

RadiumBrowserMainPartsLinux::~RadiumBrowserMainPartsLinux() = default;

void RadiumBrowserMainPartsLinux::PostCreateMainMessageLoop() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
#if BUILDFLAG(IS_CHROMEOS)
  if (command_line->HasSwitch(
          chromeos::tast_support::kRecordStackSamplingDataSwitch)) {
    stack_sampling_recorder_ =
        base::MakeRefCounted<chromeos::tast_support::StackSamplingRecorder>();
    stack_sampling_recorder_->Start();
  }
  // Don't initialize DBus here. Ash and Lacros Bluetooth DBusManager
  // initialization depend on FeatureList, and is done elsewhere.
#endif  // BUILDFLAG(IS_CHROMEOS)

#if !BUILDFLAG(IS_CHROMEOS)
  bluez::BluezDBusManager::Initialize(nullptr /* system_bus */);

  // Set up crypt config. This needs to be done before anything starts the
  // network service, as the raw encryption key needs to be shared with the
  // network service for encrypted cookie storage.
  // Chrome OS does not need a crypt config as its user data directories are
  // already encrypted and none of the true encryption backends used by desktop
  // Linux are available on Chrome OS anyway.
  std::unique_ptr<os_crypt::Config> config =
      std::make_unique<os_crypt::Config>();
  // Forward to os_crypt the flag to use a specific password store.
  config->store =
      command_line->GetSwitchValueASCII(password_manager::kPasswordStore);
  // Forward the product name
  config->product_name = l10n_util::GetStringUTF8(IDS_PRODUCT_NAME);
  // OSCrypt can be disabled in a special settings file.
  config->should_use_preference =
      base::CommandLine::ForCurrentProcess()->HasSwitch(
          password_manager::kEnableEncryptionSelection);
  radium::GetDefaultUserDataDirectory(&config->user_data_path);
  OSCrypt::SetConfig(std::move(config));
#endif  // !BUILDFLAG(IS_CHROMEOS)

  RadiumBrowserMainPartsPosix::PostCreateMainMessageLoop();
}

void RadiumBrowserMainPartsLinux::PreProfileInit() {
#if !BUILDFLAG(IS_CHROMEOS_ASH)
  // Needs to be called after we have chrome::DIR_USER_DATA and
  // g_browser_process.  This happens in PreCreateThreads.
  // base::GetLinuxDistro() will initialize its value if needed.
  base::ThreadPool::PostTask(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::BEST_EFFORT},
      base::BindOnce(base::IgnoreResult(&base::GetLinuxDistro)));
#endif

  RadiumBrowserMainPartsPosix::PreProfileInit();
}

#if BUILDFLAG(IS_LINUX)
void RadiumBrowserMainPartsLinux::PostMainMessageLoopRun() {
  RadiumBrowserMainPartsPosix::PostMainMessageLoopRun();
  ui::OzonePlatform::GetInstance()->PostMainMessageLoopRun();
}
#endif

void RadiumBrowserMainPartsLinux::PostDestroyThreads() {
#if BUILDFLAG(IS_CHROMEOS)
  // No-op; per PostBrowserStart() comment, this is done elsewhere.
#else
  bluez::BluezDBusManager::Shutdown();
  bluez::BluezDBusThreadManager::Shutdown();
#endif  // BUILDFLAG(IS_CHROMEOS)

  RadiumBrowserMainPartsPosix::PostDestroyThreads();
}
