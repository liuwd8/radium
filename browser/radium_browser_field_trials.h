// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_FIELD_TRIALS_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_FIELD_TRIALS_H_

#include "components/variations/platform_field_trials.h"

class PrefService;

class RadiumBrowserFieldTrials : public variations::PlatformFieldTrials {
 public:
  explicit RadiumBrowserFieldTrials(PrefService* local_state);
  RadiumBrowserFieldTrials(const RadiumBrowserFieldTrials&) = delete;
  RadiumBrowserFieldTrials& operator=(const RadiumBrowserFieldTrials&) = delete;

  ~RadiumBrowserFieldTrials();

 private:
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_FIELD_TRIALS_H_
