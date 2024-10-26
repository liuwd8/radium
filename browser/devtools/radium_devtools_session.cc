// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/devtools/radium_devtools_session.h"

#include "content/public/browser/devtools_agent_host_client_channel.h"

RadiumDevToolsSession::RadiumDevToolsSession(
    content::DevToolsAgentHostClientChannel* channel)
    : dispatcher_(this), client_channel_(channel) {}

RadiumDevToolsSession::~RadiumDevToolsSession() = default;

void RadiumDevToolsSession::HandleCommand(
    base::span<const uint8_t> message,
    content::DevToolsManagerDelegate::NotHandledCallback callback) {
  crdtp::Dispatchable dispatchable(crdtp::SpanFrom(message));
  DCHECK(dispatchable.ok());  // Checked by content::DevToolsSession.
  crdtp::UberDispatcher::DispatchResult dispatched =
      dispatcher_.Dispatch(dispatchable);

  // auto command_uma_id = GetCommandUmaId(std::string_view(
  //     reinterpret_cast<const char*>(dispatchable.Method().begin()),
  //     dispatchable.Method().size()));
  // std::string client_type =
  // client_channel_->GetClient()->GetTypeForMetrics(); DCHECK(client_type ==
  // "DevTools" || client_type == "Extension" ||
  //        client_type == "RemoteDebugger" || client_type == "Other");
  // base::UmaHistogramSparse("DevTools.CDPCommandFrom" + client_type,
  //                          command_uma_id);

  if (!dispatched.MethodFound()) {
    std::move(callback).Run(message);
    return;
  }
  pending_commands_[dispatchable.CallId()] = std::move(callback);
  dispatched.Run();
}

void RadiumDevToolsSession::SendProtocolResponse(
    int call_id,
    std::unique_ptr<protocol::Serializable> message) {
  pending_commands_.erase(call_id);
  client_channel_->DispatchProtocolMessageToClient(message->Serialize());
}

void RadiumDevToolsSession::SendProtocolNotification(
    std::unique_ptr<protocol::Serializable> message) {
  client_channel_->DispatchProtocolMessageToClient(message->Serialize());
}

void RadiumDevToolsSession::FlushProtocolNotifications() {}

void RadiumDevToolsSession::FallThrough(int call_id,
                                        crdtp::span<uint8_t> method,
                                        crdtp::span<uint8_t> message) {
  auto callback = std::move(pending_commands_[call_id]);
  pending_commands_.erase(call_id);
  std::move(callback).Run(message);
}
