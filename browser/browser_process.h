// Copyright 2024 The Chromium Wechat Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_BROWSER_PROCESS_H_
#define RADIUM_BROWSER_BROWSER_PROCESS_H_

#include "base/functional/callback.h"
#include "base/functional/callback_forward.h"
#include "base/sequence_checker.h"
#include "components/keep_alive_registry/keep_alive_state_observer.h"

class GpuModeManager;

class BrowserProcess : public KeepAliveStateObserver {
 public:
  static BrowserProcess* Get();

  explicit BrowserProcess();
  BrowserProcess(const BrowserProcess&) = delete;
  BrowserProcess& operator=(const BrowserProcess&) = delete;

  ~BrowserProcess() override;

  // Called to complete initialization.
  void Init();

#if !BUILDFLAG(IS_ANDROID)
  // Sets a closure to be run to break out of a run loop on browser shutdown
  // (when the KeepAlive count reaches zero).
  void SetQuitClosure(base::OnceClosure quit_closure);
#endif

  GpuModeManager* gpu_mode_manager();

 private:
  // KeepAliveStateObserver:
  void OnKeepAliveStateChanged(bool is_keeping_alive) override;
  void OnKeepAliveRestartStateChanged(bool can_restart) override;

  // Methods called to control our lifetime. The browser process can be "pinned"
  // to make sure it keeps running.
  void Pin();
  void Unpin();

#if !BUILDFLAG(IS_ANDROID)
  // Called to signal the process' main message loop to exit.
  base::OnceClosure quit_closure_;
#endif

  SEQUENCE_CHECKER(sequence_checker_);
};

#endif  // RADIUM_BROWSER_BROWSER_PROCESS_H_
