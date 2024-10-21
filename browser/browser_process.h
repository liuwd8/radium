// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_BROWSER_PROCESS_H_
#define RADIUM_BROWSER_BROWSER_PROCESS_H_

#include "base/callback_list.h"
#include "base/functional/callback.h"
#include "base/functional/callback_forward.h"
#include "base/sequence_checker.h"
#include "components/keep_alive_registry/keep_alive_state_observer.h"

class GlobalFeatures;
class GpuModeManager;
class PrefRegistrySimple;
class PrefService;
class RadiumFeatureListCreator;
class SystemNetworkContextManager;

namespace os_crypt_async {
class KeyProvider;
class OSCryptAsync;
}  // namespace os_crypt_async

namespace policy {
class RadiumBrowserPolicyConnector;
class PolicyService;
}  // namespace policy

class BrowserProcess : public KeepAliveStateObserver {
 public:
  static BrowserProcess* Get();
  static void RegisterPrefs(PrefRegistrySimple* registry);

  explicit BrowserProcess(
      RadiumFeatureListCreator* radium_feature_list_creator);
  BrowserProcess(const BrowserProcess&) = delete;
  BrowserProcess& operator=(const BrowserProcess&) = delete;

  ~BrowserProcess() override;

  // Called to complete initialization.
  void Init();

#if !BUILDFLAG(IS_ANDROID)
  // Sets a closure to be run to break out of a run loop on browser shutdown
  // (when the KeepAlive count reaches zero).
  void SetQuitClosure(base::OnceClosure quit_closure);
#endif

  // Called before the browser threads are created.
  void PreCreateThreads();

  // Called after the threads have been created but before the message loops
  // starts running. Allows the browser process to do any initialization that
  // requires all threads running.
  void PreMainMessageLoopRun();

  // Most cleanup is done by these functions, driven from
  // ChromeBrowserMain based on notifications from the content
  // framework, rather than in the destructor, so that we can
  // interleave cleanup with threads being stopped.
#if !BUILDFLAG(IS_ANDROID)
  void StartTearDown();
  void PostDestroyThreads();
#endif

  policy::RadiumBrowserPolicyConnector* browser_policy_connector();
  PrefService* local_state();
  GpuModeManager* gpu_mode_manager();
  os_crypt_async::OSCryptAsync* os_crypt_async();
  policy::PolicyService* policy_service();
  SystemNetworkContextManager* system_network_context_manager();

  GlobalFeatures* GetFeatures();

 private:
  // KeepAliveStateObserver:
  void OnKeepAliveStateChanged(bool is_keeping_alive) override;
  void OnKeepAliveRestartStateChanged(bool can_restart) override;

  // Methods called to control our lifetime. The browser process can be "pinned"
  // to make sure it keeps running.
  void Pin();
  void Unpin();

  // Must be destroyed after |local_state_|.
  // Must be destroyed after |profile_manager_|.
  std::unique_ptr<policy::RadiumBrowserPolicyConnector> const
      browser_policy_connector_;

  const std::unique_ptr<PrefService> local_state_;

#if !BUILDFLAG(IS_ANDROID)
  // Called to signal the process' main message loop to exit.
  base::OnceClosure quit_closure_;
#endif

  std::unique_ptr<GlobalFeatures> features_;

  std::unique_ptr<os_crypt_async::OSCryptAsync> os_crypt_async_;
  std::optional<base::CallbackListSubscription>
      os_crypt_async_init_subscription_;

  SEQUENCE_CHECKER(sequence_checker_);
};

#endif  // RADIUM_BROWSER_BROWSER_PROCESS_H_
