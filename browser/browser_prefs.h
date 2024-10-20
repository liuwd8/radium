// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_BROWSER_PREFS_H_
#define RADIUM_BROWSER_BROWSER_PREFS_H_

class PrefRegistrySimple;
class PrefService;

// Register all prefs that will be used via the local state PrefService.
void RegisterLocalState(PrefRegistrySimple* registry);

#endif  // RADIUM_BROWSER_BROWSER_PREFS_H_
