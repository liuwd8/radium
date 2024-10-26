// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_process.h"

#include <memory>

#include "base/run_loop.h"
#include "base/sequence_checker.h"
#include "base/trace_event/trace_event.h"
#include "components/keep_alive_registry/keep_alive_registry.h"
#include "components/language/core/browser/pref_names.h"
#include "components/os_crypt/async/browser/key_provider.h"
#include "components/os_crypt/async/browser/os_crypt_async.h"
#include "components/os_crypt/async/browser/secret_portal_key_provider.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/devtools/remote_debugging_server.h"
#include "radium/browser/global_features.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/policy/radium_browser_policy_connector.h"
#include "radium/common/pref_names.h"

namespace {
BrowserProcess* g_browser_process = nullptr;

#if BUILDFLAG(IS_LINUX)
// Enables usage of os_crypt_async::SecretPortalKeyProvider.  Once
// `kSecretPortalKeyProviderUseForEncryption` is enabled, this flag cannot be
// disabled without losing data.
BASE_FEATURE(kDbusSecretPortal,
             "DbusSecretPortal",
             base::FEATURE_ENABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_LINUX)

#if BUILDFLAG(IS_LINUX)
// If true, encrypt new data with the key provided by SecretPortalKeyProvider.
// Otherwise, it will only decrypt existing data.
BASE_FEATURE(kSecretPortalKeyProviderUseForEncryption,
             "SecretPortalKeyProviderUseForEncryption",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_LINUX)

}  // namespace

BrowserProcess* BrowserProcess::Get() {
  return g_browser_process;
}

void BrowserProcess::RegisterPrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(prefs::kDefaultBrowserSettingEnabled, false);

  registry->RegisterBooleanPref(prefs::kAllowCrossOriginAuthPrompt, false);

#if BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_ANDROID)
  registry->RegisterBooleanPref(prefs::kEulaAccepted, false);
#endif  // BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_ANDROID)

  registry->RegisterStringPref(language::prefs::kApplicationLocale,
                               std::string());
#if BUILDFLAG(IS_CHROMEOS_ASH)
  registry->RegisterStringPref(prefs::kOwnerLocale, std::string());
  registry->RegisterStringPref(prefs::kHardwareKeyboardLayout, std::string());
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

  // registry->RegisterBooleanPref(prefs::kDevToolsRemoteDebuggingAllowed,
  // true);

#if BUILDFLAG(IS_LINUX)
  os_crypt_async::SecretPortalKeyProvider::RegisterLocalPrefs(registry);
#endif
}

BrowserProcess::BrowserProcess(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : browser_policy_connector_(
          radium_feature_list_creator->TakeRadiumBrowserPolicyConnector()),
      local_state_(radium_feature_list_creator->TakePrefService()) {
  g_browser_process = this;
}

BrowserProcess::~BrowserProcess() {
#if !BUILDFLAG(IS_ANDROID)
  KeepAliveRegistry::GetInstance()->RemoveObserver(this);
#endif

  g_browser_process = nullptr;
}

void BrowserProcess::Init() {
#if !BUILDFLAG(IS_ANDROID)
  KeepAliveRegistry::GetInstance()->SetIsShuttingDown(false);
  KeepAliveRegistry::GetInstance()->AddObserver(this);
#endif  // !BUILDFLAG(IS_ANDROID)

  features_ = std::make_unique<GlobalFeatures>();
  features_->Init();
}

#if !BUILDFLAG(IS_ANDROID)
void BrowserProcess::SetQuitClosure(base::OnceClosure quit_closure) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(quit_closure);
  DCHECK(!quit_closure_);
  quit_closure_ = std::move(quit_closure);
}
#endif

void BrowserProcess::PreCreateThreads() {
  // Create SystemNetworkContextManager without a NetworkService if it has not
  // been requested yet.
  if (!SystemNetworkContextManager::HasInstance()) {
    SystemNetworkContextManager::CreateInstance(local_state());
  }
}

void BrowserProcess::CreateDevToolsProtocolHandler() {
  // StartupBrowserCreator::LaunchBrowser can be run multiple times when browser
  // is started with several profiles or existing browser process is reused.
  if (!remote_debugging_server_) {
    remote_debugging_server_ = std::make_unique<RemoteDebuggingServer>();
  }
}

void BrowserProcess::PreMainMessageLoopRun() {
  // OSCryptAsync provider configuration. If empty, this delegates all
  // encryption operations to OSCrypt.
  std::vector<std::pair<size_t, std::unique_ptr<os_crypt_async::KeyProvider>>>
      providers;

#if BUILDFLAG(IS_WIN)
  // The DPAPI key provider requires OSCrypt::Init to have already been called
  // to initialize the key storage. This happens in
  // ChromeBrowserMainPartsWin::PreCreateMainMessageLoop.
  providers.emplace_back(std::make_pair(
      /*precedence=*/10u,
      std::make_unique<os_crypt_async::DPAPIKeyProvider>(local_state())));

  providers.emplace_back(std::make_pair(
      // Note: 15 is chosen to be higher than the 10 precedence above for
      // DPAPI. This ensures that when the the provider is enabled for
      // encryption, the App-Bound encryption key is used and not the DPAPI
      // one.
      /*precedence=*/15u,
      std::make_unique<os_crypt_async::AppBoundEncryptionProviderWin>(
          local_state(),
          base::FeatureList::IsEnabled(
              features::kUseAppBoundEncryptionProviderForEncryption))));
#endif  // BUILDFLAG(IS_WIN)

#if BUILDFLAG(IS_LINUX)
  if (base::FeatureList::IsEnabled(kDbusSecretPortal)) {
    providers.emplace_back(
        /*precedence=*/10u,
        std::make_unique<os_crypt_async::SecretPortalKeyProvider>(
            local_state(), base::FeatureList::IsEnabled(
                               kSecretPortalKeyProviderUseForEncryption)));
  }
#endif  // BUILDFLAG(IS_LINUX)

  os_crypt_async_ =
      std::make_unique<os_crypt_async::OSCryptAsync>(std::move(providers));
}

void BrowserProcess::StartTearDown() {
  // Debugger must be cleaned up before ProfileManager.
  remote_debugging_server_.reset();

  // Need to clear profiles (download managers) before the IO thread.
  {
    TRACE_EVENT0("shutdown", "BrowserProcess::StartTearDown:ProfileManager");
    // `profile_manager_` must be destroyed before `background_mode_manager_`,
    // because the background mode manager does not stop observing profile
    // changes at destruction (notifying the observers would cause a use-after-
    // free).
    features_.reset();
  }

  local_state_->CommitPendingWrite();

  // This expects to be destroyed before the task scheduler is torn down.
  SystemNetworkContextManager::DeleteInstance();
}

void BrowserProcess::PostDestroyThreads() {}

policy::RadiumBrowserPolicyConnector*
BrowserProcess::browser_policy_connector() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return browser_policy_connector_.get();
}

PrefService* BrowserProcess::local_state() {
  return local_state_.get();
}

GpuModeManager* BrowserProcess::gpu_mode_manager() {
  // TODO
  return nullptr;
}

os_crypt_async::OSCryptAsync* BrowserProcess::os_crypt_async() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return os_crypt_async_.get();
}

policy::PolicyService* BrowserProcess::policy_service() {
  return browser_policy_connector()->GetPolicyService();
}

SystemNetworkContextManager* BrowserProcess::system_network_context_manager() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(SystemNetworkContextManager::GetInstance());
  return SystemNetworkContextManager::GetInstance();
}

GlobalFeatures* BrowserProcess::GetFeatures() {
  return features_.get();
}

void BrowserProcess::OnKeepAliveStateChanged(bool is_keeping_alive) {
  if (is_keeping_alive) {
    Pin();
  } else {
    Unpin();
  }
}

void BrowserProcess::OnKeepAliveRestartStateChanged(bool can_restart) {}

void BrowserProcess::Pin() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  // CHECK(!IsShuttingDown());
}

void BrowserProcess::Unpin() {
#if !BUILDFLAG(IS_ANDROID)
  KeepAliveRegistry::GetInstance()->SetIsShuttingDown();
#endif  // !BUILDFLAG(IS_ANDROID)

  CHECK(base::RunLoop::IsRunningOnCurrentThread());

#if !BUILDFLAG(IS_ANDROID)
  std::move(quit_closure_).Run();
#endif  // !BUILDFLAG(IS_ANDROID)
}
