// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/lifetime/browser_shutdown.h"

#include "base/clang_profiling_buildflags.h"
#include "build/config/compiler/compiler_buildflags.h"
#include "components/crash/core/common/crash_key.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "radium/browser/browser_process.h"

namespace browser_shutdown {

namespace {

// Whether the browser is trying to quit (e.g., Quit chosen from menu).
bool g_trying_to_quit = false;

base::Time* g_shutdown_started = nullptr;
ShutdownType g_shutdown_type = ShutdownType::kNotValid;
int g_shutdown_num_processes;
int g_shutdown_num_processes_slow;

const char* ToShutdownTypeString(ShutdownType type) {
  switch (type) {
    case ShutdownType::kNotValid:
      NOTREACHED();
    case ShutdownType::kWindowClose:
      return "close";
    case ShutdownType::kBrowserExit:
      return "exit";
    case ShutdownType::kEndSession:
      return "end";
    case ShutdownType::kSilentExit:
      return "silent_exit";
    case ShutdownType::kOtherExit:
      return "other_exit";
  }
  return "";
}

// Utility function to verify that globals are accessed on the UI/Main thread.
void CheckAccessedOnCorrectThread() {
  // Some APIs below are accessed after UI thread has been torn down, so cater
  // for both situations here.
  DCHECK(
      content::BrowserThread::CurrentlyOn(content::BrowserThread::UI) ||
      !content::BrowserThread::IsThreadInitialized(content::BrowserThread::UI));
}

}  // namespace

void OnShutdownStarting(ShutdownType type) {
  CheckAccessedOnCorrectThread();
  if (g_shutdown_type != ShutdownType::kNotValid) {
    return;
  }

  static crash_reporter::CrashKeyString<11> shutdown_type_key("shutdown-type");
  shutdown_type_key.Set(ToShutdownTypeString(type));

  g_shutdown_type = type;
  // For now, we're only counting the number of renderer processes
  // since we can't safely count the number of plugin processes from this
  // thread, and we'd really like to avoid anything which might add further
  // delays to shutdown time.
  DCHECK(!g_shutdown_started);
  g_shutdown_started = new base::Time(base::Time::Now());

  // TODO(crbug.com/40685224): Check if this should also be enabled for
  // coverage builds.
#if BUILDFLAG(CLANG_PROFILING_INSIDE_SANDBOX) && BUILDFLAG(CLANG_PGO_PROFILING)
  // Wait for all the child processes to dump their profiling data without
  // blocking the main thread.
  base::RunLoop nested_run_loop(base::RunLoop::Type::kNestableTasksAllowed);
  content::AskAllChildrenToDumpProfilingData(nested_run_loop.QuitClosure());
  nested_run_loop.Run();
#endif

  // Call FastShutdown on all of the RenderProcessHosts.  This will be
  // a no-op in some cases, so we still need to go through the normal
  // shutdown path for the ones that didn't exit here.
  if (BrowserProcess::Get()) {
    g_shutdown_num_processes = 0;
    g_shutdown_num_processes_slow = 0;
    for (content::RenderProcessHost::iterator i(
             content::RenderProcessHost::AllHostsIterator());
         !i.IsAtEnd(); i.Advance()) {
      ++g_shutdown_num_processes;
      if (!i.GetCurrentValue()->FastShutdownIfPossible()) {
        ++g_shutdown_num_processes_slow;
      }
    }
  }
}

bool HasShutdownStarted() {
  CheckAccessedOnCorrectThread();
  return g_shutdown_type != ShutdownType::kNotValid;
}

bool ShouldIgnoreUnloadHandlers() {
  CheckAccessedOnCorrectThread();
  return g_shutdown_type == ShutdownType::kEndSession ||
         g_shutdown_type == ShutdownType::kSilentExit;
}

void SetTryingToQuit(bool quitting) {
  CheckAccessedOnCorrectThread();
  g_trying_to_quit = quitting;
}

bool IsTryingToQuit() {
  CheckAccessedOnCorrectThread();
  return g_trying_to_quit;
}

}  // namespace browser_shutdown
