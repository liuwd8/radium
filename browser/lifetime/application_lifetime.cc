// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/lifetime/application_lifetime.h"

#include "radium/browser/browser_process.h"
#include "radium/browser/browser_process_platform_part.h"

#if !BUILDFLAG(IS_ANDROID)
#include "radium/browser/lifetime/application_lifetime_desktop.h"
#endif  // !BUILDFLAG(IS_ANDROID)

namespace radium {

namespace {

void AttemptExitInternal(bool try_to_quit_application) {
#if !BUILDFLAG(IS_ANDROID)
  OnClosingAllBrowsers(true);
#endif  // !BUILDFLAG(IS_ANDROID)
  BrowserProcess::Get()->platform_part()->AttemptExit(try_to_quit_application);
}

}  // namespace

void AttemptExit() {
  // If we know that all browsers can be closed without blocking,
  // don't notify users of crashes beyond this point.
  // Note that MarkAsCleanShutdown() does not set UMA's exit cleanly bit
  // so crashes during shutdown are still reported in UMA.
#if !BUILDFLAG(IS_ANDROID)
  // Android doesn't use Browser.
  if (AreAllBrowsersCloseable()) {
    MarkAsCleanShutdown();
  }
#endif  // !BUILDFLAG(IS_ANDROID)
  AttemptExitInternal(true);
}

}  // namespace radium
