// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_ANDROID_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_ANDROID_H_

#include "base/functional/callback_helpers.h"
#include "radium/browser/radium_browser_main_parts.h"

namespace crash_reporter {
class ChildExitObserver;
}

class RadiumBrowserMainPartsAndroid : public RadiumBrowserMainParts {
 public:
  explicit RadiumBrowserMainPartsAndroid(
      RadiumFeatureListCreator* radium_feature_list_creator);
  RadiumBrowserMainPartsAndroid(const RadiumBrowserMainPartsAndroid&) = delete;
  RadiumBrowserMainPartsAndroid& operator=(
      const RadiumBrowserMainPartsAndroid&) = delete;

  ~RadiumBrowserMainPartsAndroid() override;

 private:
  // content::BrowserMainParts:
  int PreCreateThreads() override;
  void PostProfileInit(Profile* profile, bool is_initial_profile) override;
  int PreEarlyInitialization() override;

  // ChromeBrowserMainParts:
  void PostBrowserStart() override;

 private:
  std::unique_ptr<crash_reporter::ChildExitObserver> child_exit_observer_;
  // Owns the Java ChromeBackupWatcher object and invokes destroy() on
  // destruction.
  base::ScopedClosureRunner backup_watcher_runner_;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_ANDROID_H_
