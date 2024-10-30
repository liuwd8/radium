// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_NET_RADIUM_MOJO_PROXY_RESOLVER_WIN_H_
#define RADIUM_BROWSER_NET_RADIUM_MOJO_PROXY_RESOLVER_WIN_H_

#include "base/time/time.h"
#include "services/proxy_resolver_win/public/mojom/proxy_resolver_win.mojom.h"
#include "url/gurl.h"

// WindowsSystemProxyResolver that acts as a proxy to the proxy_resolver_win
// service. Starts the service as needed, and maintains no active mojo pipes to
// it, so that it's automatically shut down as needed.
//
// RadiumMojoProxyResolverWin must be created and used only on the UI thread.
class RadiumMojoProxyResolverWin
    : public proxy_resolver_win::mojom::WindowsSystemProxyResolver {
 public:
  explicit RadiumMojoProxyResolverWin(const base::TimeDelta& idle_timeout);
  RadiumMojoProxyResolverWin(const RadiumMojoProxyResolverWin&) = delete;
  RadiumMojoProxyResolverWin& operator=(const RadiumMojoProxyResolverWin&) =
      delete;
  ~RadiumMojoProxyResolverWin() override;

  // Convenience method that creates a self-owned WindowsSystemProxyResolver and
  // returns a remote endpoint to control it.
  static mojo::PendingRemote<WindowsSystemProxyResolver>
  CreateWithSelfOwnedReceiver();

  // The test version allows for setting a custom idle timeout.
  static mojo::PendingRemote<WindowsSystemProxyResolver>
  CreateWithSelfOwnedReceiverForTesting(const base::TimeDelta& idle_timeout);

  // proxy_resolver_win::mojom::WindowsSystemProxyResolver implementation:
  void GetProxyForUrl(const GURL& url,
                      GetProxyForUrlCallback callback) override;

 private:
  const base::TimeDelta idle_timeout_;
};

#endif  // RADIUM_BROWSER_NET_RADIUM_MOJO_PROXY_RESOLVER_WIN_H_
