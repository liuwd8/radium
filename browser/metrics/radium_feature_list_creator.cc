// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/metrics/radium_feature_list_creator.h"

#include "base/check.h"
#include "base/debug/leak_annotations.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "content/child/field_trial.h"
#include "radium/browser/browser_prefs.h"
#include "radium/browser/policy/radium_browser_policy_connector.h"
#include "radium/browser/radium_browser_field_trials.h"
#include "radium/common/radium_paths.h"

RadiumFeatureListCreator::RadiumFeatureListCreator() = default;
RadiumFeatureListCreator::~RadiumFeatureListCreator() = default;

void RadiumFeatureListCreator::CreateFeatureList() {
  CreatePrefService();
  SetUpFieldTrials();
}

void RadiumFeatureListCreator::SetApplicationLocale(const std::string& locale) {
}

void RadiumFeatureListCreator::OverrideCachedUIStrings() {}

std::unique_ptr<PrefService> RadiumFeatureListCreator::TakePrefService() {
  return std::move(local_state_);
}

std::unique_ptr<policy::RadiumBrowserPolicyConnector>
RadiumFeatureListCreator::TakeRadiumBrowserPolicyConnector() {
  return std::move(browser_policy_connector_);
}

void RadiumFeatureListCreator::CreatePrefService() {
  base::FilePath local_state_file;
  bool result =
      base::PathService::Get(radium::FILE_LOCAL_STATE, &local_state_file);
  DCHECK(result);

  auto pref_registry = base::MakeRefCounted<PrefRegistrySimple>();
  RegisterLocalState(pref_registry.get());

  browser_policy_connector_ =
      std::make_unique<policy::RadiumBrowserPolicyConnector>();

  // ManagementService needs Local State but creating local state needs
  // ManagementService, instantiate the underlying PrefStore early and share it
  // between both.
  auto local_state_pref_store =
      base::MakeRefCounted<JsonPrefStore>(local_state_file);

  PrefServiceFactory pref_service_factory;
  pref_service_factory.set_user_prefs(local_state_pref_store);

  local_state_ = pref_service_factory.Create(pref_registry);
}

void RadiumFeatureListCreator::SetUpFieldTrials() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();

  // Initialize statistical testing infrastructure.
  //
  // This is intentionally leaked since it needs to live for the duration of the
  // process and there's no benefit in cleaning it up at exit.
  base::FieldTrialList* leaked_field_trial_list = new base::FieldTrialList();
  ANNOTATE_LEAKING_OBJECT_PTR(leaked_field_trial_list);
  std::ignore = leaked_field_trial_list;

  // Ensure any field trials in browser are reflected into the child process.
  base::FieldTrialList::CreateTrialsInChildProcess(command_line);
  std::unique_ptr<base::FeatureList> feature_list(new base::FeatureList);
  base::FieldTrialList::ApplyFeatureOverridesInChildProcess(feature_list.get());
  base::FeatureList::SetInstance(std::move(feature_list));
}
