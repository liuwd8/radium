// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_METRICS_RADIUM_FEATURE_LIST_CREATOR_H_
#define RADIUM_BROWSER_METRICS_RADIUM_FEATURE_LIST_CREATOR_H_

#include <memory>
#include <string>

class PrefService;
class RadiumBrowserFieldTrials;

namespace policy {
class RadiumBrowserPolicyConnector;
}

class RadiumFeatureListCreator {
 public:
  explicit RadiumFeatureListCreator();
  RadiumFeatureListCreator(const RadiumFeatureListCreator&) = delete;
  RadiumFeatureListCreator& operator=(const RadiumFeatureListCreator&) = delete;

  ~RadiumFeatureListCreator();

  // Initializes all necessary parameters to create the feature list and calls
  // base::FeatureList::SetInstance() to set the global instance.
  void CreateFeatureList();

  // Sets the application locale and verifies (via a CHECK) that it matches
  // what was used when creating field trials.
  void SetApplicationLocale(const std::string& locale);

  // Overrides cached UI strings on the resource bundle once it is initialized.
  void OverrideCachedUIStrings();

  // Passes ownership of the |local_state_| to the caller.
  std::unique_ptr<PrefService> TakePrefService();

  // Passes ownership of the |browser_policy_connector_| to the caller.
  std::unique_ptr<policy::RadiumBrowserPolicyConnector>
  TakeRadiumBrowserPolicyConnector();

  PrefService* local_state() { return local_state_.get(); }

 private:
  void CreatePrefService();
  void SetUpFieldTrials();

  // Must be destroyed after |local_state_|.
  std::unique_ptr<policy::RadiumBrowserPolicyConnector>
      browser_policy_connector_;

  // If TakePrefService() is called, the caller will take the ownership
  // of this variable. Stop using this variable afterwards.
  std::unique_ptr<PrefService> local_state_;

  std::unique_ptr<RadiumBrowserFieldTrials> browser_field_trials_;
};

#endif  // RADIUM_BROWSER_METRICS_RADIUM_FEATURE_LIST_CREATOR_H_
