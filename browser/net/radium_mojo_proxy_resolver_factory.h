// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_NET_RADIUM_MOJO_PROXY_RESOLVER_FACTORY_H_
#define RADIUM_BROWSER_NET_RADIUM_MOJO_PROXY_RESOLVER_FACTORY_H_

#include <memory>
#include <string>

#include "base/sequence_checker.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "services/proxy_resolver/public/mojom/proxy_resolver.mojom.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/mojom/connector.mojom.h"

// ProxyResolverFactory that acts as a proxy to the proxy resolver service.
// Starts the service as needed, and maintains no active mojo pipes to it,
// so that it's automatically shut down as needed.
//
// ChromeMojoProxyResolverFactories must be created and used only on the UI
// thread.
class RadiumMojoProxyResolverFactory
    : public proxy_resolver::mojom::ProxyResolverFactory {
 public:
  RadiumMojoProxyResolverFactory();

  RadiumMojoProxyResolverFactory(const RadiumMojoProxyResolverFactory&) =
      delete;
  RadiumMojoProxyResolverFactory& operator=(
      const RadiumMojoProxyResolverFactory&) = delete;

  ~RadiumMojoProxyResolverFactory() override;

  // Convenience method that creates a self-owned ProxyResolverFactory and
  // returns a remote endpoint to control it.
  static mojo::PendingRemote<proxy_resolver::mojom::ProxyResolverFactory>
  CreateWithSelfOwnedReceiver();

  // proxy_resolver::mojom::ProxyResolverFactory implementation:
  void CreateResolver(
      const std::string& pac_script,
      mojo::PendingReceiver<proxy_resolver::mojom::ProxyResolver> receiver,
      mojo::PendingRemote<
          proxy_resolver::mojom::ProxyResolverFactoryRequestClient> client)
      override;

 private:
  std::unique_ptr<service_manager::Connector> service_manager_connector_;
  SEQUENCE_CHECKER(sequence_checker_);
};

#endif  // RADIUM_BROWSER_NET_RADIUM_MOJO_PROXY_RESOLVER_FACTORY_H_
