// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_GLOBAL_FEATURES_H_
#define RADIUM_BROWSER_GLOBAL_FEATURES_H_

#include <memory.h>

#include "base/functional/callback.h"
#include "build/build_config.h"

class GlobalFeatures {
 public:
  static std::unique_ptr<GlobalFeatures> CreateGlobalFeatures();

  explicit GlobalFeatures();
  GlobalFeatures(const GlobalFeatures&) = delete;
  GlobalFeatures& operator=(const GlobalFeatures&) = delete;

  ~GlobalFeatures();

  // Call this method to stub out GlobalFeatures for tests.
  using GlobalFeaturesFactory =
      base::RepeatingCallback<std::unique_ptr<GlobalFeatures>()>;
  static void ReplaceGlobalFeaturesForTesting(GlobalFeaturesFactory factory);

  // Called exactly once to initialize features.
  void Init();
};

#endif  // RADIUM_BROWSER_GLOBAL_FEATURES_H_
