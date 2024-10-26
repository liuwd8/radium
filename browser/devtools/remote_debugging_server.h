// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_
#define RADIUM_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_

class RemoteDebuggingServer {
 public:
  explicit RemoteDebuggingServer();
  RemoteDebuggingServer(const RemoteDebuggingServer&) = delete;
  RemoteDebuggingServer& operator=(const RemoteDebuggingServer&) = delete;

  ~RemoteDebuggingServer();
};

#endif  // RADIUM_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_
