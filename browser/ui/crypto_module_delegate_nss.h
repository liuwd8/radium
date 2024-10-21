// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_CRYPTO_MODULE_DELEGATE_NSS_H_
#define RADIUM_BROWSER_UI_CRYPTO_MODULE_DELEGATE_NSS_H_

#include <string>

#include "base/synchronization/waitable_event.h"
#include "crypto/nss_crypto_module_delegate.h"
#include "net/base/host_port_pair.h"

// An enum to describe the reason for the password request.
enum CryptoModulePasswordReason {
  kCryptoModulePasswordCertEnrollment,
  kCryptoModulePasswordClientAuth,
  kCryptoModulePasswordListCerts,
  kCryptoModulePasswordCertImport,
  kCryptoModulePasswordCertExport,
};

// Create a delegate which only handles unlocking slots.
crypto::CryptoModuleBlockingPasswordDelegate*
CreateCryptoModuleBlockingPasswordDelegate(CryptoModulePasswordReason reason,
                                           const net::HostPortPair& server);

#endif  // RADIUM_BROWSER_UI_CRYPTO_MODULE_DELEGATE_NSS_H_
