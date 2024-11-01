// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/radium_constants.h"

#include "radium/common/radium_version.h"

#define FPL FILE_PATH_LITERAL

namespace radium {

const char kRadiumVersion[] = RADIUM_VERSION_STRING;

// directory names
#if BUILDFLAG(IS_WIN)
extern const wchar_t kUserDataDirname[];
#endif

const base::FilePath::CharType kCacheDirname[] = FPL("Cache");
const base::FilePath::CharType kCookieFilename[] = FPL("Cookies");

#if BUILDFLAG(IS_MAC)
const base::FilePath::CharType kBrowserProcessExecutableName[] =
    FPL(PRODUCT_FULLNAME_STRING);
const base::FilePath::CharType kHelperProcessExecutableName[] =
    FPL(PRODUCT_FULLNAME_STRING " Helper");
const base::FilePath::CharType kBrowserProcessExecutablePath[] =
    FPL(PRODUCT_FULLNAME_STRING ".app/Contents/MacOS/" PRODUCT_FULLNAME_STRING);
const base::FilePath::CharType kHelperProcessExecutablePath[] =
    FPL(PRODUCT_FULLNAME_STRING
        " Helper.app/Contents/MacOS/" PRODUCT_FULLNAME_STRING " Helper");
const base::FilePath::CharType kFrameworkName[] =
    FPL(PRODUCT_FULLNAME_STRING " Framework.framework");
const char kMacHelperSuffixAlerts[] = " (Alerts)";
#endif  // BUILDFLAG(IS_MAC)

const base::FilePath::CharType kGCMStoreDirname[] = FPL("GCM Store");
const base::FilePath::CharType kInitialProfile[] = FPL("Default");
const base::FilePath::CharType kLocalStateFilename[] = FPL("Local State");
const base::FilePath::CharType kMediaCacheDirname[] = FPL("Media Cache");
const base::FilePath::CharType kMultiProfileDirPrefix[] = FPL("Profile ");
const base::FilePath::CharType kNetworkDataDirname[] = FPL("Network");
const base::FilePath::CharType kNetworkPersistentStateFilename[] =
    FPL("Network Persistent State");
const base::FilePath::CharType kSCTAuditingPendingReportsFileName[] =
    FPL("SCT Auditing Pending Reports");
const base::FilePath::CharType kSystemProfileDir[] = FPL("System Profile");
const base::FilePath::CharType kTransportSecurityPersisterFilename[] =
    FPL("TransportSecurity");
const base::FilePath::CharType kTrustTokenFilename[] = FPL("Trust Tokens");
// Only use if the ENABLE_REPORTING build flag is true
const base::FilePath::CharType kReportingAndNelStoreFilename[] =
    FPL("Reporting and NEL");

}  // namespace radium

#undef FPL
