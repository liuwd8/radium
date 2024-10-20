// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_RADIUM_CONSTANTS_H_
#define RADIUM_COMMON_RADIUM_CONSTANTS_H_

#include "base/files/file_path.h"
#include "build/build_config.h"

namespace radium {

extern const base::FilePath::CharType kGCMStoreDirname[];
extern const base::FilePath::CharType kLocalStateFilename[];

}  // namespace radium

#endif  // RADIUM_COMMON_RADIUM_CONSTANTS_H_
