// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/lifetime/application_lifetime_desktop.h"

#include <optional>

#include "base/callback_list.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/process/process.h"
#include "base/threading/hang_watcher.h"
#include "base/time/time.h"
#include "base/types/strong_alias.h"
#include "build/build_config.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/lifetime/browser_shutdown.h"

#if BUILDFLAG(IS_WIN)
#include "base/win/win_util.h"
#endif

namespace radium {

namespace {

base::RepeatingCallbackList<void(bool)>& GetClosingAllBrowsersCallbackList() {
  static base::NoDestructor<base::RepeatingCallbackList<void(bool)>>
      callback_list;
  return *callback_list;
}

}  // namespace

void CloseAllBrowsers() {
  CloseAllTopWidgetForPlatform();
}

void ShutdownIfNeeded() {
  HandleAppExitingForPlatform();
}

void SessionEnding() {
  // This is a time-limited shutdown where we need to write as much to
  // disk as we can as soon as we can, and where we must kill the
  // process within a hang timeout to avoid user prompts.

  // EndSession is invoked once per frame. Only do something the first time.
  static bool already_ended = false;
  // We may get called in the middle of shutdown, e.g. https://crbug.com/70852
  // and https://crbug.com/1187418.  In this case, do nothing.
  if (already_ended || !BrowserProcess::Get()) {
    return;
  }
  already_ended = true;

  std::optional<base::WatchHangsInScope> watch_hangs_scope;
  if (base::HangWatcher::IsCrashReportingEnabled()) {
    // TODO(crbug.com/40840897): Migrate away from ShutdownWatcher and its old
    // timing.
    base::HangWatcher::SetShuttingDown();
    constexpr base::TimeDelta kShutdownHangDelay{base::Seconds(30)};
    watch_hangs_scope.emplace(kShutdownHangDelay);
  }

  browser_shutdown::OnShutdownStarting(
      browser_shutdown::ShutdownType::kEndSession);

  OnClosingAllBrowsers(true);

  // Write important data first.
  BrowserProcess::Get()->EndSession();

#if BUILDFLAG(IS_WIN)
  base::win::SetShouldCrashOnProcessDetach(false);
#endif  // BUILDFLAG(IS_WIN)

  // On Windows 7 and later, the system will consider the process ripe for
  // termination as soon as it hides or destroys its windows. Since any
  // execution past that point will be non-deterministically cut short, we
  // might as well put ourselves out of that misery deterministically.
  base::Process::TerminateCurrentProcessImmediately(0);
}

void OnClosingAllBrowsers(bool closing) {
  GetClosingAllBrowsersCallbackList().Notify(closing);
}

base::CallbackListSubscription AddClosingAllBrowsersCallback(
    base::RepeatingCallback<void(bool)> closing_all_browsers_callback) {
  return GetClosingAllBrowsersCallbackList().Add(
      std::move(closing_all_browsers_callback));
}

void MarkAsCleanShutdown() {}

bool AreAllBrowsersCloseable() {
  return true;
}

}  // namespace radium
