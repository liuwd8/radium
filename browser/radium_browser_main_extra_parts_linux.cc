// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/environment.h"
#include "build/build_config.h"
#include "radium/browser/radium_browser_main_extra_parts_linux.h"

void RadiumBrowserMainExtraPartsLinux::InitOzonePlatformHint() {
#if BUILDFLAG(IS_LINUX)
  auto* const command_line = base::CommandLine::ForCurrentProcess();
  auto env = base::Environment::Create();
  std::string desktop_startup_id;
  if (env->GetVar("DESKTOP_STARTUP_ID", &desktop_startup_id)) {
    command_line->AppendSwitchASCII("desktop-startup-id", desktop_startup_id);
  }
#endif  // BUILDFLAG(IS_LINUX)
}

RadiumBrowserMainExtraPartsLinux::RadiumBrowserMainExtraPartsLinux() = default;
RadiumBrowserMainExtraPartsLinux::~RadiumBrowserMainExtraPartsLinux() = default;
