// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_POLICY_RADIUM_BROWSER_POLICY_CONNECTOR_H_
#define RADIUM_BROWSER_POLICY_RADIUM_BROWSER_POLICY_CONNECTOR_H_

#include "components/policy/core/browser/browser_policy_connector.h"

namespace policy {

class RadiumBrowserPolicyConnector : public BrowserPolicyConnector {
 public:
  explicit RadiumBrowserPolicyConnector();
  RadiumBrowserPolicyConnector(const RadiumBrowserPolicyConnector&) = delete;
  RadiumBrowserPolicyConnector& operator=(const RadiumBrowserPolicyConnector&) =
      delete;

  ~RadiumBrowserPolicyConnector() override;

  void Init(PrefService* local_state,
            scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
      override;

  bool IsDeviceEnterpriseManaged() const override;

  bool HasMachineLevelPolicies() override;

  // BrowserPolicyConnector:
  // Command line switch only supports Dev and Canary channel, trunk and
  // browser tests on Win, Mac, Linux and Android.
  bool IsCommandLineSwitchSupported() const override;
};

}  // namespace policy

#endif  // RADIUM_BROWSER_POLICY_RADIUM_BROWSER_POLICY_CONNECTOR_H_
