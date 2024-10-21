// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/crypto_module_delegate_nss.h"

crypto::CryptoModuleBlockingPasswordDelegate*
CreateCryptoModuleBlockingPasswordDelegate(CryptoModulePasswordReason reason,
                                           const net::HostPortPair& server) {
  return nullptr;
}
