// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_PROFILES_PROFILE_H_
#define RADIUM_BROWSER_PROFILES_PROFILE_H_

#include "content/public/browser/browser_context.h"

class PrefService;
class SimpleFactoryKey;

namespace content {
class WebUI;
}

namespace sync_preferences {
class PrefServiceSyncable;
}

class Profile : public content::BrowserContext {
 public:
  enum class CreateMode {
    kSynchronous,
    kAsynchronous,
  };

  class Delegate {
   public:
    virtual ~Delegate();

    // Called when creation of the profile is started.
    virtual void OnProfileCreationStarted(Profile* profile,
                                          CreateMode create_mode) = 0;

    // Called when creation of the profile is finished.
    virtual void OnProfileCreationFinished(Profile* profile,
                                           CreateMode create_mode,
                                           bool success,
                                           bool is_new_profile) = 0;
  };

  // Create a new profile given a path. If `create_mode` is kAsynchronous then
  // the profile is initialized asynchronously.
  // Can return null if `create_mode` is kSynchronous and the creation of
  // the profile directory fails.
  static std::unique_ptr<Profile> CreateProfile(const base::FilePath& path,
                                                Delegate* delegate,
                                                CreateMode create_mode);

  // Returns the profile corresponding to the given browser context.
  static Profile* FromBrowserContext(content::BrowserContext* browser_context);

  // Returns the profile corresponding to the given WebUI.
  static Profile* FromWebUI(content::WebUI* web_ui);

  Profile(const Profile&) = delete;
  Profile& operator=(const Profile&) = delete;

  ~Profile() override;

  // Retrieves a pointer to the PrefService that manages the
  // preferences for this user profile.
  PrefService* GetPrefs();
  const PrefService* GetPrefs() const;

  // Returns the base name of the profile, which is the profile directory name
  // within the user data directory, e.g. "Default", "Profile 1", "Profile 2".
  base::FilePath GetBaseName() const;

  // IsRegularProfile(), IsSystemProfile(), IsIncognitoProfile(), and
  // IsGuestSession() are mutually exclusive.
  // Note: IsGuestSession() is not mutually exclusive with the rest of the
  // methods mentioned above on Ash. TODO(crbug.com/40233408).
  //
  // IsSystemProfile() returns true for both regular and off-the-record profile
  //   of the system profile.
  // IsOffTheRecord() is true for the off the record profile of Incognito mode,
  // system profile, Guest sessions, and also non-primary OffTheRecord profiles.

  // Returns whether it's a regular profile.
  bool IsRegularProfile() const;

  // Returns whether it is an Incognito profile. An Incognito profile is an
  // off-the-record profile that is used for incognito mode.
  bool IsIncognitoProfile() const;

  // Returns whether it is a Guest session. This covers both regular and
  // off-the-record profiles of a Guest session.
  bool IsGuestSession() const;

  // Returns whether it is a system profile.
  bool IsSystemProfile() const;

  // Return the original "recording" profile. This method returns this if the
  // profile is not OffTheRecord.
  Profile* GetOriginalProfile();

  // Return the original "recording" profile. This method returns this if the
  // profile is not OffTheRecord.
  const Profile* GetOriginalProfile() const;

  bool ShouldRestoreOldSessionCookies() const;
  bool ShouldPersistSessionCookies() const;

  // content::BrowserContext:
  std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
  base::FilePath GetPath() override;
  bool IsOffTheRecord() override;
  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PlatformNotificationService* GetPlatformNotificationService()
      override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::StorageNotificationService* GetStorageNotificationService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  content::PermissionControllerDelegate* GetPermissionControllerDelegate()
      override;
  content::ReduceAcceptLanguageControllerDelegate*
  GetReduceAcceptLanguageControllerDelegate() override;
  content::ClientHintsControllerDelegate* GetClientHintsControllerDelegate()
      override;
  content::BackgroundFetchDelegate* GetBackgroundFetchDelegate() override;
  content::BackgroundSyncController* GetBackgroundSyncController() override;
  content::BrowsingDataRemoverDelegate* GetBrowsingDataRemoverDelegate()
      override;

 private:
  explicit Profile(const base::FilePath& path,
                   Delegate* delegate,
                   CreateMode create_mode,
                   scoped_refptr<base::SequencedTaskRunner> io_task_runner);

  // Creates |prefs| from scratch in normal startup.
  void LoadPrefsForNormalStartup(bool async_prefs);

  // Switch locale (when possible) and proceed to OnLocaleReady().
  void OnPrefsLoaded(CreateMode create_mode, bool success);

  const bool is_off_the_record_;

  base::FilePath path_;

  // Task runner used for file access in the profile path.
  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;

  std::unique_ptr<PrefService> prefs_;

  // The key to index KeyedService instances created by
  // SimpleKeyedServiceFactory.
  std::unique_ptr<SimpleFactoryKey> key_;

  raw_ptr<Profile::Delegate> delegate_;
};

#endif  // RADIUM_BROWSER_PROFILES_PROFILE_H_
