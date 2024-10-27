// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/profiles/profile.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/location.h"
#include "base/task/thread_pool.h"
#include "base/trace_event/trace_event.h"
#include "components/keyed_service/core/simple_factory_key.h"
#include "components/keyed_service/core/simple_key_map.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service_factory.h"
#include "components/profile_metrics/browser_profile_type.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "radium/browser/prefs/profile_prefs.h"

std::unique_ptr<Profile> Profile::CreateProfile(const base::FilePath& path,
                                                Delegate* delegate,
                                                CreateMode create_mode) {
  TRACE_EVENT1("browser,startup", "Profile::CreateProfile", "profile_path",
               path.AsUTF8Unsafe());

  // Get sequenced task runner for making sure that file operations of
  // this profile are executed in expected order (what was previously assured by
  // the FILE thread).
  scoped_refptr<base::SequencedTaskRunner> io_task_runner =
      base::ThreadPool::CreateSequencedTaskRunner(
          {base::TaskShutdownBehavior::BLOCK_SHUTDOWN, base::MayBlock()});

  io_task_runner->PostTask(FROM_HERE, base::BindOnce(
                                          [](const base::FilePath& path) {
                                            if (!base::PathExists(path)) {
                                              base::CreateDirectory(path);
                                            }
                                          },
                                          path));

  std::unique_ptr<Profile> profile = base::WrapUnique(
      new Profile(path, delegate, create_mode, io_task_runner));
  return profile;
}

// static
Profile* Profile::FromBrowserContext(content::BrowserContext* browser_context) {
  if (!browser_context) {
    return nullptr;
  }

  return static_cast<Profile*>(browser_context);
}
// static
Profile* Profile::FromWebUI(content::WebUI* web_ui) {
  return FromBrowserContext(web_ui->GetWebContents()->GetBrowserContext());
}

Profile::Profile(const base::FilePath& path,
                 Delegate* delegate,
                 CreateMode create_mode,
                 scoped_refptr<base::SequencedTaskRunner> io_task_runner)
    : is_off_the_record_(false),
      path_(path),
      io_task_runner_(io_task_runner),
      delegate_(delegate) {
  CHECK(!path.empty()) << "Using an empty path will attempt to write profile "
                          "files to the root directory!";

  profile_metrics::SetBrowserProfileType(
      this, profile_metrics::BrowserProfileType::kRegular);
  if (delegate_) {
    delegate_->OnProfileCreationStarted(this, create_mode);
  }

  // The ProfileImpl can be created both synchronously and asynchronously.
  bool async_prefs = create_mode == CreateMode::kAsynchronous;

#if BUILDFLAG(IS_ANDROID)
  auto* startup_data = g_browser_process->startup_data();
  DCHECK(startup_data && startup_data->GetProfileKey());
  TakePrefsFromStartupData();
  async_prefs = false;
#else
  LoadPrefsForNormalStartup(async_prefs);
#endif

  // Register on BrowserContext.
  user_prefs::UserPrefs::Set(this, prefs_.get());

  SimpleKeyMap::GetInstance()->Associate(this, key_.get());

  if (async_prefs) {
    // Wait for the notification that prefs has been loaded
    // (successfully or not).  Note that we can use base::Unretained
    // because the PrefService is owned by this class and lives on
    // the same thread.
    prefs_->AddPrefInitObserver(base::BindOnce(
        &Profile::OnPrefsLoaded, base::Unretained(this), create_mode));
  } else {
    // Prefs were loaded synchronously so we can continue directly.
    OnPrefsLoaded(create_mode, true);
  }
}

Profile::~Profile() {
  NotifyWillBeDestroyed();

  SimpleKeyMap::GetInstance()->Dissociate(this);
}

PrefService* Profile::GetPrefs() {
  return const_cast<PrefService*>(
      static_cast<const Profile*>(this)->GetPrefs());
}

const PrefService* Profile::GetPrefs() const {
  DCHECK(prefs_);  // Should explicitly be initialized.
  return prefs_.get();
}

base::FilePath Profile::GetBaseName() const {
  return path_.BaseName();
}

bool Profile::IsRegularProfile() const {
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kRegular;
}

bool Profile::IsIncognitoProfile() const {
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kIncognito;
}

bool Profile::IsGuestSession() const {
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kGuest;
}

bool Profile::IsSystemProfile() const {
#if BUILDFLAG(IS_ANDROID)
  DCHECK_NE(profile_metrics::GetBrowserProfileType(this),
            profile_metrics::BrowserProfileType::kSystem);
  return false;
#else  // BUILDFLAG(IS_ANDROID)
  return profile_metrics::GetBrowserProfileType(this) ==
         profile_metrics::BrowserProfileType::kSystem;
#endif
}

Profile* Profile::GetOriginalProfile() {
  return this;
}

const Profile* Profile::GetOriginalProfile() const {
  return this;
}

bool Profile::ShouldRestoreOldSessionCookies() const {
  return true;
}

bool Profile::ShouldPersistSessionCookies() const {
  return true;
}

scoped_refptr<base::SequencedTaskRunner> Profile::GetIOTaskRunner() {
  return io_task_runner_;
}

// content::BrowserContext implementation ------------------------------------
std::unique_ptr<content::ZoomLevelDelegate> Profile::CreateZoomLevelDelegate(
    const base::FilePath& partition_path) {
  return nullptr;
}

base::FilePath Profile::GetPath() {
  return path_;
}

bool Profile::IsOffTheRecord() {
  return is_off_the_record_;
}

content::DownloadManagerDelegate* Profile::GetDownloadManagerDelegate() {
  return nullptr;
}

content::BrowserPluginGuestManager* Profile::GetGuestManager() {
  return nullptr;
}

storage::SpecialStoragePolicy* Profile::GetSpecialStoragePolicy() {
  return nullptr;
}

content::PlatformNotificationService*
Profile::GetPlatformNotificationService() {
  return nullptr;
}

content::PushMessagingService* Profile::GetPushMessagingService() {
  return nullptr;
}

content::StorageNotificationService* Profile::GetStorageNotificationService() {
  return nullptr;
}

content::SSLHostStateDelegate* Profile::GetSSLHostStateDelegate() {
  return nullptr;
}

content::PermissionControllerDelegate*
Profile::GetPermissionControllerDelegate() {
  return nullptr;
}

content::ReduceAcceptLanguageControllerDelegate*
Profile::GetReduceAcceptLanguageControllerDelegate() {
  return nullptr;
}

content::ClientHintsControllerDelegate*
Profile::GetClientHintsControllerDelegate() {
  return nullptr;
}

content::BackgroundFetchDelegate* Profile::GetBackgroundFetchDelegate() {
  return nullptr;
}

content::BackgroundSyncController* Profile::GetBackgroundSyncController() {
  return nullptr;
}

content::BrowsingDataRemoverDelegate*
Profile::GetBrowsingDataRemoverDelegate() {
  return nullptr;
}

void Profile::LoadPrefsForNormalStartup(bool async_prefs) {
  const base::FilePath& path = GetPath();
  key_ = std::make_unique<SimpleFactoryKey>(path, path.empty());

  auto pref_registry = base::MakeRefCounted<user_prefs::PrefRegistrySyncable>();
  prefs::RegisterProfilePrefs(pref_registry.get(), "");

  PrefServiceFactory pref_service_factory;
  if (IsOffTheRecord()) {
    pref_service_factory.set_user_prefs(
        base::MakeRefCounted<InMemoryPrefStore>());
  } else {
    pref_service_factory.set_user_prefs(base::MakeRefCounted<JsonPrefStore>(
        path_.Append(FILE_PATH_LITERAL("Preferences")), nullptr,
        io_task_runner_));
  }

  pref_service_factory.set_async(async_prefs);
  prefs_ = pref_service_factory.Create(pref_registry);
}

void Profile::OnPrefsLoaded(CreateMode create_mode, bool success) {
  TRACE_EVENT0("browser", "ProfileImpl::OnPrefsLoaded");
  if (!success) {
    if (delegate_) {
      delegate_->OnProfileCreationFinished(this, create_mode, false, false);
    }
    return;
  }

  if (delegate_) {
    delegate_->OnProfileCreationFinished(this, create_mode, success, true);
  }
}
