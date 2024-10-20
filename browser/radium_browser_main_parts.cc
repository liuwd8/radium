// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts.h"

#include "base/trace_event/trace_event.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/radium_browser_main_extra_parts.h"

RadiumBrowserMainParts::RadiumBrowserMainParts() = default;

RadiumBrowserMainParts::~RadiumBrowserMainParts() {
  // Delete parts in the reverse of the order they were added.
  while (!radium_extra_parts_.empty()) {
    radium_extra_parts_.pop_back();
  }
}

void RadiumBrowserMainParts::AddParts(
    std::unique_ptr<RadiumBrowserMainExtraParts> parts) {
  radium_extra_parts_.push_back(std::move(parts));
}

int RadiumBrowserMainParts::PreMainMessageLoopRun() {
  TRACE_EVENT("startup", "RadiumBrowserMainParts::PreMainMessageLoopRun");

  result_code_ = PreMainMessageLoopRunImpl();

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PreMainMessageLoopRun();
  }

  return result_code_;
}

void RadiumBrowserMainParts::ToolkitInitialized() {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::ToolkitInitialized");

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->ToolkitInitialized();
  }

  // // Comes after the extra parts' calls since on GTK that builds the native
  // // theme that, in turn, adds the GTK core color mixer; core mixers should
  // all
  // // be added before we add chrome mixers.
  // ui::ColorProviderManager::Get().AppendColorProviderInitializer(
  //     base::BindRepeating(color::AddComponentsColorMixers));
  // ui::ColorProviderManager::Get().AppendColorProviderInitializer(
  //     base::BindRepeating(AddChromeColorMixers));
}

void RadiumBrowserMainParts::PreCreateMainMessageLoop() {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::PreCreateMainMessageLoop");

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PreCreateMainMessageLoop();
  }
}

void RadiumBrowserMainParts::PostCreateMainMessageLoop() {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::PostCreateMainMessageLoop");

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PostCreateMainMessageLoop();
  }
}

int RadiumBrowserMainParts::PreCreateThreads() {
  // IMPORTANT
  // Calls in this function should not post tasks or create threads as
  // components used to handle those tasks are not yet available. This work
  // should be deferred to PreMainMessageLoopRunImpl.

  TRACE_EVENT0("startup", "RadiumBrowserMainParts::PreCreateThreads");
  result_code_ = PreCreateThreadsImpl();

  if (result_code_ == content::RESULT_CODE_NORMAL_EXIT) {
    for (auto& radium_extra_part : radium_extra_parts_) {
      radium_extra_part->PreCreateThreads();
    }
  }

  // Create an instance of GpuModeManager to watch gpu mode pref change.
  BrowserProcess::Get()->gpu_mode_manager();

  return result_code_;
}

int RadiumBrowserMainParts::PreCreateThreadsImpl() {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::PreCreateThreadsImpl");
  return content::RESULT_CODE_NORMAL_EXIT;
}

int RadiumBrowserMainParts::PreMainMessageLoopRunImpl() {
  return result_code_;
}
