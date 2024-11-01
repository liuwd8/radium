// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "components/policy/core/common/policy_paths.h"
#include "radium/common/radium_constants.h"
#include "radium/common/radium_paths.h"
#include "radium/common/radium_paths_internal.h"

#if BUILDFLAG(IS_MAC)
#include "base/apple/bundle_locations.h"
#include "base/apple/foundation_util.h"
#endif

namespace {

#if BUILDFLAG(IS_LINUX)
// The path to the external extension <id>.json files.
// /usr/share seems like a good choice, see: http://www.pathname.com/fhs/
const base::FilePath::CharType kFilepathSinglePrefExtensions[] =
    FILE_PATH_LITERAL("/usr/share/radium/extensions");
#endif

// Gets the path for internal plugins.
bool GetInternalPluginsDirectory(base::FilePath* result) {
  return false;
}

// Gets the path for bundled implementations of components. Note that these
// implementations should not be used if higher-versioned component-updated
// implementations are available in DIR_USER_DATA.
bool GetComponentDirectory(base::FilePath* result) {
#if BUILDFLAG(IS_MAC)
  // If called from Chrome, return the framework's Libraries directory.
  if (base::apple::AmIBundled()) {
    *result = radium::GetFrameworkBundlePath();
    DCHECK(!result->empty());
    *result = result->Append("Libraries");
    return true;
  }
// In tests, just look in the assets directory (below).
#endif

  // The rest of the world expects components in the assets directory.
  return base::PathService::Get(base::DIR_ASSETS, result);
}

}  // namespace

namespace radium {

bool PathProvider(int key, base::FilePath* result) {
  // Some keys are just aliases...
  switch (key) {
    case radium::DIR_LOGS:
#ifdef NDEBUG
      // Release builds write to the data dir
      return base::PathService::Get(radium::DIR_USER_DATA, result);
#else
      // Debug builds write next to the binary (in the build tree)
      // TODO(crbug.com/40202595): implement workable solution for Fuchsia.
#if BUILDFLAG(IS_MAC)
      // Apps may not write into their own bundle.
      if (base::apple::AmIBundled()) {
        return base::PathService::Get(radium::DIR_USER_DATA, result);
      }
#endif  // BUILDFLAG(IS_MAC)
      return base::PathService::Get(base::DIR_EXE, result);
#endif  // NDEBUG
  }

  // Assume that we will not need to create the directory if it does not exist.
  // This flag can be set to true for the cases where we want to create it.
  bool create_dir = false;

  base::FilePath cur;
  switch (key) {
    case radium::DIR_USER_DATA:
      if (!GetDefaultUserDataDirectory(&cur)) {
        return false;
      }
      create_dir = true;
      break;
    case radium::DIR_USER_DOCUMENTS:
      if (!GetUserDocumentsDirectory(&cur)) {
        return false;
      }
      create_dir = true;
      break;
    case radium::DIR_USER_MUSIC:
      if (!GetUserMusicDirectory(&cur)) {
        return false;
      }
      break;
    case radium::DIR_USER_PICTURES:
      if (!GetUserPicturesDirectory(&cur)) {
        return false;
      }
      break;
    case radium::DIR_USER_VIDEOS:
      if (!GetUserVideosDirectory(&cur)) {
        return false;
      }
      break;
    case radium::DIR_DEFAULT_DOWNLOADS_SAFE:
#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
      if (!GetUserDownloadsDirectorySafe(&cur)) {
        return false;
      }
      break;
#else
      // Fall through for all other platforms.
#endif
    case radium::DIR_DEFAULT_DOWNLOADS:
#if BUILDFLAG(IS_ANDROID)
      if (!base::android::GetDownloadsDirectory(&cur)) {
        return false;
      }
#else
      if (!GetUserDownloadsDirectory(&cur)) {
        return false;
      }
      // Do not create the download directory here, we have done it twice now
      // and annoyed a lot of users.
#endif
      break;
    case radium::DIR_CRASH_METRICS:
#if BUILDFLAG(IS_CHROMEOS_LACROS)
      cur = base::FilePath(kLacrosLogDirectory);
#else
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)
      break;
    case radium::DIR_CRASH_DUMPS:
// Only use /var/log/{chrome,lacros} on IS_CHROMEOS_DEVICE builds. For
// non-device builds we fall back to the #else below and store relative to the
// default user-data directory.
#if BUILDFLAG(IS_CHROMEOS_DEVICE)
#if BUILDFLAG(IS_CHROMEOS_ASH)
      // ChromeOS uses a separate directory. See http://crosbug.com/25089
      cur = base::FilePath("/var/log/chrome");
#elif BUILDFLAG(IS_CHROMEOS_LACROS)
      cur = base::FilePath(kLacrosLogDirectory);
#endif  // BUILDFlAG(IS_CHROMEOS_ASH)
#elif BUILDFLAG(IS_ANDROID)
      if (!base::android::GetCacheDirectory(&cur)) {
        return false;
      }
#else
      // The crash reports are always stored relative to the default user data
      // directory.  This avoids the problem of having to re-initialize the
      // exception handler after parsing command line options, which may
      // override the location of the app's profile directory.
      // TODO(scottmg): Consider supporting --user-data-dir. See
      // https://crbug.com/565446.
      if (!GetDefaultUserDataDirectory(&cur)) {
        return false;
      }
#endif
#if BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN) || BUILDFLAG(IS_ANDROID)
      cur = cur.Append(FILE_PATH_LITERAL("Crashpad"));
#else
      cur = cur.Append(FILE_PATH_LITERAL("Crash Reports"));
#endif
      create_dir = true;
      break;
    case radium::DIR_LOCAL_TRACES:
#if BUILDFLAG(IS_ANDROID)
      if (!base::PathService::Get(base::DIR_CACHE, &cur)) {
        return false;
      }
#else
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
#endif
      cur = cur.Append(FILE_PATH_LITERAL("Local Traces"));
      create_dir = true;
      break;
#if BUILDFLAG(IS_WIN)
    case radium::DIR_WATCHER_DATA:
      // The watcher data is always stored relative to the default user data
      // directory.  This allows the watcher to be initialized before
      // command-line options have been parsed.
      if (!GetDefaultUserDataDirectory(&cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("Diagnostics"));
      break;
    case radium::DIR_ROAMING_USER_DATA:
      if (!GetDefaultRoamingUserDataDirectory(&cur)) {
        return false;
      }
      create_dir = true;
      break;
#endif
    case radium::DIR_RESOURCES:
#if BUILDFLAG(IS_MAC)
      cur = base::apple::FrameworkBundlePath();
      cur = cur.Append(FILE_PATH_LITERAL("Resources"));
#else
      if (!base::PathService::Get(base::DIR_ASSETS, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("resources"));
#endif
      break;
    case radium::DIR_APP_DICTIONARIES:
#if !BUILDFLAG(IS_WIN)
      // On most platforms, we can't write into the directory where
      // binaries are stored, so keep dictionaries in the user data dir.
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
#else
      // TODO(crbug.com/40840089): Migrate Windows to use `DIR_USER_DATA` like
      // other platforms.
      if (!base::PathService::Get(base::DIR_EXE, &cur)) {
        return false;
      }
#endif
      cur = cur.Append(FILE_PATH_LITERAL("Dictionaries"));
      create_dir = true;
      break;
    case radium::DIR_INTERNAL_PLUGINS:
      if (!GetInternalPluginsDirectory(&cur)) {
        return false;
      }
      break;
    case radium::DIR_COMPONENTS:
      if (!GetComponentDirectory(&cur)) {
        return false;
      }
      break;
    case radium::FILE_LOCAL_STATE:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(radium::kLocalStateFilename);
      break;
    case radium::FILE_RECORDED_SCRIPT:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("script.log"));
      break;
    // PNaCl is currenly installable via the component updater or by being
    // simply built-in.  DIR_PNACL_BASE is used as the base directory for
    // installation via component updater.  DIR_PNACL_COMPONENT will be
    // the final location of pnacl, which is a subdir of DIR_PNACL_BASE.
    case radium::DIR_PNACL_BASE:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("pnacl"));
      break;
    // Where PNaCl files are ultimately located.  The default finds the files
    // inside the InternalPluginsDirectory / build directory, as if it
    // was shipped along with chrome.  The value can be overridden
    // if it is installed via component updater.
    case radium::DIR_PNACL_COMPONENT:
#if BUILDFLAG(IS_MAC)
      // PNaCl really belongs in the InternalPluginsDirectory but actually
      // copying it there would result in the files also being shipped, which
      // we don't want yet. So for now, just find them in the directory where
      // they get built.
      if (!base::PathService::Get(base::DIR_EXE, &cur)) {
        return false;
      }
      if (base::apple::AmIBundled()) {
        // If we're called from chrome, it's beside the app (outside the
        // app bundle), if we're called from a unittest, we'll already be
        // outside the bundle so use the exe dir.
        // exe_dir gave us .../Chromium.app/Contents/MacOS/Chromium.
        cur = cur.DirName();
        cur = cur.DirName();
        cur = cur.DirName();
      }
#else
      if (!GetInternalPluginsDirectory(&cur)) {
        return false;
      }
#endif
      cur = cur.Append(FILE_PATH_LITERAL("pnacl"));
      break;

#if BUILDFLAG(ENABLE_WIDEVINE)
    case radium::DIR_BUNDLED_WIDEVINE_CDM:
      if (!GetComponentDirectory(&cur)) {
        return false;
      }
      cur = cur.AppendASCII(kWidevineCdmBaseDirectory);
      break;

    case radium::DIR_COMPONENT_UPDATED_WIDEVINE_CDM: {
      int components_dir =
#if BUILDFLAG(IS_CHROMEOS_LACROS)
          static_cast<int>(chromeos::lacros_paths::LACROS_SHARED_DIR);
#else
          radium::DIR_USER_DATA;
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)
      if (!base::PathService::Get(components_dir, &cur)) {
        return false;
      }
      cur = cur.AppendASCII(kWidevineCdmBaseDirectory);
      break;
    }
    case radium::FILE_COMPONENT_WIDEVINE_CDM_HINT:
      if (!base::PathService::Get(radium::DIR_COMPONENT_UPDATED_WIDEVINE_CDM,
                                  &cur)) {
        return false;
      }
      cur = cur.Append(kComponentUpdatedWidevineCdmHint);
      break;
#endif  // BUILDFLAG(ENABLE_WIDEVINE)

    case radium::FILE_RESOURCES_PACK:  // Falls through.
    case radium::FILE_DEV_UI_RESOURCES_PACK:
#if BUILDFLAG(IS_MAC)
      cur = base::apple::FrameworkBundlePath();
      cur = cur.Append(FILE_PATH_LITERAL("Resources"))
                .Append(FILE_PATH_LITERAL("radium_resources.pak"));
#elif BUILDFLAG(IS_ANDROID)
      if (!base::PathService::Get(ui::DIR_RESOURCE_PAKS_ANDROID, &cur)) {
        return false;
      }
      if (key == radium::FILE_DEV_UI_RESOURCES_PACK) {
        cur = cur.Append(FILE_PATH_LITERAL("dev_ui_resources.pak"));
      } else {
        DCHECK_EQ(radium::FILE_RESOURCES_PACK, key);
        cur = cur.Append(FILE_PATH_LITERAL("radium_resources.pak"));
      }
#else
      // If we're not bundled on mac or Android, resources.pak should be in
      // the "assets" location (e.g. next to the binary, on many platforms, or
      // in /pkg/data under Fuchsia, etc).
      if (!base::PathService::Get(base::DIR_ASSETS, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("radium_resources.pak"));
#endif
      break;

#if BUILDFLAG(IS_CHROMEOS_LACROS)
    case radium::FILE_RESOURCES_FOR_SHARING_PACK:
      if (!GetDefaultUserDataDirectory(&cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL(crosapi::kSharedResourcesPackName));
      break;
    case radium::FILE_ASH_RESOURCES_PACK:
      if (!base::PathService::Get(chromeos::lacros_paths::ASH_RESOURCES_DIR,
                                  &cur)) {
        return false;
      }
      cur = cur.Append("resources.pak");
      break;
#endif

#if BUILDFLAG(IS_CHROMEOS_ASH)
    case radium::DIR_CHROMEOS_WALLPAPERS:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("wallpapers"));
      break;
    case radium::DIR_CHROMEOS_WALLPAPER_THUMBNAILS:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("wallpaper_thumbnails"));
      break;
    case radium::DIR_CHROMEOS_CUSTOM_WALLPAPERS:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("custom_wallpapers"));
      break;
    case radium::DIR_CHROMEOS_CRD_DATA:
      if (!GetChromeOsCrdDataDirInternal(&cur,
                                         /*should_be_created=*/&create_dir)) {
        return false;
      }
      break;
#endif
    // The following are only valid in the development environment, and
    // will fail if executed from an installed executable (because the
    // generated path won't exist).
    case radium::DIR_GEN_TEST_DATA:
      if (!base::PathService::Get(base::DIR_OUT_TEST_DATA_ROOT, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("test_data"));
      if (!base::PathExists(cur)) {  // We don't want to create this.
        return false;
      }
      break;
    case radium::DIR_TEST_DATA:
      if (!base::PathService::Get(base::DIR_SRC_TEST_DATA_ROOT, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("radium"));
      cur = cur.Append(FILE_PATH_LITERAL("test"));
      cur = cur.Append(FILE_PATH_LITERAL("data"));
      if (!base::PathExists(cur)) {  // We don't want to create this.
        return false;
      }
      break;
    case radium::DIR_TEST_TOOLS:
      if (!base::PathService::Get(base::DIR_SRC_TEST_DATA_ROOT, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("radium"));
      cur = cur.Append(FILE_PATH_LITERAL("tools"));
      cur = cur.Append(FILE_PATH_LITERAL("test"));
      if (!base::PathExists(cur)) {  // We don't want to create this
        return false;
      }
      break;
#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_OPENBSD)
    case radium::DIR_POLICY_FILES: {
      cur = base::FilePath(policy::kPolicyPath);
      break;
    }
#endif
// TODO(crbug.com/40118868): Revisit once build flag switch of lacros-chrome is
// complete.
#if BUILDFLAG(IS_CHROMEOS_ASH) ||                              \
    ((BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)) && \
     BUILDFLAG(CHROMIUM_BRANDING)) ||                          \
    BUILDFLAG(IS_MAC)
    case radium::DIR_USER_EXTERNAL_EXTENSIONS: {
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("External Extensions"));
      break;
    }
#endif
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
    case radium::DIR_STANDALONE_EXTERNAL_EXTENSIONS: {
      cur = base::FilePath(kFilepathSinglePrefExtensions);
      break;
    }
#endif
    case radium::DIR_EXTERNAL_EXTENSIONS:
#if BUILDFLAG(IS_MAC)
      if (!radium::GetGlobalApplicationSupportDirectory(&cur)) {
        return false;
      }

      cur = cur.Append(FILE_PATH_LITERAL("Liuwd8"))
                .Append(FILE_PATH_LITERAL("Radium"))
                .Append(FILE_PATH_LITERAL("External Extensions"));
#else
      if (!base::PathService::Get(base::DIR_MODULE, &cur)) {
        return false;
      }

      cur = cur.Append(FILE_PATH_LITERAL("extensions"));
      create_dir = true;
#endif
      break;

    case radium::DIR_DEFAULT_APPS:
#if BUILDFLAG(IS_MAC)
      cur = base::apple::FrameworkBundlePath();
      cur = cur.Append(FILE_PATH_LITERAL("Default Apps"));
#else
      if (!base::PathService::Get(base::DIR_MODULE, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("default_apps"));
#endif
      break;

#if BUILDFLAG(ENABLE_EXTENSIONS) && \
    (BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_MAC))
    case radium::DIR_NATIVE_MESSAGING:
#if BUILDFLAG(IS_MAC)
      cur = base::FilePath(FILE_PATH_LITERAL(
          "/Library/Application Support/Radium/NativeMessagingHosts"));
#else  // BUILDFLAG(IS_MAC)
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
      cur = base::FilePath(
          FILE_PATH_LITERAL("/etc/opt/chrome/native-messaging-hosts"));
#else
      cur = base::FilePath(
          FILE_PATH_LITERAL("/etc/chromium/native-messaging-hosts"));
#endif
#endif  // !BUILDFLAG(IS_MAC)
      break;

    case radium::DIR_USER_NATIVE_MESSAGING:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("NativeMessagingHosts"));
      break;
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_MAC)
#if !BUILDFLAG(IS_ANDROID)
    case radium::DIR_GLOBAL_GCM_STORE:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(kGCMStoreDirname);
      break;
#endif  // !BUILDFLAG(IS_ANDROID)
#if BUILDFLAG(IS_CHROMEOS_ASH)
    case radium::FILE_CHROME_OS_TPM_FIRMWARE_UPDATE_LOCATION:
      cur = base::FilePath(kChromeOSTPMFirmwareUpdateLocation);
      break;
    case radium::FILE_CHROME_OS_TPM_FIRMWARE_UPDATE_SRK_VULNERABLE_ROCA:
      cur = base::FilePath(kChromeOSTPMFirmwareUpdateSRKVulnerableROCA);
      break;
    case radium::DIR_CHROMEOS_HOMEDIR_MOUNT:
#if BUILDFLAG(IS_CHROMEOS_DEVICE)
      cur = base::FilePath(kChromeOSCryptohomeMountRoot);
#else
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(kFakeCryptohomeMountRootDirname);
#endif  // BUILDFLAG(IS_CHROMEOS_DEVICE)
      break;
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)
    case radium::DIR_OPTIMIZATION_GUIDE_PREDICTION_MODELS:
      if (!base::PathService::Get(radium::DIR_USER_DATA, &cur)) {
        return false;
      }
      cur = cur.Append(FILE_PATH_LITERAL("OptimizationGuidePredictionModels"));
      create_dir = true;
      break;

    default:
      return false;
  }

  if (create_dir && !base::PathExists(cur) && !base::CreateDirectory(cur)) {
    return false;
  }

  *result = cur;
  return true;
}

// This cannot be done as a static initializer sadly since Visual Studio will
// eliminate this object file if there is no direct entry point into it.
void RegisterPathProvider() {
  base::PathService::RegisterProvider(PathProvider, PATH_START, PATH_END);
}

}  // namespace radium
