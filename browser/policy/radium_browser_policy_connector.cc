// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/policy/radium_browser_policy_connector.h"

#include "base/version_info/channel.h"
#include "radium/browser/policy/configuration_policy_handler_list_factory.h"
#include "radium/common/channel_info.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace policy {

RadiumBrowserPolicyConnector::RadiumBrowserPolicyConnector()
    : BrowserPolicyConnector(base::BindRepeating(&BuildHandlerList)) {}

RadiumBrowserPolicyConnector::~RadiumBrowserPolicyConnector() = default;

void RadiumBrowserPolicyConnector::Init(
    PrefService* local_state,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory) {}

bool RadiumBrowserPolicyConnector::IsDeviceEnterpriseManaged() const {
  return false;
}

bool RadiumBrowserPolicyConnector::HasMachineLevelPolicies() {
  return false;
}

bool RadiumBrowserPolicyConnector::IsCommandLineSwitchSupported() const {
  version_info::Channel channel = radium::GetChannel();
  return channel != version_info::Channel::STABLE &&
         channel != version_info::Channel::BETA;
}

}  // namespace policy
