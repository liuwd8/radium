// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_INSTALL_STATIC_INITIALIZE_FROM_PRIMARY_MODULE_H_
#define RADIUM_INSTALL_STATIC_INITIALIZE_FROM_PRIMARY_MODULE_H_

namespace install_static {

// Initializes an InstallDetails instance for this module with the payload from
// the process's primary module (which exports an "GetInstallDetailsPayload"
// function for this express purpose).
void InitializeFromPrimaryModule();

}  // namespace install_static

#endif  // RADIUM_INSTALL_STATIC_INITIALIZE_FROM_PRIMARY_MODULE_H_
