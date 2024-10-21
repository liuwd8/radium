// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_POLICY_CONFIGURATION_POLICY_HANDLER_LIST_FACTORY_H_
#define RADIUM_BROWSER_POLICY_CONFIGURATION_POLICY_HANDLER_LIST_FACTORY_H_

#include <memory>

namespace policy {

class ConfigurationPolicyHandlerList;
class Schema;

// Builds a platform-specific handler list.
std::unique_ptr<ConfigurationPolicyHandlerList> BuildHandlerList(
    const Schema& chrome_schema);

}  // namespace policy

#endif  // RADIUM_BROWSER_POLICY_CONFIGURATION_POLICY_HANDLER_LIST_FACTORY_H_
