// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_DEVTOOLS_RADIUM_DEVTOOLS_SESSION_H_
#define RADIUM_BROWSER_DEVTOOLS_RADIUM_DEVTOOLS_SESSION_H_

#include "content/public/browser/devtools_manager_delegate.h"
#include "radium/browser/devtools/protocol/protocol.h"

namespace content {
class DevToolsAgentHostClientChannel;
}  // namespace content

class RadiumDevToolsSession : public protocol::FrontendChannel {
 public:
  explicit RadiumDevToolsSession(
      content::DevToolsAgentHostClientChannel* channel);
  RadiumDevToolsSession(const RadiumDevToolsSession&) = delete;
  RadiumDevToolsSession& operator=(const RadiumDevToolsSession&) = delete;

  ~RadiumDevToolsSession() override;

  void HandleCommand(
      base::span<const uint8_t> message,
      content::DevToolsManagerDelegate::NotHandledCallback callback);

 private:
  // protocol::FrontendChannel:
  void SendProtocolResponse(
      int call_id,
      std::unique_ptr<protocol::Serializable> message) override;
  void SendProtocolNotification(
      std::unique_ptr<protocol::Serializable> message) override;
  void FlushProtocolNotifications() override;
  void FallThrough(int call_id,
                   crdtp::span<uint8_t> method,
                   crdtp::span<uint8_t> message) override;

 private:
  base::flat_map<int, content::DevToolsManagerDelegate::NotHandledCallback>
      pending_commands_;

  protocol::UberDispatcher dispatcher_;
  raw_ptr<content::DevToolsAgentHostClientChannel> client_channel_;
};

#endif  // RADIUM_BROWSER_DEVTOOLS_RADIUM_DEVTOOLS_SESSION_H_
