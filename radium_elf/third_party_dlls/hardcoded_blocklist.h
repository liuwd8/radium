// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_RADIUM_ELF_THIRD_PARTY_DLLS_HARDCODED_BLOCKLIST_H_
#define RADIUM_RADIUM_ELF_THIRD_PARTY_DLLS_HARDCODED_BLOCKLIST_H_

#include <string>

namespace third_party_dlls {

// Returns true if a matching name is found in the hard-coded blocklist.
// Note: |module_name| must be an ASCII encoded string.
bool DllMatch(const std::string& module_name);

}  // namespace third_party_dlls

#endif  // RADIUM_RADIUM_ELF_THIRD_PARTY_DLLS_HARDCODED_BLOCKLIST_H_
