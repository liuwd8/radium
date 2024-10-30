// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/install_static/install_modes.h"

namespace install_static {

namespace {

std::wstring GetUnregisteredKeyPathForProduct() {
  return std::wstring(L"Software\\").append(kProductPathName);
}

}  // namespace

std::wstring GetClientsKeyPath(const wchar_t* app_guid) {
  return GetUnregisteredKeyPathForProduct();
}

std::wstring GetClientStateKeyPath(const wchar_t* app_guid) {
  return GetUnregisteredKeyPathForProduct();
}

std::wstring GetClientStateMediumKeyPath(const wchar_t* app_guid) {
  return GetUnregisteredKeyPathForProduct();
}

}  // namespace install_static
