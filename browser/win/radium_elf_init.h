// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_WIN_RADIUM_ELF_INIT_H_
#define RADIUM_BROWSER_WIN_RADIUM_ELF_INIT_H_

// Field trial name and full name for the blacklist disabled group.
inline constexpr char kBrowserBlocklistTrialName[] = "BrowserBlocklist";
inline constexpr char kBrowserBlocklistTrialDisabledGroupName[] = "NoBlocklist";

// Prepare any initialization code for Chrome Elf's setup (This will generally
// only affect future runs since Chrome Elf is already setup by this point).
void InitializeRadiumElf();

// Set the required state for an enabled browser blacklist.
void BrowserBlocklistBeaconSetup();

#endif  // RADIUM_BROWSER_WIN_RADIUM_ELF_INIT_H_
