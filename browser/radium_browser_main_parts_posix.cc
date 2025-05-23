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
#include "radium/browser/devtools/radium_devtools_manager_delegate.h"
#include "radium/browser/lifetime/application_lifetime.h"
#include "radium/browser/lifetime/application_lifetime_desktop.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/radium_browser_main_parts.h"
#include "radium/browser/shutdown_signal_handlers_posix.h"

namespace SessionRestore {

bool IsRestoringSynchronously() {
  return false;
}

}  // namespace SessionRestore

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

  // DevTools delegate's browser keeplive may prevent browser from closing so
  // remove it before proceeding because we have an explicit shutdown request.
  RadiumDevToolsManagerDelegate::AllowBrowserToClose();

  if (SessionRestore::IsRestoringSynchronously()) {
    // ExitHandler takes care of deleting itself.
    new ExitHandler();
  } else {
#if BUILDFLAG(IS_LINUX)
    switch (signal) {
      case SIGINT:
      case SIGHUP:
        // SIGINT gets sent when the user types Ctrl+C, but the session is
        // likely not going away, so try to exit gracefully.  SIGHUP is sent on
        // most systems as a first warning of shutdown.  If the process takes
        // too long to quit, the next signal is usually SIGTERM.
        Exit();
        break;
      case SIGTERM:
        // SIGTERM is usually sent instead of SIGKILL to gracefully shutdown
        // processes.  But most systems use it as a shutdown warning, so
        // conservatively assume that the session is ending.  If the process
        // still doesn't quit within a bounded time, most systems will finally
        // send SIGKILL, which we're unable to install a signal handler for.
        // TODO(thomasanderson): Try to distinguish if the session is really
        // ending or not.  Maybe there's a systemd or DBus API to query.
        radium::SessionEnding();
        break;
      default:
        NOTREACHED();
    }
#else
    Exit();
#endif
  }
}

ExitHandler::ExitHandler() = default;

ExitHandler::~ExitHandler() = default;

void ExitHandler::OnSessionRestoreDone(Profile* profile, int /* num_tabs */) {
  if (!SessionRestore::IsRestoringSynchronously()) {
    // At this point the message loop may not be running (meaning we haven't
    // gotten through browser startup, but are close). Post the task to at which
    // point the message loop is running.
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE, base::BindOnce(&ExitHandler::Exit));
    delete this;
  }
}

// static
void ExitHandler::Exit() {
  radium::AttemptExit();
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
  UNSAFE_BUFFERS(memset(&action, 0, sizeof(action)));
  action.sa_handler = SIGCHLDHandler;
  CHECK_EQ(0, sigaction(SIGCHLD, &action, nullptr));

  return content::RESULT_CODE_NORMAL_EXIT;
}

void RadiumBrowserMainPartsPosix::PostCreateMainMessageLoop() {
  RadiumBrowserMainParts::PostCreateMainMessageLoop();

  // Exit in response to SIGINT, SIGTERM, etc.
  InstallShutdownSignalHandlers(
      base::BindOnce(&ExitHandler::ExitWhenPossibleOnUIThread),
      content::GetUIThreadTaskRunner({}));
}
