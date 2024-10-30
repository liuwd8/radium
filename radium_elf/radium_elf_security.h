// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_RADIUM_ELF_RADIUM_ELF_SECURITY_H_
#define RADIUM_RADIUM_ELF_RADIUM_ELF_SECURITY_H_

namespace elf_security {

// Setup any early browser-process security.
void EarlyBrowserSecurity();

// Returns whether we set the Extension Point Disable mitigation during early
// browser security.
bool IsExtensionPointDisableSet();

// Turns on or off the validate function for testing purposes.
void ValidateExeForTesting(bool on);

}  // namespace elf_security

#endif  // RADIUM_RADIUM_ELF_RADIUM_ELF_SECURITY_H_
