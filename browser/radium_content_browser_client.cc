// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/radium_content_browser_client.h"

#include <memory>

#include "base/path_service.h"
#include "base/version_info/channel.h"
#include "base/version_info/version_info_values.h"
#include "build/build_config.h"
#include "components/embedder_support/user_agent_utils.h"
#include "components/keep_alive_registry/keep_alive_registry.h"
#include "components/net_log/chrome_net_log.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/content_descriptors.h"
#include "content/public/common/url_utils.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/devtools/radium_devtools_manager_delegate.h"
#include "radium/browser/metrics/radium_feature_list_creator.h"
#include "radium/browser/net/profile_network_context_service.h"
#include "radium/browser/net/profile_network_context_service_factory.h"
#include "radium/browser/net/system_network_context_manager.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/profiles/radium_browser_main_extra_parts_profiles.h"
#include "radium/browser/radium_browser_interface_binders.h"
#include "radium/browser/radium_browser_main_extra_parts.h"
#include "radium/browser/radium_browser_main_parts.h"
#include "radium/browser/themes/theme_service.h"
#include "radium/browser/themes/theme_service_factory.h"
#include "radium/browser/ui/tab_contents/radium_web_contents_view_delegate.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views.h"
#include "radium/common/channel_info.h"
#include "radium/common/logging_radium.h"
#include "radium/common/pref_names.h"
#include "radium/common/radium_paths.h"
#include "radium/common/radium_paths_internal.h"
#include "radium/common/radium_version.h"
#include "radium/common/webui_url_constants.h"
#include "third_party/blink/public/common/web_preferences/web_preferences.h"
#include "ui/native_theme/native_theme.h"

#if BUILDFLAG(IS_WIN)
#include "radium/browser/lifetime/application_lifetime_desktop.h"
#include "radium/browser/radium_browser_main_parts_win.h"
#elif BUILDFLAG(IS_LINUX)
#include "components/crash/core/app/crash_switches.h"
#include "components/crash/core/app/crashpad.h"
#include "radium/browser/radium_browser_main_parts_linux.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views_linux.h"
#elif BUILDFLAG(IS_MAC)
#include "radium/browser/radium_browser_main_extra_parts_mac.h"
#include "radium/browser/radium_browser_main_parts_mac.h"
#elif BUILDFLAG(IS_ANDROID)
#include "components/crash/content/browser/child_exit_observer_android.h"
#include "components/crash/content/browser/crash_handler_host_linux.h"
#include "components/crash/content/browser/crash_memory_metrics_collector_android.h"
#include "radium/browser/android/devtools_manager_delegate_android.h"
#include "radium/browser/radium_browser_main_parts_android.h"
#include "radium/browser/tab/tab_android.h"
#include "radium/browser/tab/tab_web_contents_delegate_android.h"
#include "radium/common/radium_descriptors.h"
#include "ui/base/resource/resource_bundle_android.h"
#include "ui/base/ui_base_paths.h"
#endif

#if BUILDFLAG(IS_LINUX)
#include "radium/browser/radium_browser_main_extra_parts_linux.h"
#elif BUILDFLAG(IS_OZONE)
#include "radium/browser/radium_browser_main_extra_parts_ozone.h"
#endif

namespace {

#if BUILDFLAG(IS_ANDROID)
int GetCrashSignalFD(const base::CommandLine& command_line) {
  return crashpad::CrashHandlerHost::Get()->GetDeathSignalSocket();
}
#elif BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
int GetCrashSignalFD(const base::CommandLine& command_line) {
  int fd;
  return crash_reporter::GetHandlerSocket(&fd, nullptr) ? fd : -1;
}
#endif  // BUILDFLAG(IS_ANDROID)

// A small ChromeBrowserMainExtraParts that invokes a callback when threads are
// ready. Used to initialize ChromeContentBrowserClient data that needs the UI
// thread.
class RadiumBrowserMainExtraPartsThreadNotifier final
    : public RadiumBrowserMainExtraParts {
 public:
  explicit RadiumBrowserMainExtraPartsThreadNotifier(
      base::OnceClosure threads_ready_closure)
      : threads_ready_closure_(std::move(threads_ready_closure)) {}

  // RadiumBrowserMainExtraParts:
  void PostCreateThreads() final { std::move(threads_ready_closure_).Run(); }

 private:
  base::OnceClosure threads_ready_closure_;
};

bool ShouldDisableForcedColorsForWebContent(content::WebContents* contents,
                                            bool in_forced_colors) {
  // if (!contents || !in_forced_colors) {
  //   return false;
  // }

  // PrefService* prefs =
  //     Profile::FromBrowserContext(contents->GetBrowserContext())->GetPrefs();
  // CHECK(prefs);

  // const base::Value::List& forced_colors_blocklist =
  //     prefs->GetList(prefs::kPageColorsBlockList);

  // if (forced_colors_blocklist.empty()) {
  //   return false;
  // }

  // GURL url = contents->GetLastCommittedURL();

  // // Forced Colors should be disabled for the current URL if it is in the
  // block
  // // list.
  // for (auto const& value : forced_colors_blocklist) {
  //   ContentSettingsPattern pattern =
  //       ContentSettingsPattern::FromString(value.GetString());

  //   if (pattern == ContentSettingsPattern::Wildcard() || !pattern.IsValid())
  //   {
  //     continue;
  //   }

  //   if (pattern.Matches(url)) {
  //     return true;
  //   }
  // }

  return false;
}

bool UpdateForcedColorsForWebContent(blink::web_pref::WebPreferences* web_prefs,
                                     content::WebContents* web_contents,
                                     const ui::NativeTheme* native_theme) {
  auto old_in_forced_colors = web_prefs->in_forced_colors;
  auto old_forced_colors_disabled = web_prefs->is_forced_colors_disabled;
  bool in_forced_colors = native_theme->InForcedColorsMode();
  bool should_disable_forced_colors =
      ShouldDisableForcedColorsForWebContent(web_contents, in_forced_colors);

  web_prefs->in_forced_colors =
      in_forced_colors && !should_disable_forced_colors;
  web_prefs->is_forced_colors_disabled = should_disable_forced_colors;

  return old_in_forced_colors != web_prefs->in_forced_colors ||
         old_forced_colors_disabled != web_prefs->is_forced_colors_disabled;
}

#if !BUILDFLAG(IS_ANDROID)
blink::mojom::PreferredColorScheme ToBlinkPreferredColorScheme(
    ui::NativeTheme::PreferredColorScheme native_theme_scheme) {
  switch (native_theme_scheme) {
    case ui::NativeTheme::PreferredColorScheme::kDark:
      return blink::mojom::PreferredColorScheme::kDark;
    case ui::NativeTheme::PreferredColorScheme::kLight:
      return blink::mojom::PreferredColorScheme::kLight;
  }
}
#endif  // !BUILDFLAG(IS_ANDROID)

// Returns true if preferred color scheme is modified based on at least one of
// the following -
// |url| - Last committed url.
// |web_contents| - For Android based on IsNightModeEnabled().
// |native_theme| - For other platforms based on native theme scheme.
bool UpdatePreferredColorScheme(blink::web_pref::WebPreferences* web_prefs,
                                const GURL& url,
                                content::WebContents* web_contents,
                                const ui::NativeTheme* native_theme) {
  auto old_preferred_color_scheme = web_prefs->preferred_color_scheme;

#if BUILDFLAG(IS_ANDROID)
  auto* delegate = TabAndroid::FromWebContents(web_contents)
                       ? static_cast<TabWebContentsDelegateAndroid*>(
                             web_contents->GetDelegate())
                       : nullptr;
  if (delegate) {
    web_prefs->preferred_color_scheme =
        delegate->IsNightModeEnabled()
            ? blink::mojom::PreferredColorScheme::kDark
            : blink::mojom::PreferredColorScheme::kLight;
    web_prefs->preferred_root_scrollbar_color_scheme =
        web_prefs->preferred_color_scheme;
  }
#else
  // Update based on native theme scheme.
  web_prefs->preferred_color_scheme =
      ToBlinkPreferredColorScheme(native_theme->GetPreferredColorScheme());

  bool using_different_colored_frame = false;
  // if (Profile* profile =
  //         Profile::FromBrowserContext(web_contents->GetBrowserContext())) {
  //   if (ThemeService* theme_service =
  //           ThemeServiceFactory::GetForProfile(profile)) {
  //     using_different_colored_frame = !theme_service->UsingDefaultTheme() ||
  //                                     theme_service->GetUserColor().has_value();
  //   }
  // }

  // Update based on the ColorProvider associated with `web_contents`. Depends
  // on the browser color mode settings and whether the user profile has set a
  // custom coloring for the browser ui.
  web_prefs->preferred_root_scrollbar_color_scheme =
      web_contents->GetColorMode() == ui::ColorProviderKey::ColorMode::kLight ||
              using_different_colored_frame
          ? blink::mojom::PreferredColorScheme::kLight
          : blink::mojom::PreferredColorScheme::kDark;
#endif  // BUILDFLAG(IS_ANDROID)

  // Reauth WebUI doesn't support dark mode yet because it shares the dialog
  // with GAIA web contents that is not correctly themed.
  constexpr bool force_light = false;

  if (force_light) {
    web_prefs->preferred_color_scheme =
        blink::mojom::PreferredColorScheme::kLight;
#if !BUILDFLAG(IS_ANDROID)
  } else if (content::HasWebUIScheme(url)) {
    // If color scheme is not forced, WebUI should track the color mode of the
    // ColorProvider associated with `web_contents`.
    web_prefs->preferred_color_scheme =
        web_contents->GetColorMode() == ui::ColorProviderKey::ColorMode::kLight
            ? blink::mojom::PreferredColorScheme::kLight
            : blink::mojom::PreferredColorScheme::kDark;
#endif  // !BUILDFLAG(IS_ANDROID)
  }

  return old_preferred_color_scheme != web_prefs->preferred_color_scheme;
}

}  // namespace

RadiumContentBrowserClient::RadiumContentBrowserClient()
    : radium_feature_list_creator_(
          std::make_unique<RadiumFeatureListCreator>()) {}

RadiumContentBrowserClient::~RadiumContentBrowserClient() = default;

RadiumFeatureListCreator*
RadiumContentBrowserClient::radium_feature_list_creator() const {
  return radium_feature_list_creator_.get();
}

std::unique_ptr<content::BrowserMainParts>
RadiumContentBrowserClient::CreateBrowserMainParts(bool is_integration_test) {
  std::unique_ptr<RadiumBrowserMainParts> main_parts =
#if BUILDFLAG(IS_WIN)
      std::make_unique<RadiumBrowserMainPartsWin>(
          radium_feature_list_creator_.get());
#elif BUILDFLAG(IS_LINUX)
      std::make_unique<RadiumBrowserMainPartsLinux>(
          radium_feature_list_creator_.get());
#elif BUILDFLAG(IS_MAC)
      std::make_unique<RadiumBrowserMainPartsMac>(
          radium_feature_list_creator_.get());
#elif BUILDFLAG(IS_ANDROID)
      std::make_unique<RadiumBrowserMainPartsAndroid>(
          radium_feature_list_creator_.get());
#else
#error "Unimplemented platform"
#endif

  main_parts->AddParts(
      std::make_unique<RadiumBrowserMainExtraPartsThreadNotifier>(
          base::BindOnce(&RadiumContentBrowserClient::InitOnUIThread,
                         weak_factory_.GetWeakPtr())));

  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsProfiles>());

// Construct additional browser parts. Stages are called in the order in
// which they are added.
#if defined(TOOLKIT_VIEWS)
#if BUILDFLAG(IS_LINUX)
  main_parts->AddParts(
      std::make_unique<RadiumBrowserMainExtraPartsViewsLinux>());
#else
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsViews>());
#endif
#endif

#if BUILDFLAG(IS_MAC)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsMac>());
#endif

#if BUILDFLAG(IS_LINUX)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsLinux>());
#elif BUILDFLAG(IS_OZONE)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsOzone>());
#endif

  return main_parts;
}

void RadiumContentBrowserClient::GetAdditionalWebUISchemes(
    std::vector<std::string>* additional_schemes) {
  additional_schemes->emplace_back(radium::kRadiumUIScheme);
}

std::unique_ptr<content::DevToolsManagerDelegate>
RadiumContentBrowserClient::CreateDevToolsManagerDelegate() {
#if BUILDFLAG(IS_ANDROID)
  return std::make_unique<DevToolsManagerDelegateAndroid>();
#else
  return std::make_unique<RadiumDevToolsManagerDelegate>();
#endif
}

std::unique_ptr<content::WebContentsViewDelegate>
RadiumContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return CreateWebContentsViewDelegate(web_contents);
}

void RadiumContentBrowserClient::OverrideWebPreferences(
    content::WebContents* web_contents,
    content::SiteInstance& main_frame_site,
    blink::web_pref::WebPreferences* web_prefs) {
  UpdateForcedColorsForWebContent(web_prefs, web_contents, GetWebTheme());
  UpdatePreferredColorScheme(
      web_prefs,
      web_contents->GetPrimaryMainFrame()->GetSiteInstance()->GetSiteURL(),
      web_contents, GetWebTheme());
}

bool RadiumContentBrowserClient::OverrideWebPreferencesAfterNavigation(
    content::WebContents* web_contents,
    content::SiteInstance& main_frame_site,
    blink::web_pref::WebPreferences* web_prefs) {
  bool prefs_changed = false;

  prefs_changed |=
      UpdateForcedColorsForWebContent(web_prefs, web_contents, GetWebTheme());

  prefs_changed |=
      UpdatePreferredColorScheme(web_prefs, web_contents->GetLastCommittedURL(),
                                 web_contents, GetWebTheme());

  return prefs_changed;
}

void RadiumContentBrowserClient::RegisterBrowserInterfaceBindersForFrame(
    content::RenderFrameHost* render_frame_host,
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map) {
  radium::internal::PopulateRadiumFrameBinders(map, render_frame_host);
  radium::internal::PopulateRadiumWebUIFrameBinders(map, render_frame_host);
}

void RadiumContentBrowserClient::RegisterWebUIInterfaceBrokers(
    content::WebUIBrowserInterfaceBrokerRegistry& registry) {
  radium::internal::PopulateRadiumWebUIFrameInterfaceBrokers(registry);
}

#if BUILDFLAG(IS_WIN)
void RadiumContentBrowserClient::SessionEnding(
    std::optional<DWORD> control_type) {
  radium::SessionEnding();
}
#endif  // BUILDFLAG(IS_WIN)

void RadiumContentBrowserClient::OnNetworkServiceCreated(
    network::mojom::NetworkService* network_service) {
  PrefService* local_state;
  if (BrowserProcess::Get()) {
    local_state = BrowserProcess::Get()->local_state();
  } else {
    local_state = radium_feature_list_creator_->local_state();
  }
  DCHECK(local_state);

  // Create SystemNetworkContextManager if it has not been created yet. We need
  // to set up global NetworkService state before anything else uses it and this
  // is the first opportunity to initialize SystemNetworkContextManager with the
  // NetworkService.
  if (!SystemNetworkContextManager::HasInstance()) {
    SystemNetworkContextManager::CreateInstance(local_state);
  }

  SystemNetworkContextManager::GetInstance()->OnNetworkServiceCreated(
      network_service);
}

void RadiumContentBrowserClient::ConfigureNetworkContextParams(
    content::BrowserContext* context,
    bool in_memory,
    const base::FilePath& relative_partition_path,
    network::mojom::NetworkContextParams* network_context_params,
    cert_verifier::mojom::CertVerifierCreationParams*
        cert_verifier_creation_params) {
  ProfileNetworkContextService* service =
      ProfileNetworkContextServiceFactory::GetForContext(context);
  if (service) {
    service->ConfigureNetworkContextParams(in_memory, relative_partition_path,
                                           network_context_params,
                                           cert_verifier_creation_params);
  } else {
    // Set default params.
    network_context_params->user_agent = GetUserAgentBasedOnPolicy(context);
    network_context_params->accept_language = GetApplicationLocale();
  }
}

#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)
void RadiumContentBrowserClient::GetAdditionalMappedFilesForChildProcess(
    const base::CommandLine& command_line,
    int child_process_id,
    content::PosixFileDescriptorInfo* mappings) {
#if BUILDFLAG(IS_ANDROID)
  base::MemoryMappedFile::Region region;
  int fd = ui::GetMainAndroidPackFd(&region);
  mappings->ShareWithRegion(kAndroidUIResourcesPakDescriptor, fd, region);

  // For Android: Native resources for DFMs should only be used by the browser
  // process. Their file descriptors and memory mapped file regions are not
  // passed to child processes.

  fd = ui::GetCommonResourcesPackFd(&region);
  mappings->ShareWithRegion(kAndroidChrome100PercentPakDescriptor, fd, region);

  fd = ui::GetLocalePackFd(&region);
  mappings->ShareWithRegion(kAndroidLocalePakDescriptor, fd, region);

  // Optional secondary locale .pak file.
  fd = ui::GetSecondaryLocalePackFd(&region);
  if (fd != -1) {
    mappings->ShareWithRegion(kAndroidSecondaryLocalePakDescriptor, fd, region);
  }

  base::FilePath app_data_path;
  base::PathService::Get(base::DIR_ANDROID_APP_DATA, &app_data_path);
  DCHECK(!app_data_path.empty());
#endif  // BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  int crash_signal_fd = GetCrashSignalFD(command_line);
  if (crash_signal_fd >= 0) {
    mappings->Share(kCrashDumpSignal, crash_signal_fd);
  }
#endif  // BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_LINUX) ||
        // BUILDFLAG(IS_CHROMEOS)
}
#endif  // BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)

base::FilePath RadiumContentBrowserClient::GetFirstPartySetsDirectory() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  DCHECK(!user_data_dir.empty());
  return user_data_dir;
}

base::FilePath RadiumContentBrowserClient::GetGrShaderDiskCacheDirectory() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  DCHECK(!user_data_dir.empty());
  return user_data_dir.Append(FILE_PATH_LITERAL("GrShaderCache"));
}

base::FilePath RadiumContentBrowserClient::GetGraphiteDawnDiskCacheDirectory() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  return user_data_dir.Append(FILE_PATH_LITERAL("GraphiteDawnCache"));
}

std::optional<base::FilePath>
RadiumContentBrowserClient::GetLocalTracesDirectory() {
  base::FilePath user_data_dir;
  if (!base::PathService::Get(radium::DIR_LOCAL_TRACES, &user_data_dir)) {
    return std::nullopt;
  }
  DCHECK(!user_data_dir.empty());
  return user_data_dir;
}

base::Value::Dict RadiumContentBrowserClient::GetNetLogConstants() {
  return net_log::GetPlatformConstantsForNetLog(
      base::CommandLine::ForCurrentProcess()->GetCommandLineString(),
      std::string(version_info::GetChannelString(radium::GetChannel())));
}

base::FilePath RadiumContentBrowserClient::GetNetLogDefaultDirectory() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  DCHECK(!user_data_dir.empty());
  return user_data_dir;
}

std::string RadiumContentBrowserClient::GetProduct() {
  return PRODUCT_SHORTNAME_STRING "/" PRODUCT_VERSION;
}

base::FilePath RadiumContentBrowserClient::GetShaderDiskCacheDirectory() {
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  DCHECK(!user_data_dir.empty());
  return user_data_dir.Append(FILE_PATH_LITERAL("ShaderCache"));
}

std::string RadiumContentBrowserClient::GetUserAgent() {
  return embedder_support::GetUserAgent();
}

void RadiumContentBrowserClient::InitOnUIThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // Initialize `network_contexts_parent_directory_`.
  base::FilePath user_data_dir;
  base::PathService::Get(radium::DIR_USER_DATA, &user_data_dir);
  DCHECK(!user_data_dir.empty());
  network_contexts_parent_directory_.push_back(user_data_dir);

  base::FilePath cache_dir;
  radium::GetUserCacheDirectory(user_data_dir, &cache_dir);
  DCHECK(!cache_dir.empty());
  // On some platforms, the cache is a child of the user_data_dir so only
  // return the one path.
  if (!user_data_dir.IsParent(cache_dir)) {
    network_contexts_parent_directory_.push_back(cache_dir);
  }

  // If the cache location has been overridden by a switch or preference,
  // include that as well.
  if (auto* local_state = BrowserProcess::Get()->local_state()) {
    base::FilePath pref_cache_dir =
        local_state->GetFilePath(prefs::kDiskCacheDir);
    if (!pref_cache_dir.empty() && !user_data_dir.IsParent(cache_dir)) {
      network_contexts_parent_directory_.push_back(pref_cache_dir);
    }
  }
}

const ui::NativeTheme* RadiumContentBrowserClient::GetWebTheme() const {
  return ui::NativeTheme::GetInstanceForWeb();
}

base::FilePath RadiumContentBrowserClient::GetLoggingFileName(
    const base::CommandLine& command_line) {
  return logging::GetLogFileName(command_line);
}

std::vector<base::FilePath>
RadiumContentBrowserClient::GetNetworkContextsParentDirectory() {
  DCHECK(!network_contexts_parent_directory_.empty());
  return network_contexts_parent_directory_;
}

bool RadiumContentBrowserClient::IsShuttingDown() {
  return KeepAliveRegistry::GetInstance()->IsShuttingDown();
}

#if BUILDFLAG(IS_MAC)
bool RadiumContentBrowserClient::SetupEmbedderSandboxParameters(
    sandbox::mojom::Sandbox sandbox_type,
    sandbox::SandboxSerializer* serializer) {
  return false;
}
#endif  // BUILDFLAG(IS_MAC)
