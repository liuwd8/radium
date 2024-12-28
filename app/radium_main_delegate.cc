// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/app/radium_main_delegate.h"

#include "base/command_line.h"
#include "base/features.h"
#include "base/functional/overloaded.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/process/memory.h"
#include "base/process/process.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/hang_watcher.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/crash/core/common/crash_key.h"
#include "components/memory_system/initializer.h"
#include "components/memory_system/parameters.h"
#include "components/metrics/persistent_histograms.h"
#include "components/startup_metric_utils/common/startup_metric_utils.h"
#include "content/public/app/initialize_mojo_core.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/profiling.h"
#include "content/public/common/url_constants.h"
#include "net/http/http_cache.h"
#include "net/url_request/url_request.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/browser/radium_content_browser_client.h"
#include "radium/browser/radium_resource_bundle_helper.h"
#include "radium/common/channel_info.h"
#include "radium/common/logging_radium.h"
#include "radium/common/profiler/unwind_util.h"
#include "radium/common/radium_constants.h"
#include "radium/common/radium_paths.h"
#include "radium/common/radium_result_codes.h"
#include "radium/common/radium_version.h"
#include "radium/common/webui_url_constants.h"
#include "services/tracing/public/cpp/stack_sampling/tracing_sampler_profiler.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"
#include "ui/linux/display_server_utils.h"
#include "ui/ozone/public/ozone_platform.h"

#if BUILDFLAG(IS_WIN)
#include <malloc.h>

#include <algorithm>

#include "base/base_switches.h"
#include "base/files/important_file_writer_cleaner.h"
#include "base/win/atl.h"
#include "base/win/dark_mode_support.h"
#include "base/win/resource_exhaustion.h"
#include "radium/child/v8_crashpad_support_win.h"
#include "radium/radium_elf/radium_elf_main.h"
#include "sandbox/win/src/sandbox.h"
#include "sandbox/win/src/sandbox_factory.h"
#include "ui/base/resource/resource_bundle_win.h"
#endif

#if BUILDFLAG(IS_MAC)
#include "base/apple/foundation_util.h"
#include "components/crash/core/common/objc_zombie.h"
#include "radium/browser/radium_browser_application_mac.h"
#include "ui/base/l10n/l10n_util_mac.h"
#endif

#if BUILDFLAG(IS_ANDROID)
#include "radium/common/radium_descriptors.h"
#endif

const char* const RadiumMainDelegate::kNonWildcardDomainNonPortSchemes[] = {
    radium::kRadiumUIScheme,
    content::kChromeDevToolsScheme,
    content::kChromeUIScheme,
    content::kChromeUIUntrustedScheme,
};
const size_t RadiumMainDelegate::kNonWildcardDomainNonPortSchemesSize =
    std::size(kNonWildcardDomainNonPortSchemes);

namespace {

#if BUILDFLAG(IS_ANDROID)

// Use the LibunwindstackNativeUnwinderAndroid for only browser main thread, and
// only on Android.
BASE_FEATURE(kUseLibunwindstackNativeUnwinderAndroid,
             "UseLibunwindstackNativeUnwinderAndroid",
             base::FEATURE_ENABLED_BY_DEFAULT);

#endif

#if BUILDFLAG(IS_WIN)
// Early versions of Chrome incorrectly registered a chromehtml: URL handler,
// which gives us nothing but trouble. Avoid launching chrome this way since
// some apps fail to properly escape arguments.
bool HasDeprecatedArguments(const std::wstring& command_line) {
  const wchar_t kChromeHtml[] = L"chromehtml:";
  std::wstring command_line_lower = base::ToLowerASCII(command_line);
  // We are only searching for ASCII characters so this is OK.
  return (command_line_lower.find(kChromeHtml) != std::wstring::npos);
}

// If we try to access a path that is not currently available, we want the call
// to fail rather than show an error dialog.
void SuppressWindowsErrorDialogs() {
  UINT new_flags = SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX;

  // Preserve existing error mode.
  UINT existing_flags = SetErrorMode(new_flags);
  SetErrorMode(existing_flags | new_flags);
}

bool IsSandboxedProcess() {
  typedef bool (*IsSandboxedProcessFunc)();
  IsSandboxedProcessFunc is_sandboxed_process_func =
      reinterpret_cast<IsSandboxedProcessFunc>(
          GetProcAddress(GetModuleHandle(NULL), "IsSandboxedProcess"));
  return is_sandboxed_process_func && is_sandboxed_process_func();
}

#endif  // BUILDFLAG(IS_WIN)

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
void AdjustLinuxOOMScore(const std::string& process_type) {
  int score = -1;

  if (process_type == switches::kPpapiPluginProcess) {
    score = content::kPluginOomScore;
  } else if (process_type == switches::kUtilityProcess ||
             process_type == switches::kGpuProcess) {
    score = content::kMiscOomScore;
  } else if (process_type == switches::kZygoteProcess || process_type.empty()) {
    // For zygotes and unlabeled process types, we want to still make
    // them killable by the OOM killer.
    score = content::kZygoteOomScore;
  } else if (process_type == switches::kRendererProcess) {
    LOG(WARNING) << "process type 'renderer' "
                 << "should be created through the zygote.";
    // When debugging, this process type can end up being run directly, but
    // this isn't the typical path for assigning the OOM score for it.  Still,
    // we want to assign a score that is somewhat representative for debugging.
    score = content::kLowestRendererOomScore;
  } else {
    NOTREACHED() << "Unknown process type";
  }
  // In the case of a 0 score, still try to adjust it. Most likely the score is
  // 0 already, but it may not be if this process inherited a higher score from
  // its parent process.
  if (score > -1) {
    base::AdjustOOMScore(base::GetCurrentProcId(), score);
  }
}
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)

// Returns true if this subprocess type needs the ResourceBundle initialized
// and resources loaded.
bool SubprocessNeedsResourceBundle(const std::string& process_type) {
  return
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
      // The zygote process opens the resources for the renderers.
      process_type == switches::kZygoteProcess ||
#endif
#if BUILDFLAG(IS_MAC)
      process_type == switches::kGpuProcess ||
#endif
      process_type == switches::kPpapiPluginProcess ||
      process_type == switches::kRendererProcess ||
      process_type == switches::kUtilityProcess;
}
#if BUILDFLAG(IS_POSIX)

#if !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_ANDROID)
void SIGTERMProfilingShutdown(int signal) {
  content::Profiling::Stop();
  struct sigaction sigact;
  memset(&sigact, 0, sizeof(sigact));
  sigact.sa_handler = SIG_DFL;
  CHECK_EQ(sigaction(SIGTERM, &sigact, nullptr), 0);
  raise(signal);
}

void SetUpProfilingShutdownHandler() {
  struct sigaction sigact;
  sigact.sa_handler = SIGTERMProfilingShutdown;
  sigact.sa_flags = SA_RESETHAND;
  sigemptyset(&sigact.sa_mask);
  CHECK_EQ(sigaction(SIGTERM, &sigact, nullptr), 0);
}
#endif  // !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_ANDROID)

#endif  // BUILDFLAG(IS_POSIX)

#if !BUILDFLAG(IS_ANDROID)
void InitLogging(const std::string& process_type) {
  logging::OldFileDeletionState file_state = logging::APPEND_TO_OLD_LOG_FILE;
  if (process_type.empty()) {
    file_state = logging::DELETE_OLD_LOG_FILE;
  }
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  logging::InitRadiumLogging(command_line, file_state);
  // Log the Chrome version for information. Do so at WARNING level as that's
  // the min level on ChromeOS.
  if (process_type.empty()) {
    LOG(WARNING) << "This is Chromium version " << radium::kRadiumVersion
                 << " (not a warning)";
  }
}
#endif  // !BUILDFLAG(IS_ANDROID)

void RecordMainStartupMetrics(const StartupTimestamps& timestamps) {
  const base::TimeTicks now = base::TimeTicks::Now();

#if BUILDFLAG(IS_WIN)
  startup_metric_utils::GetCommon().RecordPreReadTime(
      timestamps.preread_begin_ticks, timestamps.preread_end_ticks);
#endif

  // On Android the main entry point time is the time when the Java code starts.
  // This happens before the shared library containing this code is even loaded.
  // The Java startup code has recorded that time, but the C++ code can't fetch
  // it from the Java side until it has initialized the JNI. See
  // RadiumMainDelegateAndroid.
#if !BUILDFLAG(IS_ANDROID)
  // On all other platforms, `timestamps.exe_entry_point_ticks` contains the exe
  // entry point time (on some platforms this is ChromeMain, on some it is
  // before).
  CHECK(!timestamps.exe_entry_point_ticks.is_null());
  startup_metric_utils::GetCommon().RecordApplicationStartTime(
      timestamps.exe_entry_point_ticks);
#endif

#if BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN) || BUILDFLAG(IS_LINUX) || \
    BUILDFLAG(IS_CHROMEOS)
  // Record the startup process creation time on supported platforms. On Android
  // this is recorded in RadiumMainDelegateAndroid.
  startup_metric_utils::GetCommon().RecordStartupProcessCreationTime(
      base::Process::Current().CreationTime());
#endif

  startup_metric_utils::GetCommon().RecordChromeMainEntryTime(now);
}

#if BUILDFLAG(IS_WIN)
constexpr wchar_t kOnResourceExhaustedMessage[] =
    L"Your computer has run out of resources and cannot start "
    PRODUCT_SHORTNAME_STRING
    L". Sign out of Windows or restart your computer and try again.";

void OnResourceExhausted() {
  // RegisterClassEx will fail if the session's pool of ATOMs is exhausted. This
  // appears to happen most often when the browser is being driven by automation
  // tools, though the underlying reason for this remains a mystery
  // (https://crbug.com/1470483). There is nothing that Chrome can do to
  // meaningfully run until the user restarts their session by signing out of
  // Windows or restarting their computer.
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kNoErrorDialogs)) {
    static constexpr wchar_t kMessageBoxTitle[] = L"System resource exhausted";
    ::MessageBox(nullptr, kOnResourceExhaustedMessage, kMessageBoxTitle, MB_OK);
  }
  base::Process::TerminateCurrentProcessImmediately(
      radium::RESULT_CODE_SYSTEM_RESOURCE_EXHAUSTED);
}
#endif

bool IsCanaryDev() {
  const auto channel = radium::GetChannel();
  return channel == version_info::Channel::CANARY ||
         channel == version_info::Channel::DEV;
}

}  // namespace

#if BUILDFLAG(IS_ANDROID)
RadiumMainDelegate::RadiumMainDelegate()
    : RadiumMainDelegate(StartupTimestamps{}) {}
#endif

RadiumMainDelegate::RadiumMainDelegate(const StartupTimestamps& timestamps) {
  // Record startup metrics in the browser process. For component builds, there
  // is no way to know the type of process (process command line is not yet
  // initialized), so the function below will also be called in renderers.
  // This doesn't matter as it simply sets global variables.
  RecordMainStartupMetrics(timestamps);
}

RadiumMainDelegate::~RadiumMainDelegate() = default;

std::optional<int> RadiumMainDelegate::BasicStartupComplete() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  // The DevTools remote debugging pipe file descriptors need to be checked
  // before any other files are opened, see https://crbug.com/1423048.
  const bool is_browser = !command_line.HasSwitch(switches::kProcessType);
  (void)is_browser;

#if BUILDFLAG(IS_WIN)
  // Browser should not be sandboxed.
  if (is_browser && IsSandboxedProcess()) {
    return radium::RESULT_CODE_INVALID_SANDBOX_STATE;
  }
#endif

#if BUILDFLAG(IS_MAC)
  // Give the browser process a longer treadmill, since crashes
  // there have more impact.
  ObjcEvilDoers::ZombieEnable(true, is_browser ? 10000 : 1000);
#endif

  content::Profiling::ProcessStarted();

  // Setup tracing sampler profiler as early as possible at startup if needed.
  SetupTracing();

#if BUILDFLAG(IS_WIN)
  v8_crashpad_support::SetUp();
#endif

#if BUILDFLAG(IS_WIN)
  // Must do this before any other usage of command line!
  if (HasDeprecatedArguments(command_line.GetCommandLineString())) {
    return 1;
  }

  // HandleVerifier detects and reports incorrect handle manipulations. It
  // tracks handle operations on builds that support DCHECK only.
#if !DCHECK_IS_ON()
  base::win::DisableHandleVerifier();
#endif

#endif  // BUILDFLAG(IS_WIN)

  radium::RegisterPathProvider();

#if !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_ANDROID)
  base::FilePath locales;
  base::PathService::Get(base::DIR_ASSETS, &locales);
  base::PathService::Override(
      ui::DIR_LOCALES,
      locales.Append(FILE_PATH_LITERAL("radium_locales")));
#endif

  ContentSettingsPattern::SetNonWildcardDomainNonPortSchemes(
      kNonWildcardDomainNonPortSchemes, kNonWildcardDomainNonPortSchemesSize);
  return std::nullopt;
}

void RadiumMainDelegate::PreSandboxStartup() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line.GetSwitchValueASCII(switches::kProcessType);

  crash_reporter::InitializeCrashKeys();

#if !BUILDFLAG(IS_ANDROID) && !BUILDFLAG(IS_WIN)
  // Android does InitLogging when library is loaded. Skip here.
  // For windows we call InitLogging when the sandbox is initialized.
  InitLogging(process_type);
#endif

  if (SubprocessNeedsResourceBundle(process_type)) {
    // Initialize ResourceBundle which handles files loaded from external
    // sources.  The language should have been passed in to us from the
    // browser process as a command line flag.
    DUMP_WILL_BE_CHECK(command_line.HasSwitch(switches::kLang) ||
                       process_type == switches::kZygoteProcess ||
                       process_type == switches::kGpuProcess ||
                       process_type == switches::kPpapiPluginProcess);

    // TODO(markusheintz): The command line flag --lang is actually processed
    // by the CommandLinePrefStore, and made available through the PrefService
    // via the preference prefs::kApplicationLocale. The browser process uses
    // the --lang flag to pass the value of the PrefService in here. Maybe
    // this value could be passed in a different way.
    std::string locale = command_line.GetSwitchValueASCII(switches::kLang);
#if BUILDFLAG(IS_CHROMEOS_ASH)
    if (process_type == switches::kZygoteProcess) {
      DUMP_WILL_BE_CHECK(locale.empty());
      // See comment at ReadAppLocale() for why we do this.
      locale = ash::startup_settings_cache::ReadAppLocale();
    }

    ui::ResourceBundle::SetLottieParsingFunctions(
        &lottie::ParseLottieAsStillImage,
        &lottie::ParseLottieAsThemedStillImage);
#endif
#if BUILDFLAG(IS_ANDROID)
    // The renderer sandbox prevents us from accessing our .pak files directly.
    // Therefore file descriptors to the .pak files that we need are passed in
    // at process creation time.
    auto* global_descriptors = base::GlobalDescriptors::GetInstance();
    int pak_fd = global_descriptors->Get(kAndroidLocalePakDescriptor);
    base::MemoryMappedFile::Region pak_region =
        global_descriptors->GetRegion(kAndroidLocalePakDescriptor);
    ui::ResourceBundle::InitSharedInstanceWithPakFileRegion(base::File(pak_fd),
                                                            pak_region);

    // Load secondary locale .pak file if it exists.
    pak_fd = global_descriptors->MaybeGet(kAndroidSecondaryLocalePakDescriptor);
    if (pak_fd != -1) {
      pak_region =
          global_descriptors->GetRegion(kAndroidSecondaryLocalePakDescriptor);
      ui::ResourceBundle::GetSharedInstance()
          .LoadSecondaryLocaleDataWithPakFileRegion(base::File(pak_fd),
                                                    pak_region);
    }

    int extra_pak_keys[] = {
        kAndroidChrome100PercentPakDescriptor,
        kAndroidUIResourcesPakDescriptor,
    };
    for (int extra_pak_key : extra_pak_keys) {
      pak_fd = global_descriptors->Get(extra_pak_key);
      pak_region = global_descriptors->GetRegion(extra_pak_key);
      ui::ResourceBundle::GetSharedInstance().AddDataPackFromFileRegion(
          base::File(pak_fd), pak_region, ui::k100Percent);
    }

    // For Android: Native resources for DFMs should only be used by the browser
    // process. Their file descriptors and memory mapped file region are not
    // passed to child processes, and are therefore not loaded here.

    base::i18n::SetICUDefaultLocale(locale);
    const std::string loaded_locale = locale;
#else
    const std::string loaded_locale =
        ui::ResourceBundle::InitSharedInstanceWithLocale(
            locale, nullptr, ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);

    auto GetResourcesPakFilePath = [](const std::string& pak_name) {
      base::FilePath path;
      if (base::PathService::Get(base::DIR_ASSETS, &path)) {
        return path.AppendASCII(pak_name.c_str());
      }
      // Return just the name of the pak file.
#if BUILDFLAG(IS_WIN)
      return base::FilePath(base::ASCIIToWide(pak_name));
#else
      return base::FilePath(pak_name.c_str());
#endif  // BUILDFLAG(IS_WIN)
    };

    // Always load the 1x data pack first as the 2x data pack contains both 1x
    // and 2x images. The 1x data pack only has 1x images, thus passes in an
    // accurate scale factor to gfx::ImageSkia::AddRepresentation.
    if (ui::IsScaleFactorSupported(ui::k100Percent)) {
      ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
          GetResourcesPakFilePath("radium_100_percent.pak"), ui::k100Percent);
    }

    if (ui::IsScaleFactorSupported(ui::k200Percent)) {
      ui::ResourceBundle::GetSharedInstance().AddOptionalDataPackFromPath(
          GetResourcesPakFilePath("radium_200_percent.pak"), ui::k200Percent);
    }

    base::FilePath resources_pack_path;
    base::PathService::Get(radium::FILE_RESOURCES_PACK, &resources_pack_path);
#if BUILDFLAG(IS_CHROMEOS_LACROS)
    if (command_line.HasSwitch(switches::kEnableResourcesFileSharing)) {
      // If LacrosResourcesFileSharing feature is enabled, Lacros refers to ash
      // resources pak file.
      base::FilePath ash_resources_pack_path;
      base::PathService::Get(chrome::FILE_ASH_RESOURCES_PACK,
                             &ash_resources_pack_path);
      base::FilePath shared_resources_pack_path;
      base::PathService::Get(chrome::FILE_RESOURCES_FOR_SHARING_PACK,
                             &shared_resources_pack_path);
      ui::ResourceBundle::GetSharedInstance()
          .AddDataPackFromPathWithAshResources(
              shared_resources_pack_path, ash_resources_pack_path,
              resources_pack_path, ui::kScaleFactorNone);
    } else {
      ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
          resources_pack_path, ui::kScaleFactorNone);
    }
#else
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        resources_pack_path, ui::kScaleFactorNone);
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)
#endif  // BUILDFLAG(IS_ANDROID)
    CHECK(!loaded_locale.empty()) << "Locale could not be found for " << locale;
  }
}

std::optional<int> RadiumMainDelegate::PreBrowserMain() {
  std::optional<int> exit_code = content::ContentMainDelegate::PreBrowserMain();
  if (exit_code.has_value()) {
    return exit_code;
  }

#if BUILDFLAG(IS_MAC)
  // Tell Cocoa to finish its initialization, which we want to do manually
  // instead of calling NSApplicationMain(). The primary reason is that NSAM()
  // never returns, which would leave all the objects currently on the stack
  // in scoped_ptrs hanging and never cleaned up. We then load the main nib
  // directly. The main event loop is run from common code using the
  // MessageLoop API, which works out ok for us because it's a wrapper around
  // CFRunLoop.

  // Initialize NSApplication using the custom subclass.
  radium_browser_application_mac::RegisterBrowserCrApp();

  if (l10n_util::GetLocaleOverride().empty()) {
    // The browser process only wants to support the language Cocoa will use,
    // so force the app locale to be overridden with that value. This must
    // happen before the ResourceBundle is loaded, which happens in
    // ChromeBrowserMainParts::PreEarlyInitialization().
    // Don't do this if the locale is already set, which is done by integration
    // tests to ensure tests always run with the same locale.
    l10n_util::OverrideLocaleWithCocoaLocale();
  }
#endif

#if BUILDFLAG(IS_WIN)
  // Register callback to handle resource exhaustion.
  base::win::SetOnResourceExhaustedFunction(OnResourceExhausted);

  if (IsExtensionPointDisableSet()) {
    sandbox::SandboxFactory::GetBrokerServices()->SetStartingMitigations(
        sandbox::MITIGATION_EXTENSION_POINT_DISABLE);
  }
#endif

  // Do not interrupt startup.
  return std::nullopt;
}

void RadiumMainDelegate::SandboxInitialized(const std::string& process_type) {
  // Note: If you are adding a new process type below, be sure to adjust the
  // AdjustLinuxOOMScore function too.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  AdjustLinuxOOMScore(process_type);
#endif
#if BUILDFLAG(IS_WIN)
  InitLogging(process_type);
  SuppressWindowsErrorDialogs();
#endif

  // If this is a browser process, initialize the persistent histograms system
  // unless headless mode is in effect. This is done as soon as possible to
  // ensure metrics collection coverage. For Fuchsia, persistent histogram
  // initialization is done after field trial initialization (so that it can be
  // controlled from the serverside and experimented with). Note: this is done
  // before field trial initialization, so the values of
  // `kPersistentHistogramsFeature` and `kPersistentHistogramsStorage` will
  // not be used. Persist histograms to a memory-mapped file.
  if (process_type.empty()) {
    base::FilePath metrics_dir;
    if (base::PathService::Get(radium::DIR_USER_DATA, &metrics_dir)) {
      InstantiatePersistentHistograms(
          metrics_dir,
          /*persistent_histograms_enabled=*/true,
          /*storage=*/kPersistentHistogramStorageMappedFile);
    } else {
      DUMP_WILL_BE_NOTREACHED();
    }
  }
}

void RadiumMainDelegate::ProcessExiting(const std::string& process_type) {
  if (SubprocessNeedsResourceBundle(process_type)) {
    ui::ResourceBundle::CleanupSharedInstance();
  }
#if !BUILDFLAG(IS_ANDROID)
  logging::CleanupRadiumLogging();
#else
  // Android doesn't use InitChromeLogging, so we close the log file manually.
  logging::CloseLogFile();
#endif  // !BUILDFLAG(IS_ANDROID)
}

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
void RadiumMainDelegate::ZygoteForked() {
  // Set up tracing for processes forked off a zygote.
  SetupTracing();

  content::Profiling::ProcessStarted();
  if (content::Profiling::BeingProfiled()) {
    base::debug::RestartProfilingAfterFork();
    SetUpProfilingShutdownHandler();
  }
}

#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)

bool RadiumMainDelegate::ShouldCreateFeatureList(InvokedIn invoked_in) {
  return absl::holds_alternative<InvokedInChildProcess>(invoked_in);
}

bool RadiumMainDelegate::ShouldInitializeMojo(InvokedIn invoked_in) {
  return ShouldCreateFeatureList(invoked_in);
}

std::optional<int> RadiumMainDelegate::PostEarlyInitialization(
    InvokedIn invoked_in) {
  DUMP_WILL_BE_CHECK(base::ThreadPoolInstance::Get());
  const auto* invoked_in_browser =
      absl::get_if<InvokedInBrowserProcess>(&invoked_in);
  if (!invoked_in_browser) {
    CommonEarlyInitialization(invoked_in);
    return std::nullopt;
  }

#if BUILDFLAG(IS_WIN)
  // Initialize the cleaner of left-behind tmp files now that the main thread
  // has its SequencedTaskRunner; see https://crbug.com/1075917.
  base::ImportantFileWriterCleaner::GetInstance().Initialize();

  // Make sure the 'uxtheme.dll' is pinned.
  base::win::AllowDarkModeForApp(true);
#endif

  // Chrome disallows cookies by default. All code paths that want to use
  // cookies need to go through one of Chrome's URLRequestContexts which have
  // a ChromeNetworkDelegate attached that selectively allows cookies again.
  net::URLRequest::SetDefaultCookiePolicyToBlock();

  // The DBus initialization above is needed for FeatureList creation here;
  // features are needed for Mojo initialization; and Mojo initialization is
  // needed for LacrosService initialization below.
  RadiumFeatureListCreator* radium_feature_list_creator =
      radium_content_browser_client_->radium_feature_list_creator();
  radium_feature_list_creator->CreateFeatureList();

#if BUILDFLAG(IS_OZONE)
  // Initialize Ozone platform and add required feature flags as per platform's
  // properties.
#if BUILDFLAG(IS_LINUX)
  ui::SetOzonePlatformForLinuxIfNeeded(*base::CommandLine::ForCurrentProcess());
#endif
  ui::OzonePlatform::PreEarlyInitialization();
#endif  // BUILDFLAG(IS_OZONE)

  content::InitializeMojoCore();

  CommonEarlyInitialization(invoked_in);

  // Initializes the resource bundle and determines the locale.
  std::string actual_locale = LoadLocalState(radium_feature_list_creator);
  radium_feature_list_creator->SetApplicationLocale(actual_locale);
  radium_feature_list_creator->OverrideCachedUIStrings();

  return std::nullopt;
}

content::ContentClient* RadiumMainDelegate::CreateContentClient() {
  return &radium_content_client_;
}

content::ContentBrowserClient*
RadiumMainDelegate::CreateContentBrowserClient() {
  radium_content_browser_client_ =
      std::make_unique<RadiumContentBrowserClient>();
  return radium_content_browser_client_.get();
}

void RadiumMainDelegate::CommonEarlyInitialization(InvokedIn invoked_in) {
  const base::CommandLine* const command_line =
      base::CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);

  // Enable Split cache by default here and not in content/ so as to not
  // impact non-Chrome embedders like WebView, Cronet etc. This only enables
  // it if not already overridden by command line, field trial etc.
  net::HttpCache::SplitCacheFeatureEnableByDefault();

  // Similarly, enable network state partitioning by default.
  net::NetworkAnonymizationKey::PartitionByDefault();

  // Start memory observation as early as possible so it can start recording
  // memory allocations. This includes heap profiling.
  InitializeMemorySystem();

  // Initialize the HangWatcher.
  base::HangWatcher::ProcessType hang_watcher_process_type;
  if (process_type.empty()) {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kBrowserProcess;
  } else if (process_type == switches::kGpuProcess) {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kGPUProcess;
  } else if (process_type == switches::kRendererProcess) {
    hang_watcher_process_type =
        base::HangWatcher::ProcessType::kRendererProcess;
  } else if (process_type == switches::kUtilityProcess) {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kUtilityProcess;
  } else {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kUnknownProcess;
  }

  const bool is_canary_dev = IsCanaryDev();
  const bool emit_crashes =
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_MAC) || \
    BUILDFLAG(IS_WIN)
      is_canary_dev;
#else
      false;
#endif

  base::HangWatcher::InitializeOnMainThread(hang_watcher_process_type,
                                            emit_crashes);

  // Force emitting `ThreadController` profiler metadata on Canary and Dev only,
  // since they are the only channels where the data is used.
  base::features::Init(
      is_canary_dev
          ? base::features::EmitThreadControllerProfilerMetadata::kForce
          : base::features::EmitThreadControllerProfilerMetadata::
                kFeatureDependent);
}

void RadiumMainDelegate::SetupTracing() {
  // It is necessary to reset the unique_ptr before assigning a new value to it.
  // This is to ensure that g_main_thread_instance inside
  // tracing_sampler_profiler.cc comes out correctly -- the old
  // TracingSamplerProfiler must destruct and clear g_main_thread_instance
  // before CreateOnMainThread() runs.
  tracing_sampler_profiler_.reset();

#if BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_LINUX)
  // Don't set up tracing in zygotes. Zygotes don't do much, and the tracing
  // system won't work after a fork because all the thread IDs will change.
  if (base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kProcessType) == switches::kZygoteProcess) {
    return;
  }
#endif  // #if BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_LINUX)

  // We pass in CreateCoreUnwindersFactory here since it lives in the chrome/
  // layer while TracingSamplerProfiler is outside of chrome/.
  //
  // When we're the browser on android, use only libunwindstack for the tracing
  // sampler profiler because it can support java frames which is essential for
  // the main thread.
  base::RepeatingCallback tracing_factory =
      base::BindRepeating(&CreateCoreUnwindersFactory);
  tracing::TracingSamplerProfiler::UnwinderType unwinder_type =
      tracing::TracingSamplerProfiler::UnwinderType::kCustomAndroid;
#if BUILDFLAG(IS_ANDROID)
  // If we are the browser process (missing process type), then use the
  // experimental libunwindstack unwinder.
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kProcessType) &&
      base::FeatureList::IsEnabled(kUseLibunwindstackNativeUnwinderAndroid)) {
    tracing_factory = base::BindRepeating(&CreateLibunwindstackUnwinderFactory);
    unwinder_type = tracing::TracingSamplerProfiler::UnwinderType::
        kLibunwindstackUnwinderAndroid;
  }
#endif
  tracing_sampler_profiler_ =
      tracing::TracingSamplerProfiler::CreateOnMainThread(
          std::move(tracing_factory), unwinder_type);
}

void RadiumMainDelegate::InitializeMemorySystem() {
  const base::CommandLine* const command_line =
      base::CommandLine::ForCurrentProcess();
  const std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  const bool is_browser_process = process_type.empty();
  const bool gwp_asan_boost_sampling = is_browser_process || IsCanaryDev();
  const memory_system::DispatcherParameters::AllocationTraceRecorderInclusion
      allocation_recorder_inclusion =
          is_browser_process ? memory_system::DispatcherParameters::
                                   AllocationTraceRecorderInclusion::kDynamic
                             : memory_system::DispatcherParameters::
                                   AllocationTraceRecorderInclusion::kIgnore;

  memory_system::Initializer()
      .SetGwpAsanParameters(gwp_asan_boost_sampling, process_type)
      // .SetProfilingClientParameters(radium::GetChannel(),
      //                               GetProfilerProcessType(*command_line))
      .SetDispatcherParameters(memory_system::DispatcherParameters::
                                   PoissonAllocationSamplerInclusion::kEnforce,
                               allocation_recorder_inclusion, process_type)
      .Initialize(memory_system_);
}
