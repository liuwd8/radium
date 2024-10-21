// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/radium_features.h"

namespace features {

// When kNoReferrers is enabled, most HTTP requests will provide empty
// referrers instead of their ordinary behavior.
BASE_FEATURE(kNoReferrers, "NoReferrers", base::FEATURE_DISABLED_BY_DEFAULT);

}  // namespace features
