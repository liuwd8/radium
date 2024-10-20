// Copyright 2024 The Chromium Wechat Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/browser_process.h"

#include "base/sequence_checker.h"
#include "components/keep_alive_registry/keep_alive_registry.h"

namespace {
BrowserProcess* g_browser_process = nullptr;
}

BrowserProcess* BrowserProcess::Get() {
  return g_browser_process;
}

BrowserProcess::BrowserProcess() {
  g_browser_process = this;
}

BrowserProcess::~BrowserProcess() {
  g_browser_process = nullptr;
}

void BrowserProcess::Init() {
#if !BUILDFLAG(IS_ANDROID)
  KeepAliveRegistry::GetInstance()->SetIsShuttingDown(false);
  KeepAliveRegistry::GetInstance()->AddObserver(this);
#endif  // !BUILDFLAG(IS_ANDROID)
}

#if !BUILDFLAG(IS_ANDROID)
void BrowserProcess::SetQuitClosure(base::OnceClosure quit_closure) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(quit_closure);
  DCHECK(!quit_closure_);
  quit_closure_ = std::move(quit_closure);
}
#endif

GpuModeManager* BrowserProcess::gpu_mode_manager() {
  // TODO
  return nullptr;
}

void BrowserProcess::OnKeepAliveStateChanged(bool is_keeping_alive) {
  if (is_keeping_alive) {
    Pin();
  } else {
    Unpin();
  }
}

void BrowserProcess::OnKeepAliveRestartStateChanged(bool can_restart) {}

void BrowserProcess::Pin() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  // CHECK(!IsShuttingDown());
}

void BrowserProcess::Unpin() {
#if !BUILDFLAG(IS_ANDROID)
  std::move(quit_closure_).Run();
#endif  // !BUILDFLAG(IS_ANDROID)
}
