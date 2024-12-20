// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/global_features.h"

#include <memory>

#include "base/check_is_test.h"
#include "base/memory/ptr_util.h"
#include "base/no_destructor.h"
#include "radium/browser/profiles/profile_manager.h"

namespace {

// This is the generic entry point for test code to stub out browser
// functionality. It is called by production code, but only used by tests.
GlobalFeatures::GlobalFeaturesFactory& GetFactory() {
  static base::NoDestructor<GlobalFeatures::GlobalFeaturesFactory> factory;
  return *factory;
}

}  // namespace

// static
std::unique_ptr<GlobalFeatures> GlobalFeatures::CreateGlobalFeatures() {
  if (GetFactory()) {
    CHECK_IS_TEST();
    return GetFactory().Run();
  }
  // Constructor is protected.
  return base::WrapUnique(new GlobalFeatures());
}

GlobalFeatures::GlobalFeatures() = default;
GlobalFeatures::~GlobalFeatures() = default;

void GlobalFeatures::Init() {
  profile_manager_ = ProfileManager::Create();
}
