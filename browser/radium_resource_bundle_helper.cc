// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_resource_bundle_helper.h"

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/trace_event/trace_event.h"
#include "components/language/core/browser/pref_names.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/common/radium_paths.h"
#include "ui/base/resource/resource_bundle.h"

#if BUILDFLAG(IS_ANDROID)
#include "ui/base/resource/resource_bundle_android.h"
#endif

namespace {

// Initializes the shared instance of ResourceBundle and returns the application
// locale. An empty |actual_locale| value indicates failure.
std::string InitResourceBundleAndDetermineLocale(PrefService* local_state) {
#if BUILDFLAG(IS_ANDROID)
  // In order for SetLoadSecondaryLocalePaks() to work ResourceBundle must
  // not have been created yet.
  DCHECK(!ui::ResourceBundle::HasSharedInstance());
  ui::DetectAndSetLoadNonWebViewLocalePaks();
#endif

  std::string preferred_locale;
  preferred_locale =
      local_state->GetString(language::prefs::kApplicationLocale);

#if BUILDFLAG(IS_CHROMEOS)
  ui::ResourceBundle::SetLottieParsingFunctions(
      &lottie::ParseLottieAsStillImage, &lottie::ParseLottieAsThemedStillImage);
#endif

  TRACE_EVENT0("startup",
               "RadiumBrowserMainParts::InitResourceBundleAndDetermineLocale");
  // On a POSIX OS other than ChromeOS, the parameter that is passed to the
  // method InitSharedInstance is ignored.
  std::string actual_locale = ui::ResourceBundle::InitSharedInstanceWithLocale(
      preferred_locale, nullptr,
#if BUILDFLAG(IS_ANDROID)
      ui::ResourceBundle::LOAD_COMMON_RESOURCES);
#else
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);
#endif  // BUILDFLAG(IS_WIN)
  CHECK(!actual_locale.empty())
      << "Locale could not be found for " << preferred_locale;

  // First run prefs needs data from the ResourceBundle, so load it now.
  {
    TRACE_EVENT0("startup",
                 "RadiumBrowserMainParts::InitResourceBundleAndDetermineLocale:"
                 ":AddDataPack");
    base::FilePath resources_pack_path;
    base::PathService::Get(radium::FILE_RESOURCES_PACK, &resources_pack_path);
#if BUILDFLAG(IS_ANDROID)
    ui::LoadMainAndroidPackFile("assets/resources.pak", resources_pack_path);

    // Avoid loading DFM native resources here, to keep startup lean. These
    // resources are loaded on-use, when an already-installed DFM loads.
#else
    auto GetResourcesPakFilePath = [](const std::string& pak_name) {
      base::FilePath path;
      if (base::PathService::Get(base::DIR_ASSETS, &path)) {
        return path.AppendASCII(pak_name.c_str());
      }
      // Return just the name of the pak file.
#if BUILDFLAG(IS_WIN)
      return base::FilePath(base::ASCIIToWide(pak_name));
#else
      return base::FilePath(pak_name.c_str());
#endif  // BUILDFLAG(IS_WIN)
    };

    // Always load the 1x data pack first as the 2x data pack contains both 1x
    // and 2x images. The 1x data pack only has 1x images, thus passes in an
    // accurate scale factor to gfx::ImageSkia::AddRepresentation.
    if (ui::IsScaleFactorSupported(ui::k100Percent)) {
      ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
          GetResourcesPakFilePath("radium_100_percent.pak"), ui::k100Percent);
    }

    if (ui::IsScaleFactorSupported(ui::k200Percent)) {
      ui::ResourceBundle::GetSharedInstance().AddOptionalDataPackFromPath(
          GetResourcesPakFilePath("radium_200_percent.pak"), ui::k200Percent);
    }

    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        resources_pack_path, ui::kScaleFactorNone);
#endif  // BUILDFLAG(IS_ANDROID)
  }

  return actual_locale;
}
}  // namespace

std::string LoadLocalState(
    RadiumFeatureListCreator* radium_feature_list_creator) {
  base::FilePath user_data_dir;
  if (!base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir)) {
    return std::string();
  }

  return InitResourceBundleAndDetermineLocale(
      radium_feature_list_creator->local_state());
}
