// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/install_static/initialize_from_primary_module.h"

#include "radium/install_static/install_details.h"

// A function exported by the primary module, which is expected to be built with
// the "primary_module" source set.
extern "C" const install_static::InstallDetails::Payload __declspec(dllimport) *
    GetInstallDetailsPayload();

namespace install_static {

void InitializeFromPrimaryModule() {
  InstallDetails::InitializeFromPayload(GetInstallDetailsPayload());
}

}  // namespace install_static
