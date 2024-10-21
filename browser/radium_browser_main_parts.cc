// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts.h"

#include "base/debug/leak_annotations.h"
#include "base/run_loop.h"
#include "base/task/current_thread.h"
#include "base/threading/hang_watcher.h"
#include "base/trace_event/trace_event.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/radium_browser_main_extra_parts.h"
#include "radium/browser/ui/startup/startup_browser_creator.h"

namespace {
#if !BUILDFLAG(IS_ANDROID)
// Initialized in PreMainMessageLoopRun() and handed off to content:: in
// WillRunMainMessageLoop() (or in TakeRunLoopForTest() in tests)
std::unique_ptr<base::RunLoop>& GetMainRunLoopInstance() {
  static base::NoDestructor<std::unique_ptr<base::RunLoop>>
      main_run_loop_instance;
  return *main_run_loop_instance;
}
#endif

#if !BUILDFLAG(IS_ANDROID)
void StartWatchingForProcessShutdownHangs() {
  // This HangWatcher scope is covering the shutdown phase up to the end of the
  // process. Intentionally leak this instance so that it is not destroyed
  // before process termination.
  base::HangWatcher::SetShuttingDown();
  auto* watcher = new base::WatchHangsInScope(base::Seconds(30));
  ANNOTATE_LEAKING_OBJECT_PTR(watcher);
  std::ignore = watcher;
}
#endif  // !BUILDFLAG(IS_ANDROID)

}  // namespace

RadiumBrowserMainParts::RadiumBrowserMainParts(
    RadiumFeatureListCreator* radium_feature_list_creator)
    : radium_feature_list_creator_(radium_feature_list_creator) {}

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

int RadiumBrowserMainParts::PreEarlyInitialization() {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::PreEarlyInitialization");
  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PreEarlyInitialization();
  }

  // Create BrowserProcess in PreEarlyInitialization() so that we can load
  // field trials (and all it depends upon).
  browser_process_ = std::make_unique<BrowserProcess>(radium_feature_list_creator_);

  // Continue on and show error later (once UI has been initialized and main
  // message loop is running).
  return content::RESULT_CODE_NORMAL_EXIT;
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
    // These members must be initialized before exiting this function normally.
#if !BUILDFLAG(IS_ANDROID)
    DCHECK(browser_creator_.get());
#endif
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

  {
    TRACE_EVENT0(
        "startup",
        "RadiumBrowserMainParts::PreCreateThreadsImpl:InitBrowserProcess");
    browser_process_->Init();
  }

#if !BUILDFLAG(IS_ANDROID)
  // These members must be initialized before returning from this function.
  // Android doesn't use StartupBrowserCreator.
  browser_creator_ = std::make_unique<StartupBrowserCreator>();
#endif  // !BUILDFLAG(IS_ANDROID)

  // ChromeOS needs ui::ResourceBundle::InitSharedInstance to be called before
  // this.
  browser_process_->PreCreateThreads();

  return content::RESULT_CODE_NORMAL_EXIT;
}

int RadiumBrowserMainParts::PreMainMessageLoopRun() {
  TRACE_EVENT("startup", "RadiumBrowserMainParts::PreMainMessageLoopRun");

  result_code_ = PreMainMessageLoopRunImpl();

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PreMainMessageLoopRun();
  }

  return result_code_;
}

#if !BUILDFLAG(IS_ANDROID)
bool RadiumBrowserMainParts::ShouldInterceptMainMessageLoopRun() {
  // Some early return paths in PreMainMessageLoopRunImpl intentionally prevent
  // the main run loop from being created. Use this as a signal to indicate that
  // the main message loop shouldn't be run.
  return !!GetMainRunLoopInstance();
}
#endif

void RadiumBrowserMainParts::WillRunMainMessageLoop(
    std::unique_ptr<base::RunLoop>& run_loop) {
#if BUILDFLAG(IS_ANDROID)
  // Chrome on Android does not use default MessageLoop. It has its own
  // Android specific MessageLoop
  NOTREACHED_IN_MIGRATION();
#else
  DCHECK(base::CurrentUIThread::IsSet());

  run_loop = std::move(GetMainRunLoopInstance());

  // Trace the entry and exit of this main message loop. We don't use the
  // TRACE_EVENT_BEGIN0 macro because the tracing infrastructure doesn't expect
  // a synchronous event around the main loop of a thread.
  TRACE_EVENT_NESTABLE_ASYNC_BEGIN0(
      "toplevel", "RadiumBrowserMainParts::MainMessageLoopRun", this);
#endif  // BUILDFLAG(IS_ANDROID)
}

void RadiumBrowserMainParts::PostMainMessageLoopRun() {
  TRACE_EVENT_NESTABLE_ASYNC_END0(
      "toplevel", "RadiumBrowserMainParts::MainMessageLoopRun", this);
  TRACE_EVENT0("startup", "ChromeBrowserMainParts::PostMainMessageLoopRun");
#if BUILDFLAG(IS_ANDROID)
  // Chrome on Android does not use default MessageLoop. It has its own
  // Android specific MessageLoop
  NOTREACHED_IN_MIGRATION();
#else
  // Start watching hangs up to the end of the process.
  StartWatchingForProcessShutdownHangs();

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PostMainMessageLoopRun();
  }

  browser_process_->StartTearDown();
#endif  // BUILDFLAG(IS_ANDROID)
}

int RadiumBrowserMainParts::PreMainMessageLoopRunImpl() {
  // Do any initializating in the browser process that requires all threads
  // running.
  browser_process_->PreMainMessageLoopRun();

  if (true) {
    // Create the RunLoop for MainMessageLoopRun() to use and transfer
    // ownership of the browser's lifetime to the BrowserProcess.
    DCHECK(!GetMainRunLoopInstance());
    GetMainRunLoopInstance() = std::make_unique<base::RunLoop>();
    browser_process_->SetQuitClosure(
        GetMainRunLoopInstance()->QuitWhenIdleClosure());
  }
  return result_code_;
}
