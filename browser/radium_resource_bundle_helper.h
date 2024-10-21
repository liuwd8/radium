// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_RESOURCE_BUNDLE_HELPER_H_
#define RADIUM_BROWSER_RADIUM_RESOURCE_BUNDLE_HELPER_H_

#include <string>

class RadiumFeatureListCreator;

// Loads the local state, and returns the application locale. An empty return
// value indicates the ResouceBundle couldn't be loaded.
std::string LoadLocalState(
    RadiumFeatureListCreator* radium_feature_list_creator);

#endif  // RADIUM_BROWSER_RADIUM_RESOURCE_BUNDLE_HELPER_H_
