// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/android/shell.h"

#include <stddef.h>

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "base/command_line.h"
#include "base/functional/callback_helpers.h"
#include "base/location.h"
#include "base/no_destructor.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "components/custom_handlers/protocol_handler.h"
#include "components/custom_handlers/protocol_handler_registry.h"
#include "components/custom_handlers/simple_protocol_handler_registry_factory.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/color_chooser.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/document_picture_in_picture_window_controller.h"
#include "content/public/browser/file_select_listener.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/picture_in_picture_window_controller.h"
#include "content/public/browser/presentation_receiver_flags.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/renderer_preferences_util.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "media/media_buildflags.h"
#include "radium/browser/android/shell_platform_delegate.h"
#include "third_party/blink/public/common/peerconnection/webrtc_ip_handling_policy.h"
#include "third_party/blink/public/common/renderer_preferences/renderer_preferences.h"
#include "third_party/blink/public/mojom/choosers/file_chooser.mojom-forward.h"
#include "third_party/blink/public/mojom/input/pointer_lock_result.mojom.h"
#include "third_party/blink/public/mojom/window_features/window_features.mojom.h"

namespace {
// Null until/unless the default main message loop is running.
base::OnceClosure& GetMainMessageLoopQuitClosure() {
  static base::NoDestructor<base::OnceClosure> closure;
  return *closure;
}

constexpr int kDefaultTestWindowWidthDip = 800;
constexpr int kDefaultTestWindowHeightDip = 600;

// Owning pointer. We can not use unique_ptr as a global. That introduces a
// static constructor/destructor.
// Acquired in Shell::Init(), released in Shell::Shutdown().
ShellPlatformDelegate* g_platform;
}  // namespace

std::vector<Shell*> Shell::windows_;
base::OnceCallback<void(Shell*)> Shell::shell_created_callback_;

Shell::Shell(std::unique_ptr<content::WebContents> web_contents,
             bool should_set_delegate)
    : WebContentsObserver(web_contents.get()),
      web_contents_(std::move(web_contents)) {
  if (should_set_delegate) {
    web_contents_->SetDelegate(this);
  }

  content::UpdateFontRendererPreferencesFromSystemSettings(
      web_contents_->GetMutableRendererPrefs());

  windows_.push_back(this);

  if (shell_created_callback_) {
    std::move(shell_created_callback_).Run(this);
  }
}

Shell::~Shell() {
  g_platform->CleanUp(this);

  for (size_t i = 0; i < windows_.size(); ++i) {
    if (windows_[i] == this) {
      windows_.erase(windows_.begin() + i);
      break;
    }
  }

  web_contents_->SetDelegate(nullptr);
  web_contents_.reset();

  if (windows().empty()) {
    g_platform->DidCloseLastWindow();
  }
}

Shell* Shell::CreateShell(std::unique_ptr<content::WebContents> web_contents,
                          const gfx::Size& initial_size,
                          bool should_set_delegate) {
  content::WebContents* raw_web_contents = web_contents.get();
  Shell* shell = new Shell(std::move(web_contents), should_set_delegate);
  g_platform->CreatePlatformWindow(shell, initial_size);

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kForceWebRtcIPHandlingPolicy)) {
    raw_web_contents->GetMutableRendererPrefs()->webrtc_ip_handling_policy =
        blink::ToWebRTCIPHandlingPolicy(command_line->GetSwitchValueASCII(
            switches::kForceWebRtcIPHandlingPolicy));
  }

  g_platform->SetContents(shell);
  g_platform->DidCreateOrAttachWebContents(shell, raw_web_contents);
  // If the RenderFrame was created during content::WebContents construction (as
  // happens for windows opened from the renderer) then the Shell won't hear
  // about the main frame being created as a WebContentsObservers. This gives
  // the delegate a chance to act on the main frame accordingly.
  if (raw_web_contents->GetPrimaryMainFrame()->IsRenderFrameLive()) {
    g_platform->MainFrameCreated(shell);
  }

  return shell;
}

// static
void Shell::SetMainMessageLoopQuitClosure(base::OnceClosure quit_closure) {
  GetMainMessageLoopQuitClosure() = std::move(quit_closure);
}

// static
void Shell::QuitMainMessageLoopForTesting() {
  auto& quit_loop = GetMainMessageLoopQuitClosure();
  if (quit_loop) {
    std::move(quit_loop).Run();
  }
}

// static
void Shell::SetShellCreatedCallback(
    base::OnceCallback<void(Shell*)> shell_created_callback) {
  DCHECK(!shell_created_callback_);
  shell_created_callback_ = std::move(shell_created_callback);
}

// static
bool Shell::ShouldHideToolbar() {
  return false;
}

// static
Shell* Shell::FromWebContents(content::WebContents* web_contents) {
  for (Shell* window : windows_) {
    if (window->web_contents() && window->web_contents() == web_contents) {
      return window;
    }
  }
  return nullptr;
}

// static
void Shell::Initialize(std::unique_ptr<ShellPlatformDelegate> platform) {
  DCHECK(!g_platform);
  g_platform = platform.release();
  g_platform->Initialize(GetShellDefaultSize());
}

// static
void Shell::Shutdown() {
  if (!g_platform) {  // Shutdown has already been called.
    return;
  }

  content::DevToolsAgentHost::DetachAllClients();

  while (!Shell::windows().empty()) {
    Shell::windows().back()->Close();
  }

  delete g_platform;
  g_platform = nullptr;

  for (auto it = content::RenderProcessHost::AllHostsIterator(); !it.IsAtEnd();
       it.Advance()) {
    it.GetCurrentValue()->DisableRefCounts();
  }
  auto& quit_loop = GetMainMessageLoopQuitClosure();
  if (quit_loop) {
    std::move(quit_loop).Run();
  }

  // Pump the message loop to allow window teardown tasks to run.
  base::RunLoop().RunUntilIdle();
}

gfx::Size Shell::AdjustWindowSize(const gfx::Size& initial_size) {
  if (!initial_size.IsEmpty()) {
    return initial_size;
  }
  return GetShellDefaultSize();
}

// static
Shell* Shell::CreateNewWindow(
    content::BrowserContext* browser_context,
    const GURL& url,
    const scoped_refptr<content::SiteInstance>& site_instance,
    const gfx::Size& initial_size) {
  content::WebContents::CreateParams create_params(browser_context,
                                                   site_instance);
  std::unique_ptr<content::WebContents> web_contents =
      content::WebContents::Create(create_params);
  Shell* shell =
      CreateShell(std::move(web_contents), AdjustWindowSize(initial_size),
                  true /* should_set_delegate */);

  if (!url.is_empty()) {
    shell->LoadURL(url);
  }
  return shell;
}

void Shell::RenderFrameCreated(content::RenderFrameHost* frame_host) {
  if (frame_host == web_contents_->GetPrimaryMainFrame()) {
    g_platform->MainFrameCreated(this);
  }
}

void Shell::LoadURL(const GURL& url) {
  LoadURLForFrame(
      url, std::string(),
      ui::PageTransitionFromInt(ui::PAGE_TRANSITION_TYPED |
                                ui::PAGE_TRANSITION_FROM_ADDRESS_BAR));
}

void Shell::LoadURLForFrame(const GURL& url,
                            const std::string& frame_name,
                            ui::PageTransition transition_type) {
  content::NavigationController::LoadURLParams params(url);
  params.frame_name = frame_name;
  params.transition_type = transition_type;
  web_contents_->GetController().LoadURLWithParams(params);
}

void Shell::LoadDataWithBaseURL(const GURL& url,
                                const std::string& data,
                                const GURL& base_url) {
  bool load_as_string = false;
  LoadDataWithBaseURLInternal(url, data, base_url, load_as_string);
}

#if BUILDFLAG(IS_ANDROID)
void Shell::LoadDataAsStringWithBaseURL(const GURL& url,
                                        const std::string& data,
                                        const GURL& base_url) {
  bool load_as_string = true;
  LoadDataWithBaseURLInternal(url, data, base_url, load_as_string);
}
#endif

void Shell::LoadDataWithBaseURLInternal(const GURL& url,
                                        const std::string& data,
                                        const GURL& base_url,
                                        bool load_as_string) {
#if !BUILDFLAG(IS_ANDROID)
  DCHECK(!load_as_string);  // Only supported on Android.
#endif

  content::NavigationController::LoadURLParams params{GURL()};
  const std::string data_url_header = "data:text/html;charset=utf-8,";
  if (load_as_string) {
    params.url = GURL(data_url_header);
    std::string data_url_as_string = data_url_header + data;
#if BUILDFLAG(IS_ANDROID)
    params.data_url_as_string = base::MakeRefCounted<base::RefCountedString>(
        std::move(data_url_as_string));
#endif
  } else {
    params.url = GURL(data_url_header + data);
  }

  params.load_type = content::NavigationController::LOAD_TYPE_DATA;
  params.base_url_for_data_url = base_url;
  params.virtual_url_for_special_cases = url;
  params.override_user_agent = content::NavigationController::UA_OVERRIDE_FALSE;
  web_contents_->GetController().LoadURLWithParams(params);
}

content::WebContents* Shell::AddNewContents(
    content::WebContents* source,
    std::unique_ptr<content::WebContents> new_contents,
    const GURL& target_url,
    WindowOpenDisposition disposition,
    const blink::mojom::WindowFeatures& window_features,
    bool user_gesture,
    bool* was_blocked) {
#if !BUILDFLAG(IS_ANDROID)
  // If the shell is opening a document picture-in-picture window, it needs to
  // inform the DocumentPictureInPictureWindowController.
  if (disposition == WindowOpenDisposition::NEW_PICTURE_IN_PICTURE) {
    DocumentPictureInPictureWindowController* controller =
        PictureInPictureWindowController::
            GetOrCreateDocumentPictureInPictureController(source);
    controller->SetChildWebContents(new_contents.get());
    controller->Show();
  }
#endif  // !BUILDFLAG(IS_ANDROID)

  CreateShell(
      std::move(new_contents), AdjustWindowSize(window_features.bounds.size()),
      !delay_popup_contents_delegate_for_testing_ /* should_set_delegate */);
  return nullptr;
}

void Shell::GoBackOrForward(int offset) {
  web_contents_->GetController().GoToOffset(offset);
}

void Shell::Reload() {
  web_contents_->GetController().Reload(content::ReloadType::NORMAL, false);
}

void Shell::ReloadBypassingCache() {
  web_contents_->GetController().Reload(content::ReloadType::BYPASSING_CACHE,
                                        false);
}

void Shell::Stop() {
  web_contents_->Stop();
}

void Shell::UpdateNavigationControls(bool should_show_loading_ui) {
  int current_index = web_contents_->GetController().GetCurrentEntryIndex();
  int max_index = web_contents_->GetController().GetEntryCount() - 1;

  g_platform->EnableUIControl(this, ShellPlatformDelegate::BACK_BUTTON,
                              current_index > 0);
  g_platform->EnableUIControl(this, ShellPlatformDelegate::FORWARD_BUTTON,
                              current_index < max_index);
  g_platform->EnableUIControl(
      this, ShellPlatformDelegate::STOP_BUTTON,
      should_show_loading_ui && web_contents_->IsLoading());
}

void Shell::ShowDevTools() {
  // if (!devtools_frontend_) {
  //   auto* devtools_frontend = ShellDevToolsFrontend::Show(web_contents());
  //   devtools_frontend_ = devtools_frontend->GetWeakPtr();
  // }

  // devtools_frontend_->Activate();
}

void Shell::CloseDevTools() {
  // if (!devtools_frontend_) {
  //   return;
  // }
  // devtools_frontend_->Close();
  // devtools_frontend_ = nullptr;
}

void Shell::ResizeWebContentForTests(const gfx::Size& content_size) {
  g_platform->ResizeWebContent(this, content_size);
}

gfx::NativeView Shell::GetContentView() {
  if (!web_contents_) {
    return gfx::NativeView();
  }
  return web_contents_->GetNativeView();
}

#if !BUILDFLAG(IS_ANDROID)
gfx::NativeWindow Shell::window() {
  return g_platform->GetNativeWindow(this);
}
#endif

#if BUILDFLAG(IS_MAC)
void Shell::ActionPerformed(int control) {
  switch (control) {
    case IDC_NAV_BACK:
      GoBackOrForward(-1);
      break;
    case IDC_NAV_FORWARD:
      GoBackOrForward(1);
      break;
    case IDC_NAV_RELOAD:
      Reload();
      break;
    case IDC_NAV_STOP:
      Stop();
      break;
  }
}

void Shell::URLEntered(const std::string& url_string) {
  if (!url_string.empty()) {
    GURL url(url_string);
    if (!url.has_scheme()) {
      url = GURL("http://" + url_string);
    }
    LoadURL(url);
  }
}
#endif

content::WebContents* Shell::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params,
    base::OnceCallback<void(content::NavigationHandle&)>
        navigation_handle_callback) {
  content::WebContents* target = nullptr;
  switch (params.disposition) {
    case WindowOpenDisposition::CURRENT_TAB:
      target = source;
      break;

    // Normally, the difference between NEW_POPUP and NEW_WINDOW is that a popup
    // should have no toolbar, no status bar, no menu bar, no scrollbars and be
    // not resizable.  For simplicity and to enable new testing scenarios in
    // content shell and web tests, popups don't get special treatment below
    // (i.e. they will have a toolbar and other things described here).
    case WindowOpenDisposition::NEW_POPUP:
    case WindowOpenDisposition::NEW_WINDOW:
    // content_shell doesn't really support tabs, but some web tests use
    // middle click (which translates into kNavigationPolicyNewBackgroundTab),
    // so we treat the cases below just like a NEW_WINDOW disposition.
    case WindowOpenDisposition::NEW_BACKGROUND_TAB:
    case WindowOpenDisposition::NEW_FOREGROUND_TAB: {
      Shell* new_window =
          Shell::CreateNewWindow(source->GetBrowserContext(),
                                 GURL(),  // Don't load anything just yet.
                                 params.source_site_instance,
                                 gfx::Size());  // Use default size.
      target = new_window->web_contents();
      break;
    }

    // No tabs in content_shell:
    case WindowOpenDisposition::SINGLETON_TAB:
    // No incognito mode in content_shell:
    case WindowOpenDisposition::OFF_THE_RECORD:
    // TODO(lukasza): Investigate if some web tests might need support for
    // SAVE_TO_DISK disposition.  This would probably require that
    // WebTestControlHost always sets up and cleans up a temporary directory
    // as the default downloads destinations for the duration of a test.
    case WindowOpenDisposition::SAVE_TO_DISK:
    // Ignoring requests with disposition == IGNORE_ACTION...
    case WindowOpenDisposition::IGNORE_ACTION:
    default:
      return nullptr;
  }

  base::WeakPtr<content::NavigationHandle> navigation_handle =
      target->GetController().LoadURLWithParams(
          content::NavigationController::LoadURLParams(params));

  if (navigation_handle_callback && navigation_handle) {
    std::move(navigation_handle_callback).Run(*navigation_handle);
  }

  return target;
}

void Shell::LoadingStateChanged(content::WebContents* source,
                                bool should_show_loading_ui) {
  UpdateNavigationControls(should_show_loading_ui);
  g_platform->SetIsLoading(this, source->IsLoading());
}

#if BUILDFLAG(IS_ANDROID)
void Shell::SetOverlayMode(bool use_overlay_mode) {
  g_platform->SetOverlayMode(this, use_overlay_mode);
}
#endif

void Shell::EnterFullscreenModeForTab(
    content::RenderFrameHost* requesting_frame,
    const blink::mojom::FullscreenOptions& options) {
  ToggleFullscreenModeForTab(
      content::WebContents::FromRenderFrameHost(requesting_frame), true);
}

void Shell::ExitFullscreenModeForTab(content::WebContents* web_contents) {
  ToggleFullscreenModeForTab(web_contents, false);
}

void Shell::ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                       bool enter_fullscreen) {
#if BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_IOS)
  g_platform->ToggleFullscreenModeForTab(this, web_contents, enter_fullscreen);
#endif
  if (is_fullscreen_ != enter_fullscreen) {
    is_fullscreen_ = enter_fullscreen;
    web_contents->GetPrimaryMainFrame()
        ->GetRenderViewHost()
        ->GetWidget()
        ->SynchronizeVisualProperties();
  }
}

bool Shell::IsFullscreenForTabOrPending(
    const content::WebContents* web_contents) {
#if BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_IOS)
  return g_platform->IsFullscreenForTabOrPending(this, web_contents);
#else
  return is_fullscreen_;
#endif
}

blink::mojom::DisplayMode Shell::GetDisplayMode(
    const content::WebContents* web_contents) {
  // TODO: should return blink::mojom::DisplayModeFullscreen wherever user puts
  // a browser window into fullscreen (not only in case of renderer-initiated
  // fullscreen mode): crbug.com/476874.
  return IsFullscreenForTabOrPending(web_contents)
             ? blink::mojom::DisplayMode::kFullscreen
             : blink::mojom::DisplayMode::kBrowser;
}

#if !BUILDFLAG(IS_ANDROID)
void Shell::RegisterProtocolHandler(content::RenderFrameHost* requesting_frame,
                                    const std::string& protocol,
                                    const GURL& url,
                                    bool user_gesture) {
  content::BrowserContext* context = requesting_frame->GetBrowserContext();
  if (context->IsOffTheRecord()) {
    return;
  }

  custom_handlers::ProtocolHandler handler =
      custom_handlers::ProtocolHandler::CreateProtocolHandler(
          protocol, url, GetProtocolHandlerSecurityLevel(requesting_frame));

  // The parameters's normalization process defined in the spec has been already
  // applied in the WebContentImpl class, so at this point it shouldn't be
  // possible to create an invalid handler.
  // https://html.spec.whatwg.org/multipage/system-state.html#normalize-protocol-handler-parameters
  DCHECK(handler.IsValid());

  custom_handlers::ProtocolHandlerRegistry* registry = custom_handlers::
      SimpleProtocolHandlerRegistryFactory::GetForBrowserContext(context, true);
  DCHECK(registry);
  if (registry->SilentlyHandleRegisterHandlerRequest(handler)) {
    return;
  }

  if (!user_gesture && !windows_.empty()) {
    // TODO(jfernandez): This is not strictly needed, but we need a way to
    // inform the observers in browser tests that the request has been
    // cancelled, to avoid timeouts. Chrome just holds the handler as pending in
    // the PageContentSettingsDelegate, but we don't have such thing in the
    // Content Shell.
    registry->OnDenyRegisterProtocolHandler(handler);
    return;
  }

  // FencedFrames can not register to handle any protocols.
  if (requesting_frame->IsNestedWithinFencedFrame()) {
    registry->OnIgnoreRegisterProtocolHandler(handler);
    return;
  }

  // TODO(jfernandez): Are we interested at all on using the
  // PermissionRequestManager in the ContentShell ?
  if (registry->registration_mode() ==
      custom_handlers::RphRegistrationMode::kAutoAccept) {
    registry->OnAcceptRegisterProtocolHandler(handler);
  }
}
#endif

void Shell::RequestPointerLock(content::WebContents* web_contents,
                               bool user_gesture,
                               bool last_unlocked_by_target) {
  // Give the platform a chance to handle the lock request, if it doesn't
  // indicate it handled it, allow the request.
  if (!g_platform->HandlePointerLockRequest(this, web_contents, user_gesture,
                                            last_unlocked_by_target)) {
    web_contents->GotResponseToPointerLockRequest(
        blink::mojom::PointerLockResult::kSuccess);
  }
}

void Shell::Close() {
  // Shell is "self-owned" and destroys itself. The ShellPlatformDelegate
  // has the chance to co-opt this and do its own destruction.
  if (!g_platform->DestroyShell(this)) {
    delete this;
  }
}

void Shell::CloseContents(content::WebContents* source) {
  Close();
}

bool Shell::CanOverscrollContent() {
#if defined(USE_AURA)
  return true;
#else
  return false;
#endif
}

void Shell::NavigationStateChanged(content::WebContents* source,
                                   content::InvalidateTypes changed_flags) {
  if (changed_flags & content::INVALIDATE_TYPE_URL) {
    g_platform->SetAddressBarURL(this, source->GetVisibleURL());
  }
}

content::JavaScriptDialogManager* Shell::GetJavaScriptDialogManager(
    content::WebContents* source) {
  // if (!dialog_manager_) {
  //   dialog_manager_ = g_platform->CreateJavaScriptDialogManager(this);
  // }
  // if (!dialog_manager_) {
  //   dialog_manager_ = std::make_unique<ShellJavaScriptDialogManager>();
  // }
  // return dialog_manager_.get();
  return nullptr;
}

#if BUILDFLAG(IS_MAC)
void Shell::PrimaryPageChanged(Page& page) {
  g_platform->DidNavigatePrimaryMainFramePostCommit(
      this, content::WebContents::FromRenderFrameHost(&page.GetMainDocument()));
}

bool Shell::HandleKeyboardEvent(content::WebContents* source,
                                const input::NativeWebKeyboardEvent& event) {
  return g_platform->HandleKeyboardEvent(this, source, event);
}
#endif

bool Shell::DidAddMessageToConsole(content::WebContents* source,
                                   blink::mojom::ConsoleMessageLevel log_level,
                                   const std::u16string& message,
                                   int32_t line_no,
                                   const std::u16string& source_id) {
  return false;
}

void Shell::RendererUnresponsive(
    content::WebContents* source,
    content::RenderWidgetHost* render_widget_host,
    base::RepeatingClosure hang_monitor_restarter) {
  LOG(WARNING) << "renderer unresponsive";
}

void Shell::ActivateContents(content::WebContents* contents) {
#if !BUILDFLAG(IS_MAC)
  // TODO(danakj): Move this to ShellPlatformDelegate.
  contents->Focus();
#else
  // Mac headless mode is quite different than other platforms. Normally
  // focusing the content::WebContents would cause the OS to focus the window.
  // Because headless mac doesn't actually have system windows, we can't go down
  // the normal path and have to fake it out in the browser process.
  g_platform->ActivateContents(this, contents);
#endif
}

#if BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_IOS)
std::unique_ptr<content::ColorChooser> Shell::OpenColorChooser(
    content::WebContents* web_contents,
    SkColor color,
    const std::vector<blink::mojom::ColorSuggestionPtr>& suggestions) {
  return g_platform->OpenColorChooser(web_contents, color, suggestions);
}
#endif

void Shell::RunFileChooser(content::RenderFrameHost* render_frame_host,
                           scoped_refptr<content::FileSelectListener> listener,
                           const blink::mojom::FileChooserParams& params) {
  run_file_chooser_count_++;
  if (hold_file_chooser_) {
    held_file_chooser_listener_ = std::move(listener);
  } else {
    g_platform->RunFileChooser(render_frame_host, std::move(listener), params);
  }
}

void Shell::EnumerateDirectory(
    content::WebContents* web_contents,
    scoped_refptr<content::FileSelectListener> listener,
    const base::FilePath& path) {
  run_file_chooser_count_++;
  if (hold_file_chooser_) {
    held_file_chooser_listener_ = std::move(listener);
  } else {
    listener->FileSelectionCanceled();
  }
}

bool Shell::IsBackForwardCacheSupported(content::WebContents& web_contents) {
  return true;
}

content::PreloadingEligibility Shell::IsPrerender2Supported(
    content::WebContents& web_contents,
    content::PreloadingTriggerType trigger_type) {
  return content::PreloadingEligibility::kEligible;
}

namespace {
class PendingCallback : public base::RefCounted<PendingCallback> {
 public:
  explicit PendingCallback(base::OnceCallback<void()> cb)
      : callback_(std::move(cb)) {}

 private:
  friend class base::RefCounted<PendingCallback>;
  ~PendingCallback() { std::move(callback_).Run(); }
  base::OnceCallback<void()> callback_;
};
}  // namespace

bool Shell::ShouldAllowRunningInsecureContent(
    content::WebContents* web_contents,
    bool allowed_per_prefs,
    const url::Origin& origin,
    const GURL& resource_url) {
  if (allowed_per_prefs) {
    return true;
  }

  return g_platform->ShouldAllowRunningInsecureContent(this);
}

content::PictureInPictureResult Shell::EnterPictureInPicture(
    content::WebContents* web_contents) {
  return content::PictureInPictureResult::kNotSupported;
}

bool Shell::ShouldResumeRequestsForCreatedWindow() {
  return !delay_popup_contents_delegate_for_testing_;
}

void Shell::SetContentsBounds(content::WebContents* source,
                              const gfx::Rect& bounds) {
  DCHECK(source ==
         web_contents());  // There's only one content::WebContents per Shell.
}

gfx::Size Shell::GetShellDefaultSize() {
  static gfx::Size default_shell_size;  // Only go through this method once.

  if (!default_shell_size.IsEmpty()) {
    return default_shell_size;
  }

  if (default_shell_size.IsEmpty()) {
    default_shell_size =
        gfx::Size(kDefaultTestWindowWidthDip, kDefaultTestWindowHeightDip);
  }

  return default_shell_size;
}

#if BUILDFLAG(IS_ANDROID)
void Shell::LoadProgressChanged(double progress) {
  g_platform->LoadProgressChanged(this, progress);
}
#endif

void Shell::TitleWasSet(content::NavigationEntry* entry) {
  if (entry) {
    g_platform->SetTitle(this, entry->GetTitle());
  }
}
