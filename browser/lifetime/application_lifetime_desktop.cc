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
#include "chrome/browser/lifetime/termination_notification.h"
#include "components/keep_alive_registry/keep_alive_registry.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/lifetime/browser_close_manager.h"
#include "radium/browser/lifetime/browser_shutdown.h"
#include "radium/browser/ui/browser.h"
#include "radium/browser/ui/browser_finder.h"
#include "radium/browser/ui/browser_list.h"
#include "ui/views/widget/widget.h"

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

void ShutdownIfNoBrowsers() {
  if (GetTotalBrowserCount() > 0) {
    return;
  }

  // Tell everyone that we are shutting down.
  browser_shutdown::SetTryingToQuit(true);

  browser_shutdown::NotifyAppTerminating();
  OnAppExiting();
}

}  // namespace

void CloseAllBrowsers() {
  // If there are no browsers and closing the last browser would quit the
  // application, send the APP_TERMINATING action here. Otherwise, it will be
  // sent by RemoveBrowser() when the last browser has closed.
  if (GetTotalBrowserCount() == 0) {
    ShutdownIfNoBrowsers();
    return;
  }

  scoped_refptr<BrowserCloseManager> browser_close_manager =
      new BrowserCloseManager;
  browser_close_manager->StartClosingBrowsers();
}

void ShutdownIfNeeded() {
  if (browser_shutdown::IsTryingToQuit()) {
    return;
  }

  ShutdownIfNoBrowsers();
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

void OnAppExiting() {
  static bool notified = false;
  if (notified) {
    return;
  }
  notified = true;
  HandleAppExitingForPlatform();
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
  if (BrowserList::GetInstance()->empty()) {
    return true;
  }

  // Check TabsNeedBeforeUnloadFired().
  for (Browser* browser : *BrowserList::GetInstance()) {
    if (browser->TabsNeedBeforeUnloadFired()) {
      return false;
    }
  }
  return true;
}

}  // namespace radium
