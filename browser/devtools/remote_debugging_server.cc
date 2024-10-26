// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/devtools/remote_debugging_server.h"

#include "base/command_line.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_socket_factory.h"
#include "content/public/common/content_switches.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/global_features.h"

namespace {

const int kBackLog = 10;

class TCPServerSocketFactory : public content::DevToolsSocketFactory {
 public:
  explicit TCPServerSocketFactory(uint16_t port) : port_(port) {}

  TCPServerSocketFactory(const TCPServerSocketFactory&) = delete;
  TCPServerSocketFactory& operator=(const TCPServerSocketFactory&) = delete;

 private:
  std::unique_ptr<net::ServerSocket> CreateLocalHostServerSocket(int port) {
    std::unique_ptr<net::ServerSocket> socket(
        std::make_unique<net::TCPServerSocket>(nullptr, net::NetLogSource()));
    if (socket->ListenWithAddressAndPort("127.0.0.1", port, kBackLog) ==
        net::OK) {
      return socket;
    }
    if (socket->ListenWithAddressAndPort("::1", port, kBackLog) == net::OK) {
      return socket;
    }
    return nullptr;
  }

  // content::DevToolsSocketFactory.
  std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
    return CreateLocalHostServerSocket(port_);
  }

  std::unique_ptr<net::ServerSocket> CreateForTethering(
      std::string* name) override {
    return nullptr;
  }

  uint16_t port_;
};

}  // namespace

RemoteDebuggingServer::RemoteDebuggingServer() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  std::string port_str =
      command_line.GetSwitchValueASCII(::switches::kRemoteDebuggingPort);
  int port;
  if (!base::StringToInt(port_str, &port) || port < 0 || port >= 65535) {
    return;
  }

  content::DevToolsAgentHost::StartRemoteDebuggingServer(
      std::make_unique<TCPServerSocketFactory>(port), base::FilePath(),
      base::FilePath());
}

RemoteDebuggingServer::~RemoteDebuggingServer() {
  // Ensure Profile is alive, because the whole DevTools subsystem
  // accesses it during shutdown.
  DCHECK(BrowserProcess::Get()->GetFeatures()->profile_manager());
  content::DevToolsAgentHost::StopRemoteDebuggingServer();
  content::DevToolsAgentHost::StopRemoteDebuggingPipeHandler();
}
