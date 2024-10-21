// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/policy/configuration_policy_handler_list_factory.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/version_info/channel.h"
#include "components/policy/core/browser/configuration_policy_handler.h"
#include "components/policy/core/browser/configuration_policy_handler_list.h"
#include "components/policy/core/browser/configuration_policy_handler_parameters.h"
#include "components/policy/policy_constants.h"
#include "radium/common/channel_info.h"

namespace policy {
namespace {
// Future policies are not supported on Stable and Beta by default.
bool AreFuturePoliciesEnabledByDefault() {
  version_info::Channel channel = radium::GetChannel();
  return channel != version_info::Channel::STABLE &&
         channel != version_info::Channel::BETA;
}

void PopulatePolicyHandlerParameters(PolicyHandlerParameters* parameters) {}
}  // namespace

std::unique_ptr<ConfigurationPolicyHandlerList> BuildHandlerList(
    const Schema& chrome_schema) {
  std::unique_ptr<ConfigurationPolicyHandlerList> handlers(
      new ConfigurationPolicyHandlerList(
          base::BindRepeating(&PopulatePolicyHandlerParameters),
          base::BindRepeating(&GetChromePolicyDetails),
          AreFuturePoliciesEnabledByDefault()));

  return handlers;
}

}  // namespace policy
