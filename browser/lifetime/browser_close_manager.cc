// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/lifetime/browser_close_manager.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

#include "base/containers/contains.h"
#include "base/functional/bind.h"
#include "base/metrics/histogram_functions.h"
#include "base/notreached.h"
#include "build/build_config.h"
#include "content/public/browser/web_contents.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/lifetime/application_lifetime.h"
#include "radium/browser/lifetime/browser_shutdown.h"
#include "radium/browser/profiles/profile_manager.h"
#include "radium/browser/ui/browser.h"
#include "radium/browser/ui/browser_list.h"
#include "radium/browser/ui/browser_window.h"

BrowserCloseManager::BrowserCloseManager() : current_browser_(nullptr) {}
BrowserCloseManager::~BrowserCloseManager() = default;

void BrowserCloseManager::StartClosingBrowsers() {
  close_timer_.emplace();

  // If the session is ending or a silent exit was requested, skip straight to
  // closing the browsers without waiting for beforeunload dialogs.
  if (browser_shutdown::ShouldIgnoreUnloadHandlers()) {
    // Tell everyone that we are shutting down.
    browser_shutdown::SetTryingToQuit(true);
    CloseBrowsers();
    return;
  }
  TryToCloseBrowsers();
}

void BrowserCloseManager::CancelBrowserClose() {
  // This is the abort endpoint. Record the metric if we haven't already.
  if (close_timer_) {
    base::UmaHistogramMediumTimes("Shutdown.Time.BrowserCloseManager.Canceled",
                                  close_timer_->Elapsed());
    close_timer_.reset();
  }

  browser_shutdown::SetTryingToQuit(false);
  for (Browser* browser : *BrowserList::GetInstance()) {
    browser->ResetTryToCloseWindow();
  }
}

void BrowserCloseManager::TryToCloseBrowsers() {
  // If all browser windows can immediately be closed, fall out of this loop and
  // close the browsers. If any browser window cannot be closed, temporarily
  // stop closing. CallBeforeUnloadHandlers prompts the user and calls
  // OnBrowserReportCloseable with the result. If the user confirms the close,
  // this will trigger TryToCloseBrowsers to try again.
  for (Browser* browser : *BrowserList::GetInstance()) {
    if (browser->TryToCloseWindow(
            false, base::BindRepeating(
                       &BrowserCloseManager::OnBrowserReportCloseable, this))) {
      current_browser_ = browser;
      return;
    }
  }

  // This is the success endpoint. If we get here, all beforeunload handlers
  // have been processed successfully.
  if (close_timer_) {
    base::UmaHistogramMediumTimes("Shutdown.Time.BrowserCloseManager.Confirmed",
                                  close_timer_->Elapsed());
    close_timer_.reset();
  }

  CheckForDownloadsInProgress();
}

void BrowserCloseManager::OnBrowserReportCloseable(bool proceed) {
  if (!current_browser_) {
    return;
  }

  current_browser_ = nullptr;

  if (proceed) {
    TryToCloseBrowsers();
  } else {
    CancelBrowserClose();
  }
}

void BrowserCloseManager::CheckForDownloadsInProgress() {
  CloseBrowsers();
}

void BrowserCloseManager::ConfirmCloseWithPendingDownloads(
    int download_count,
    base::OnceCallback<void(bool)> callback) {
  NOTREACHED();
}

void BrowserCloseManager::OnReportDownloadsCancellable(bool proceed) {
  NOTREACHED();
}

void BrowserCloseManager::CloseBrowsers() {
  BrowserList::GetInstance()->ForEachCurrentAndNewBrowser([](Browser* browser) {
    bool ignore_unload_handlers =
        browser_shutdown::ShouldIgnoreUnloadHandlers();
    browser->set_force_skip_warning_user_on_close(ignore_unload_handlers);
    browser->window()->Close();
    if (ignore_unload_handlers) {
      NOTREACHED();
    }
  });
}
