// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_INSTALL_STATIC_CHROMIUM_INSTALL_MODES_H_
#define RADIUM_INSTALL_STATIC_CHROMIUM_INSTALL_MODES_H_

namespace install_static {

// Note: This list of indices must be kept in sync with the brand-specific
// resource strings in chrome/installer/util/prebuild/create_string_rc.
enum InstallConstantIndex {
  CHROMIUM_INDEX,
  NUM_INSTALL_MODES,
};

}  // namespace install_static

#endif  // RADIUM_INSTALL_STATIC_CHROMIUM_INSTALL_MODES_H_
