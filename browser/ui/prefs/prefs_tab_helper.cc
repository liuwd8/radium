// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/prefs/prefs_tab_helper.h"

#include <stddef.h>
#include <stdint.h>

#include <set>
#include <string>

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "build/build_config.h"
#include "components/language/core/browser/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/overlay_user_pref_store.h"
#include "components/prefs/pref_service.h"
#include "components/proxy_config/proxy_config_pref_names.h"
#include "components/strings/grit/components_locale_settings.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/buildflags/buildflags.h"
#include "media/media_buildflags.h"
#include "radium/browser/profiles/profile.h"
#include "radium/common/pref_names.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/common/renderer_preferences/renderer_preferences.h"
#include "third_party/blink/public/common/web_preferences/web_preferences.h"
#include "ui/base/l10n/l10n_util.h"

using blink::web_pref::WebPreferences;
using content::WebContents;

PrefsTabHelper::PrefsTabHelper(WebContents* contents)
    : content::WebContentsUserData<PrefsTabHelper>(*contents),
      profile_(Profile::FromBrowserContext(contents->GetBrowserContext())) {}

PrefsTabHelper::~PrefsTabHelper() {}

// static
void PrefsTabHelper::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry,
    const std::string& locale) {
  WebPreferences pref_defaults;
  registry->RegisterBooleanPref(prefs::kEnableReferrers, true);
}

// static
void PrefsTabHelper::GetServiceInstance() {}

void PrefsTabHelper::UpdateWebPreferences() {
  GetWebContents().NotifyPreferencesChanged();
}

void PrefsTabHelper::UpdateRendererPreferences() {
  GetWebContents().SyncRendererPrefs();
}

void PrefsTabHelper::OnFontFamilyPrefChanged(const std::string& pref_name) {}

void PrefsTabHelper::OnWebPrefChanged(const std::string& pref_name) {
  // Use PostTask to dispatch the OnWebkitPreferencesChanged notification to
  // give other observers (particularly the FontFamilyCache) a chance to react
  // to the pref change.
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&PrefsTabHelper::NotifyWebkitPreferencesChanged,
                                weak_ptr_factory_.GetWeakPtr(), pref_name));
}

void PrefsTabHelper::NotifyWebkitPreferencesChanged(
    const std::string& pref_name) {
#if !BUILDFLAG(IS_ANDROID)
  OnFontFamilyPrefChanged(pref_name);
#endif

  GetWebContents().OnWebPreferencesChanged();
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(PrefsTabHelper);
