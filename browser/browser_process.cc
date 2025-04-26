// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_process.h"

#include <iostream>
#include <memory>

#include "base/run_loop.h"
#include "base/sequence_checker.h"
#include "base/synchronization/waitable_event.h"
#include "base/trace_event/trace_event.h"
#include "components/language/core/browser/pref_names.h"
#include "components/os_crypt/async/browser/key_provider.h"
#include "components/os_crypt/async/browser/os_crypt_async.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/browser_process_platform_part.h"
#include "radium/browser/devtools/remote_debugging_server.h"
#include "radium/browser/global_features.h"
#include "radium/browser/lifetime/browser_shutdown.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/policy/radium_browser_policy_connector.h"
#include "radium/browser/profiles/profile_manager.h"
#include "radium/common/pref_names.h"

#if BUILDFLAG(IS_WIN)
#include "base/win/windows_version.h"
#include "components/app_launch_prefetch/app_launch_prefetch.h"
#include "components/os_crypt/async/browser/dpapi_key_provider.h"
#endif

#if BUILDFLAG(IS_LINUX)
#include "components/os_crypt/async/browser/secret_portal_key_provider.h"
#endif

#if !BUILDFLAG(IS_ANDROID)
#include "components/keep_alive_registry/keep_alive_registry.h"
#include "radium/browser/lifetime/application_lifetime_desktop.h"
#endif

namespace {
BrowserProcess* g_browser_process = nullptr;

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_OZONE)
// How long to wait for the File thread to complete during EndSession, on Linux
// and Windows. We have a timeout here because we're unable to run the UI
// messageloop and there's some deadlock risk. Our only option is to exit
// anyway.
constexpr base::TimeDelta kEndSessionTimeout = base::Seconds(10);
#endif

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

// Used at the end of session to block the UI thread for completion of sentinel
// tasks on the set of threads used to persist profile data and local state.
// This is done to ensure that the data has been persisted to disk before
// continuing.
class RundownTaskCounter
    : public base::RefCountedThreadSafe<RundownTaskCounter> {
 public:
  RundownTaskCounter();

  RundownTaskCounter(const RundownTaskCounter&) = delete;
  RundownTaskCounter& operator=(const RundownTaskCounter&) = delete;

  // Increments |count_| and returns a closure bound to Decrement(). All
  // closures returned by this RundownTaskCounter's GetRundownClosure() method
  // must be invoked for TimedWait() to complete its wait without timing
  // out.
  base::OnceClosure GetRundownClosure();

  // Waits until the count is zero or |timeout| expires.
  // This can only be called once per instance.
  void TimedWait(base::TimeDelta timeout);

 private:
  friend class base::RefCountedThreadSafe<RundownTaskCounter>;
  ~RundownTaskCounter() {}

  // Decrements the counter and releases the waitable event on transition to
  // zero.
  void Decrement();

  // The count starts at one to defer the possibility of one->zero transitions
  // until TimedWait is called.
  base::AtomicRefCount count_{1};
  base::WaitableEvent waitable_event_;
};

RundownTaskCounter::RundownTaskCounter() = default;

base::OnceClosure RundownTaskCounter::GetRundownClosure() {
  // As the count starts off at one, it should never get to zero unless
  // TimedWait has been called.
  DCHECK(!count_.IsZero());

  count_.Increment();

  return base::BindOnce(&RundownTaskCounter::Decrement, this);
}

void RundownTaskCounter::Decrement() {
  if (!count_.Decrement()) {
    waitable_event_.Signal();
  }
}

void RundownTaskCounter::TimedWait(base::TimeDelta timeout) {
  // Decrement the excess count from the constructor.
  Decrement();

  // RundownTaskCounter::TimedWait() could return
  // |waitable_event_.TimedWait()|'s result if any user ever cared about whether
  // it returned per success or timeout. Currently no user of this API cares and
  // as such this return value is ignored.
  waitable_event_.TimedWait(timeout);
}

}  // namespace

BrowserProcess* BrowserProcess::Get() {
  return g_browser_process;
}

void BrowserProcess::RegisterPrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(prefs::kDefaultBrowserSettingEnabled, false);

  registry->RegisterBooleanPref(prefs::kAllowCrossOriginAuthPrompt, false);

  registry->RegisterStringPref(language::prefs::kApplicationLocale,
                               std::string());
#if BUILDFLAG(IS_CHROMEOS)
  registry->RegisterStringPref(prefs::kOwnerLocale, std::string());
  registry->RegisterStringPref(prefs::kHardwareKeyboardLayout, std::string());
#endif  // BUILDFLAG(IS_CHROMEOS)

  registry->RegisterBooleanPref(prefs::kDevToolsRemoteDebuggingAllowed, true);

#if BUILDFLAG(IS_LINUX)
  os_crypt_async::SecretPortalKeyProvider::RegisterLocalPrefs(registry);
#endif
}

BrowserProcess::BrowserProcess(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : browser_policy_connector_(
          radium_feature_list_creator->TakeRadiumBrowserPolicyConnector()),
      local_state_(radium_feature_list_creator->TakePrefService()),
      platform_part_(std::make_unique<BrowserProcessPlatformPart>()) {
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
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
#if !BUILDFLAG(IS_ANDROID)
  auto maybe_remote_debugging_server =
      RemoteDebuggingServer::GetInstance(local_state_.get());
  if (maybe_remote_debugging_server.has_value()) {
    remote_debugging_server_ = std::move(*maybe_remote_debugging_server);
    return;
  }

  // For errors, follow content/browser/devtools/devtools_http_handler.cc that
  // reports its remote debugging port on stderr for symmetry.
  switch (maybe_remote_debugging_server.error()) {
    case RemoteDebuggingServer::NotStartedReason::kNotRequested:
      break;
    case RemoteDebuggingServer::NotStartedReason::kDisabledByPolicy:
      std::cerr
          << "\nDevTools remote debugging is disallowed by the system admin.\n"
          << std::endl;
      break;
    case RemoteDebuggingServer::NotStartedReason::kDisabledByDefaultUserDataDir:
      std::cerr << "\nDevTools remote debugging requires a non-default data "
                   "directory. Specify this using --user-data-dir.\n"
                << std::endl;
      break;
  }
#endif
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

  // providers.emplace_back(std::make_pair(
  //     // Note: 15 is chosen to be higher than the 10 precedence above for
  //     // DPAPI. This ensures that when the the provider is enabled for
  //     // encryption, the App-Bound encryption key is used and not the DPAPI
  //     // one.
  //     /*precedence=*/15u,
  //     std::make_unique<os_crypt_async::AppBoundEncryptionProviderWin>(
  //         local_state(),
  //         base::FeatureList::IsEnabled(
  //             features::kUseAppBoundEncryptionProviderForEncryption))));
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

bool BrowserProcess::IsShuttingDown() {
#if !BUILDFLAG(IS_ANDROID)
  return KeepAliveRegistry::GetInstance()->IsShuttingDown();
#else
  return browser_shutdown::HasShutdownStarted();
#endif
}

#if !BUILDFLAG(IS_ANDROID)
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
#endif

void BrowserProcess::EndSession() {
  // Mark all the profiles as clean.
  ProfileManager* pm = features_->profile_manager();
  scoped_refptr<RundownTaskCounter> rundown_counter =
      base::MakeRefCounted<RundownTaskCounter>();
  for (Profile* profile : pm->GetLoadedProfiles()) {
    if (profile->GetPrefs()) {
      profile->GetPrefs()->CommitPendingWrite(
          base::OnceClosure(), rundown_counter->GetRundownClosure());
    }
  }

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_OZONE)
  rundown_counter->TimedWait(kEndSessionTimeout);
#else
  NOTIMPLEMENTED();
#endif
}

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

BrowserProcessPlatformPart* BrowserProcess::platform_part() {
  return platform_part_.get();
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

  radium::ShutdownIfNeeded();
#endif  // !BUILDFLAG(IS_ANDROID)
}
