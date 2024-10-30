// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_desktop_window_tree_host_win.h"

#include <shobjidl.h>
#include <windows.h>

#include <combaseapi.h>
#include <wrl/client.h>

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/memory/ref_counted_delete_on_sequence.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/win/windows_types.h"
#include "base/win/windows_version.h"
#include "components/policy/core/common/policy_pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "radium/browser/browser_process.h"
#include "radium/browser/ui/views/frame/untitled_widget.h"
#include "ui/base/win/hwnd_metrics.h"
#include "ui/display/win/screen_win.h"

class VirtualDesktopHelper
    : public base::RefCountedDeleteOnSequence<VirtualDesktopHelper> {
 public:
  using WorkspaceChangedCallback = base::OnceCallback<void()>;

  explicit VirtualDesktopHelper(const std::string& initial_workspace);
  VirtualDesktopHelper(const VirtualDesktopHelper&) = delete;
  VirtualDesktopHelper& operator=(const VirtualDesktopHelper&) = delete;

  // public methods are all called on the UI thread.
  void Init(HWND hwnd);

  std::string GetWorkspace();

  // |callback| is called when the task to get the desktop id of |hwnd|
  // completes, if the workspace has changed.
  void UpdateWindowDesktopId(HWND hwnd, WorkspaceChangedCallback callback);

  bool GetInitialWorkspaceRemembered() const;

  void SetInitialWorkspaceRemembered(bool remembered);

 private:
  friend class base::RefCountedDeleteOnSequence<VirtualDesktopHelper>;
  friend class base::DeleteHelper<VirtualDesktopHelper>;

  ~VirtualDesktopHelper();

  // Called on the UI thread as a task reply.
  void SetWorkspace(WorkspaceChangedCallback callback,
                    const std::string& workspace);

  void InitImpl(HWND hwnd, const std::string& initial_workspace);

  static std::string GetWindowDesktopIdImpl(
      HWND hwnd,
      Microsoft::WRL::ComPtr<IVirtualDesktopManager> virtual_desktop_manager);

  // All member variables, except where noted, are only accessed on the ui
  // thead.

  // Workspace browser window was opened on. This is used to tell the
  // BrowserWindowState about the initial workspace, which has to happen after
  // |this| is fully set up.
  const std::string initial_workspace_;

  // On Windows10, this is the virtual desktop the browser window was on,
  // last we checked. This is used to tell if the window has moved to a
  // different desktop, and notify listeners. It will only be set if
  // we created |virtual_desktop_helper_|.
  std::optional<std::string> workspace_;

  bool initial_workspace_remembered_ = false;

  // Only set on Windows 10. This is created and accessed on a separate
  // COMSTAT thread. It will be null if creation failed.
  Microsoft::WRL::ComPtr<IVirtualDesktopManager> virtual_desktop_manager_;

  base::WeakPtrFactory<VirtualDesktopHelper> weak_factory_{this};
};

VirtualDesktopHelper::VirtualDesktopHelper(const std::string& initial_workspace)
    : base::RefCountedDeleteOnSequence<VirtualDesktopHelper>(
          base::ThreadPool::CreateCOMSTATaskRunner(
              {base::MayBlock(),
               base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN})),
      initial_workspace_(initial_workspace) {}

void VirtualDesktopHelper::Init(HWND hwnd) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  owning_task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&VirtualDesktopHelper::InitImpl, this, hwnd,
                                initial_workspace_));
}

VirtualDesktopHelper::~VirtualDesktopHelper() {}

std::string VirtualDesktopHelper::GetWorkspace() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  if (!workspace_.has_value()) {
    workspace_ = initial_workspace_;
  }

  return workspace_.value_or(std::string());
}

void VirtualDesktopHelper::UpdateWindowDesktopId(
    HWND hwnd,
    WorkspaceChangedCallback callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  owning_task_runner()->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&VirtualDesktopHelper::GetWindowDesktopIdImpl, hwnd,
                     virtual_desktop_manager_),
      base::BindOnce(&VirtualDesktopHelper::SetWorkspace, this,
                     std::move(callback)));
}

bool VirtualDesktopHelper::GetInitialWorkspaceRemembered() const {
  return initial_workspace_remembered_;
}

void VirtualDesktopHelper::SetInitialWorkspaceRemembered(bool remembered) {
  initial_workspace_remembered_ = remembered;
}

void VirtualDesktopHelper::SetWorkspace(WorkspaceChangedCallback callback,
                                        const std::string& workspace) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  // If GetWindowDesktopId() fails, |workspace| will be empty, and it's most
  // likely that the current value of |workspace_| is still correct, so don't
  // overwrite it.
  if (workspace.empty()) {
    return;
  }

  bool workspace_changed = workspace != workspace_.value_or(std::string());
  workspace_ = workspace;
  if (workspace_changed) {
    std::move(callback).Run();
  }
}

void VirtualDesktopHelper::InitImpl(HWND hwnd,
                                    const std::string& initial_workspace) {
  DCHECK(!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  // Virtual Desktops on Windows are best-effort and may not always be
  // available.
  if (FAILED(::CoCreateInstance(__uuidof(::VirtualDesktopManager), nullptr,
                                CLSCTX_ALL,
                                IID_PPV_ARGS(&virtual_desktop_manager_))) ||
      initial_workspace.empty()) {
    return;
  }
  GUID guid = GUID_NULL;
  HRESULT hr =
      CLSIDFromString(base::UTF8ToWide(initial_workspace).c_str(), &guid);
  if (SUCCEEDED(hr)) {
    // There are valid reasons MoveWindowToDesktop can fail, e.g.,
    // the desktop was deleted. If it fails, the window will open on the
    // current desktop.
    virtual_desktop_manager_->MoveWindowToDesktop(hwnd, guid);
  }
}

// static
std::string VirtualDesktopHelper::GetWindowDesktopIdImpl(
    HWND hwnd,
    Microsoft::WRL::ComPtr<IVirtualDesktopManager> virtual_desktop_manager) {
  DCHECK(!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  if (!virtual_desktop_manager) {
    return std::string();
  }

  GUID workspace_guid;
  HRESULT hr =
      virtual_desktop_manager->GetWindowDesktopId(hwnd, &workspace_guid);
  if (FAILED(hr) || workspace_guid == GUID_NULL) {
    return std::string();
  }

  LPOLESTR workspace_widestr;
  StringFromCLSID(workspace_guid, &workspace_widestr);
  std::string workspace_id = base::WideToUTF8(workspace_widestr);
  CoTaskMemFree(workspace_widestr);
  return workspace_id;
}

UntitledDesktopWindowTreeHostWin::UntitledDesktopWindowTreeHostWin(
    views::internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura,
    UntitledWidget* untitled_widget)
    : views::DesktopWindowTreeHostWin(native_widget_delegate,
                                      desktop_native_widget_aura),
      untitled_widget_(untitled_widget) {
  // TODO(crbug.com/40118412) Make turning off this policy turn off
  // native window occlusion on this browser win.
  if (!BrowserProcess::Get()->local_state()->GetBoolean(
          policy::policy_prefs::kNativeWindowOcclusionEnabled)) {
    SetNativeWindowOcclusionEnabled(false);
  }
}

UntitledDesktopWindowTreeHostWin::~UntitledDesktopWindowTreeHostWin() = default;

views::DesktopWindowTreeHost*
UntitledDesktopWindowTreeHostWin::AsDesktopWindowTreeHost() {
  return this;
}

int UntitledDesktopWindowTreeHostWin::GetMinimizeButtonOffset() const {
  return 0;
}

bool UntitledDesktopWindowTreeHostWin::UsesNativeSystemMenu() const {
  return true;
}

void UntitledDesktopWindowTreeHostWin::Init(
    const views::Widget::InitParams& params) {
  DesktopWindowTreeHostWin::Init(params);
  virtual_desktop_helper_ = new VirtualDesktopHelper(params.workspace);
  virtual_desktop_helper_->Init(GetHWND());
}

void UntitledDesktopWindowTreeHostWin::Show(
    ui::mojom::WindowShowState show_state,
    const gfx::Rect& restore_bounds) {  // This will make BrowserWindowState
                                        // remember the initial workspace.
  // It has to be called after DesktopNativeWidgetAura is observing the host
  // and the session service is tracking the window.
  if (virtual_desktop_helper_ &&
      !virtual_desktop_helper_->GetInitialWorkspaceRemembered()) {
    // If |virtual_desktop_helper_| has an empty workspace, kick off an update,
    // which will eventually call OnHostWorkspaceChanged.
    if (virtual_desktop_helper_->GetWorkspace().empty()) {
      UpdateWorkspace();
    } else {
      OnHostWorkspaceChanged();
    }
  }
  DesktopWindowTreeHostWin::Show(show_state, restore_bounds);
}

std::string UntitledDesktopWindowTreeHostWin::GetWorkspace() const {
  return virtual_desktop_helper_ ? virtual_desktop_helper_->GetWorkspace()
                                 : std::string();
}

int UntitledDesktopWindowTreeHostWin::GetInitialShowState() const {
  STARTUPINFO si = {0};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  GetStartupInfo(&si);
  return si.wShowWindow;
}

bool UntitledDesktopWindowTreeHostWin::GetClientAreaInsets(
    gfx::Insets* insets,
    HMONITOR monitor) const {
  // Always use default insets for opaque frame.
  if (!ShouldUseNativeFrame()) {
    return false;
  }

  // Use default insets for popups and apps, unless we are custom drawing the
  // titlebar.
  if (!untitled_widget_->ShouldBrowserCustomDrawTitlebar()) {
    return false;
  }

  if (GetWidget()->IsFullscreen()) {
    // In fullscreen mode there is no frame.
    *insets = gfx::Insets();
  } else {
    const int frame_thickness = ui::GetFrameThickness(monitor);
    // Reduce the non-client border size; UpdateDWMFrame() will instead extend
    // the border into the window client area. For maximized windows, Windows
    // outdents the window rect from the screen's client rect by
    // |frame_thickness| on each edge, meaning |insets| must contain
    // |frame_thickness| on all sides (including the top) to avoid the client
    // area extending onto adjacent monitors. For non-maximized windows,
    // however, the top inset must be zero, since if there is any nonclient
    // area, Windows will draw a full native titlebar outside the client area.
    // (This doesn't occur in the maximized case.)
    int top_thickness = 0;
    if (untitled_widget_->ShouldBrowserCustomDrawTitlebar() &&
        GetWidget()->IsMaximized()) {
      top_thickness = frame_thickness;
    }
    *insets = gfx::Insets::TLBR(top_thickness, frame_thickness, frame_thickness,
                                frame_thickness);
  }
  return true;
}

bool UntitledDesktopWindowTreeHostWin::GetDwmFrameInsetsInPixels(
    gfx::Insets* insets) const {
  // For "normal" windows on Aero, we always need to reset the glass area
  // correctly, even if we're not currently showing the native frame (e.g.
  // because a theme is showing), so we explicitly check for that case rather
  // than checking ShouldUseNativeFrame() here.  Using that here would mean we
  // wouldn't reset the glass area to zero when moving from the native frame to
  // an opaque frame, leading to graphical glitches behind the opaque frame.
  // Instead, we use that function below to tell us whether the frame is
  // currently native or opaque.
  if (!GetWidget()->client_view() ||
      !DesktopWindowTreeHostWin::ShouldUseNativeFrame()) {
    return false;
  }

  // Don't extend the glass in at all if it won't be visible.
  if (!ShouldUseNativeFrame() || GetWidget()->IsFullscreen() ||
      untitled_widget_->ShouldBrowserCustomDrawTitlebar()) {
    *insets = gfx::Insets();
  } else {
    // The glass should extend to the bottom of the tabstrip.
    gfx::Size tabstrip_region_size(1, untitled_widget_->GetGlassHeight());
    tabstrip_region_size = display::win::ScreenWin::DIPToScreenSize(
        GetHWND(), tabstrip_region_size);

    *insets = gfx::Insets::TLBR(tabstrip_region_size.height(), 0, 0, 0);
  }
  return true;
}

void UntitledDesktopWindowTreeHostWin::PostHandleMSG(UINT message,
                                                     WPARAM w_param,
                                                     LPARAM l_param) {
  switch (message) {
    case WM_SETFOCUS: {
      UpdateWorkspace();
      break;
    }
    // case WM_CREATE:
    //   minimize_button_metrics_.Init(GetHWND());
    //   break;
    case WM_WINDOWPOSCHANGED: {
      // Windows lies to us about the position of the minimize button before a
      // window is visible. We use this position to place the incognito avatar
      // in RTL mode, so when the window is shown, we need to re-layout and
      // schedule a paint for the non-client frame view so that the icon top has
      // the correct position when the window becomes visible. This fixes bugs
      // where the icon appears to overlay the minimize button. Note that we
      // will call Layout every time SetWindowPos is called with SWP_SHOWWINDOW,
      // however callers typically are careful about not specifying this flag
      // unless necessary to avoid flicker. This may be invoked during creation
      // on XP and before the non_client_view has been created.
      WINDOWPOS* window_pos = reinterpret_cast<WINDOWPOS*>(l_param);
      views::NonClientView* non_client_view = GetWidget()->non_client_view();
      if (window_pos->flags & SWP_SHOWWINDOW && non_client_view) {
        non_client_view->DeprecatedLayoutImmediately();
        non_client_view->SchedulePaint();
      }
      break;
    }
    case WM_DWMCOLORIZATIONCOLORCHANGED: {
      // The activation border may have changed color.
      views::NonClientView* non_client_view = GetWidget()->non_client_view();
      if (non_client_view) {
        non_client_view->SchedulePaint();
      }
      break;
    }
  }
}

void UntitledDesktopWindowTreeHostWin::UpdateWorkspace() {
  if (!virtual_desktop_helper_) {
    return;
  }
  virtual_desktop_helper_->UpdateWindowDesktopId(
      GetHWND(),
      base::BindOnce(&UntitledDesktopWindowTreeHostWin::OnHostWorkspaceChanged,
                     weak_factory_.GetWeakPtr()));
}

////////////////////////////////////////////////////////////////////////////////
// UntitledDesktopWindowTreeHost, public:

// static
UntitledDesktopWindowTreeHost*
UntitledDesktopWindowTreeHost::CreateBrowserDesktopWindowTreeHost(
    views::internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura,
    UntitledWidget* untitled_widget) {
  return new UntitledDesktopWindowTreeHostWin(
      native_widget_delegate, desktop_native_widget_aura, untitled_widget);
}
