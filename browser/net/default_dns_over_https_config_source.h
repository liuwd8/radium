// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_NET_DEFAULT_DNS_OVER_HTTPS_CONFIG_SOURCE_H_
#define RADIUM_BROWSER_NET_DEFAULT_DNS_OVER_HTTPS_CONFIG_SOURCE_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "components/prefs/pref_change_registrar.h"
#include "radium/browser/net/dns_over_https_config_source.h"

class PrefRegistrySimple;
class PrefService;

// Default, cross-platfrom implementation of DnsOverHttpsConfigSource that
// listens to local_state prefs to determine the DNS-over-HTTPS configuration.
class DefaultDnsOverHttpsConfigSource : public DnsOverHttpsConfigSource {
 public:
  DefaultDnsOverHttpsConfigSource(PrefService* local_state,
                                  bool set_up_pref_defaults);

  DefaultDnsOverHttpsConfigSource(const DefaultDnsOverHttpsConfigSource&) =
      delete;
  DefaultDnsOverHttpsConfigSource& operator=(
      const DefaultDnsOverHttpsConfigSource&) = delete;
  ~DefaultDnsOverHttpsConfigSource() override;

  static void RegisterPrefs(PrefRegistrySimple* registry);

  // DnsOverHttpsConfigSource:
  std::string GetDnsOverHttpsMode() const override;
  std::string GetDnsOverHttpsTemplates() const override;
  bool IsConfigManaged() const override;
  void SetDohChangeCallback(base::RepeatingClosure callback) override;

 private:
  PrefChangeRegistrar pref_change_registrar_;
};

#endif  // RADIUM_BROWSER_NET_DEFAULT_DNS_OVER_HTTPS_CONFIG_SOURCE_H_
