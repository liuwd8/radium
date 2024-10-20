// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/radium_constants.h"

#define FPL FILE_PATH_LITERAL

namespace radium {

const base::FilePath::CharType kGCMStoreDirname[] = FPL("GCM Store");
const base::FilePath::CharType kLocalStateFilename[] = FPL("Local State");

}  // namespace radium

#undef FPL
