// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_RADIUM_CONSTANTS_H_
#define RADIUM_COMMON_RADIUM_CONSTANTS_H_

#include "base/files/file_path.h"
#include "build/build_config.h"

namespace radium {

extern const base::FilePath::CharType kCacheDirname[];
extern const base::FilePath::CharType kCookieFilename[];
extern const base::FilePath::CharType kGCMStoreDirname[];
extern const base::FilePath::CharType kInitialProfile[];
extern const base::FilePath::CharType kLocalStateFilename[];
extern const base::FilePath::CharType kMediaCacheDirname[];
extern const base::FilePath::CharType kMultiProfileDirPrefix[];
extern const base::FilePath::CharType kNetworkDataDirname[];
extern const base::FilePath::CharType kNetworkPersistentStateFilename[];
extern const base::FilePath::CharType kSCTAuditingPendingReportsFileName[];
extern const base::FilePath::CharType kSystemProfileDir[];
extern const base::FilePath::CharType kTransportSecurityPersisterFilename[];
extern const base::FilePath::CharType kTrustTokenFilename[];
extern const base::FilePath::CharType kReportingAndNelStoreFilename[];

}  // namespace radium

#endif  // RADIUM_COMMON_RADIUM_CONSTANTS_H_
