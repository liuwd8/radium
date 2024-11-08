// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_browser_main_parts.h"

#include "base/command_line.h"
#include "base/debug/leak_annotations.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/task/current_thread.h"
#include "base/threading/hang_watcher.h"
#include "base/trace_event/trace_event.h"
#include "components/color/color_mixers.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_service.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/global_features.h"
#include "radium/browser/profiles/profile_manager.h"
#include "radium/browser/profiles/profiles_state.h"
#include "radium/browser/radium_browser_main_extra_parts.h"
#include "radium/browser/ui/color/radium_color_mixers.h"
#include "radium/browser/ui/startup/startup_browser_creator.h"
#include "radium/browser/ui/webui/radium_web_ui_configs.h"
#include "radium/common/radium_paths.h"
#include "radium/common/radium_result_codes.h"
#include "ui/color/color_provider_manager.h"

#if BUILDFLAG(IS_WIN)
#include "base/trace_event/trace_event_etw_export_win.h"
#include "base/win/win_util.h"
#include "radium/browser/win/parental_controls.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#elif BUILDFLAG(IS_ANDROID)
#include "radium/browser/android/shell.h"
#include "radium/browser/android/shell_platform_delegate.h"
#endif  // BUILDFLAG(IS_WIN)

#if defined(USE_AURA)
#include "ui/aura/env.h"
#endif

namespace {
StartupProfileInfo CreateInitialProfile(
    const base::FilePath& cur_dir,
    const base::CommandLine& parsed_command_line) {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::CreateProfile");

  ProfileManager* profile_manager =
      BrowserProcess::Get()->GetFeatures()->profile_manager();
  const base::FilePath& user_data_dir = profile_manager->user_data_dir();

  StartupProfileInfo profile_info;
  profile_info.profile = profile_manager->GetProfile(
      profiles::GetDefaultProfileDir(user_data_dir), true);
  profile_info.mode = StartupProfileMode::kBrowserWindow;
  return profile_info;
}

#if !BUILDFLAG(IS_ANDROID)
// Initialized in PreMainMessageLoopRun() and handed off to content:: in
// WillRunMainMessageLoop() (or in TakeRunLoopForTest() in tests)
std::unique_ptr<base::RunLoop>& GetMainRunLoopInstance() {
  static base::NoDestructor<std::unique_ptr<base::RunLoop>>
      main_run_loop_instance;
  return *main_run_loop_instance;
}
#endif

void DisallowKeyedServiceFactoryRegistration() {
  // From this point, do not allow KeyedServiceFactories to be registered, all
  // factories should be registered in the main registration function
  // `ChromeBrowserMainExtraPartsProfiles::EnsureBrowserContextKeyedServiceFactoriesBuilt()`.
  BrowserContextDependencyManager::GetInstance()
      ->DisallowKeyedServiceFactoryRegistration(
          "ChromeBrowserMainExtraPartsProfiles::"
          "EnsureBrowserContextKeyedServiceFactoriesBuilt()");
}

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

  if (!base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir_)) {
    return radium::RESULT_CODE_MISSING_DATA;
  }

#if BUILDFLAG(IS_ANDROID)
  Profile::CreateProfilePrefService(base::PassKey<RadiumBrowserMainParts>(),
                                    user_data_dir_);
#endif

  // Continue on and show error later (once UI has been initialized and main
  // message loop is running).
  return content::RESULT_CODE_NORMAL_EXIT;
}

void RadiumBrowserMainParts::ToolkitInitialized() {
  TRACE_EVENT0("startup", "RadiumBrowserMainParts::ToolkitInitialized");

  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->ToolkitInitialized();
  }

  // Comes after the extra parts' calls since on GTK that builds the native
  // theme that, in turn, adds the GTK core color mixer; core mixers should all
  // be added before we add chrome mixers.
  ui::ColorProviderManager::Get().AppendColorProviderInitializer(
      base::BindRepeating(color::AddComponentsColorMixers));
  ui::ColorProviderManager::Get().AppendColorProviderInitializer(
      base::BindRepeating(AddRadiumColorMixers));
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

  browser_process_->PostDestroyThreads();

  browser_process_->StartTearDown();

  // From this point, the BrowserProcess class is no longer alive.
  browser_process_.reset();
#endif  // BUILDFLAG(IS_ANDROID)
}

void RadiumBrowserMainParts::PreProfileInit() {
  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PreProfileInit();
  }

  DisallowKeyedServiceFactoryRegistration();
}

void RadiumBrowserMainParts::PostProfileInit(Profile* profile,
                                             bool is_initial_profile) {}

void RadiumBrowserMainParts::PreBrowserStart() {
  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PreBrowserStart();
  }
}

void RadiumBrowserMainParts::PostBrowserStart() {
  for (auto& radium_extra_part : radium_extra_parts_) {
    radium_extra_part->PostBrowserStart();
  }
}

int RadiumBrowserMainParts::PreMainMessageLoopRunImpl() {
#if BUILDFLAG(IS_WIN)
  // Windows parental controls calls can be slow, so we do an early init here
  // that calculates this value off of the UI thread.
  InitializeWinParentalControls();
#endif

  // Do any initializating in the browser process that requires all threads
  // running.
  browser_process_->PreMainMessageLoopRun();

#if defined(USE_AURA)
  // Make sure aura::Env has been initialized.
  CHECK(aura::Env::GetInstance());
#endif  // defined(USE_AURA)

  // Desktop construction occurs here, (required before profile creation).
  PreProfileInit();

  // Needs to be done before PostProfileInit, to allow connecting DevTools
  // before WebUI for the CrOS login that can be called inside PostProfileInit
  BrowserProcess::Get()->CreateDevToolsProtocolHandler();

  StartupProfileInfo profile_info = CreateInitialProfile(
      base::FilePath(), *base::CommandLine::ForCurrentProcess());

  RegisterRadiumWebUIConfigs();

  PreBrowserStart();

#if !BUILDFLAG(IS_ANDROID)
  // We are in regular browser boot sequence. Open initial tabs and enter the
  // main message loop.
  std::vector<Profile*> last_opened_profiles;

  if (browser_creator_->Start(*base::CommandLine::ForCurrentProcess(),
                              base::FilePath(), profile_info,
                              last_opened_profiles)) {
    // Create the RunLoop for MainMessageLoopRun() to use and transfer
    // ownership of the browser's lifetime to the BrowserProcess.
    DCHECK(!GetMainRunLoopInstance());
    GetMainRunLoopInstance() = std::make_unique<base::RunLoop>();
    browser_process_->SetQuitClosure(
        GetMainRunLoopInstance()->QuitWhenIdleClosure());
  }
  browser_creator_.reset();
#else
  Shell::Initialize(std::make_unique<ShellPlatformDelegate>());
#endif  // !BUILDFLAG(IS_ANDROID)

  PostBrowserStart();

  return result_code_;
}
