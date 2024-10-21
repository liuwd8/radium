// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts_posix.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/resource.h>

#include "base/callback_list.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/radium_browser_main_parts.h"

namespace {

// See comment in |PreEarlyInitialization()|, where sigaction is called.
void SIGCHLDHandler(int signal) {}

// ExitHandler takes care of servicing an exit (from a signal) at the
// appropriate time. Specifically if we get an exit and have not finished
// session restore we delay the exit. To do otherwise means we're exiting part
// way through startup which causes all sorts of problems.
class ExitHandler {
 public:
  ExitHandler(const ExitHandler&) = delete;
  ExitHandler& operator=(const ExitHandler&) = delete;

  // Invokes exit when appropriate.
  static void ExitWhenPossibleOnUIThread(int signal);

 private:
  ExitHandler();
  ~ExitHandler();

  // Called when a session restore has finished.
  void OnSessionRestoreDone(Profile* profile, int num_tabs_restored);

  // Does the appropriate call to Exit.
  static void Exit();

  // Points to the on-session-restored callback that was registered with
  // SessionRestore's callback list. When objects of this class are destroyed,
  // the subscription's destructor will automatically unregister the callback in
  // SessionRestore, so that the callback list does not contain any obsolete
  // callbacks.
  base::CallbackListSubscription on_session_restored_callback_subscription_;
};

// static
void ExitHandler::ExitWhenPossibleOnUIThread(int signal) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

ExitHandler::ExitHandler() = default;

ExitHandler::~ExitHandler() = default;

void ExitHandler::OnSessionRestoreDone(Profile* profile, int /* num_tabs */) {}

// static
void ExitHandler::Exit() {
  LOG(ERROR) << __func__;
}

}  // namespace

RadiumBrowserMainPartsPosix::RadiumBrowserMainPartsPosix(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : RadiumBrowserMainParts(radium_feature_list_creator) {}

RadiumBrowserMainPartsPosix::~RadiumBrowserMainPartsPosix() = default;

int RadiumBrowserMainPartsPosix::PreEarlyInitialization() {
  const int result = RadiumBrowserMainParts::PreEarlyInitialization();
  if (result != content::RESULT_CODE_NORMAL_EXIT) {
    return result;
  }

  // We need to accept SIGCHLD, even though our handler is a no-op because
  // otherwise we cannot wait on children. (According to POSIX 2001.)
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIGCHLDHandler;
  CHECK_EQ(0, sigaction(SIGCHLD, &action, nullptr));

  return content::RESULT_CODE_NORMAL_EXIT;
}

void RadiumBrowserMainPartsPosix::PostCreateMainMessageLoop() {
  RadiumBrowserMainParts::PostCreateMainMessageLoop();

  // // Exit in response to SIGINT, SIGTERM, etc.
  // InstallShutdownSignalHandlers(
  //     base::BindOnce(&ExitHandler::ExitWhenPossibleOnUIThread),
  //     content::GetUIThreadTaskRunner({}));
}
