// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "components/constrained_window/constrained_window_views.h"
#include "components/media_router/browser/media_router_dialog_controller.h"
#include "components/ui_devtools/connector_delegate.h"
#include "components/ui_devtools/devtools_server.h"
#include "components/ui_devtools/switches.h"
#include "components/ui_devtools/views/devtools_server_util.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/tracing_service.h"
#include "radium/browser/ui/views/devtools_process_observer.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views.h"
#include "radium/browser/ui/views/radium_constrained_window_views_client.h"
#include "radium/browser/ui/views/radium_layout_provider.h"
#include "radium/browser/ui/views/radium_views_delegate.h"
#include "radium/common/radium_paths.h"
#include "sandbox/policy/switches.h"
#include "ui/display/screen.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/views_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/wm/core/wm_state.h"

#if BUILDFLAG(IS_LINUX)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "content/public/common/content_switches.h"
#include "ui/base/l10n/l10n_util.h"
#endif  // BUILDFLAG(IS_LINUX)

namespace {

// This connector is used in ui_devtools's TracingAgent to hook up with the
// tracing service.
class UiDevtoolsConnector : public ui_devtools::ConnectorDelegate {
 public:
  UiDevtoolsConnector() = default;
  ~UiDevtoolsConnector() override = default;

  void BindTracingConsumerHost(
      mojo::PendingReceiver<tracing::mojom::ConsumerHost> receiver) override {
    content::GetTracingService().BindConsumerHost(std::move(receiver));
  }
};

}  // namespace

RadiumBrowserMainExtraPartsViews::RadiumBrowserMainExtraPartsViews() = default;
RadiumBrowserMainExtraPartsViews::~RadiumBrowserMainExtraPartsViews() = default;

void RadiumBrowserMainExtraPartsViews::ToolkitInitialized() {
  // The delegate needs to be set before any UI is created so that windows
  // display the correct icon.
  if (!views::ViewsDelegate::GetInstance()) {
    views_delegate_ = std::make_unique<RadiumViewsDelegate>();
  }

  SetConstrainedWindowViewsClient(CreateRadiumConstrainedWindowViewsClient());

#if defined(USE_AURA)
  wm_state_ = std::make_unique<wm::WMState>();
#endif

  // TODO(pkasting): Try to move ViewsDelegate creation here as well;
  // see https://crbug.com/691894#c1
  if (!views::LayoutProvider::Get()) {
    layout_provider_ = RadiumLayoutProvider::CreateLayoutProvider();
  }
}

void RadiumBrowserMainExtraPartsViews::PreCreateThreads() {
#if defined(USE_AURA)
  // The Screen instance may already be set in tests.
  if (!display::Screen::GetScreen()) {
    screen_ = views::CreateDesktopScreen();
  }
#endif
}

void RadiumBrowserMainExtraPartsViews::PreProfileInit() {
  if (ui_devtools::UiDevToolsServer::IsUiDevToolsEnabled(
          ui_devtools::switches::kEnableUiDevTools)) {
    CreateUiDevTools();
  }

  // media_router::MediaRouterDialogController::SetGetOrCreate(
  //     base::BindRepeating([](content::WebContents* web_contents) {
  //       DCHECK(web_contents);
  //       media_router::MediaRouterDialogController* controller = nullptr;
  //       // This call does nothing if the controller already exists.
  //       media_router::MediaRouterDialogControllerViews::CreateForWebContents(
  //           web_contents);
  //       controller =
  //           media_router::MediaRouterDialogControllerViews::FromWebContents(
  //               web_contents);
  //       return controller;
  //     }));

#if BUILDFLAG(IS_LINUX)
  // On the Linux desktop, we want to prevent the user from logging in as root,
  // so that we don't destroy the profile. Now that we have some minimal ui
  // initialized, check to see if we're running as root and bail if we are.
  if (geteuid() != 0) {
    return;
  }

  // Allow running inside an unprivileged user namespace. In that case, the
  // root directory will be owned by an unmapped UID and GID (although this
  // may not be the case if a chroot is also being used).
  struct stat st;
  if (stat("/", &st) == 0 && st.st_uid != 0) {
    return;
  }

  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(sandbox::policy::switches::kNoSandbox)) {
    return;
  }

  // std::u16string title = l10n_util::GetStringFUTF16(
  //     IDS_REFUSE_TO_RUN_AS_ROOT,
  //     l10n_util::GetStringUTF16(IDS_PRODUCT_NAME));
  // std::u16string message = l10n_util::GetStringFUTF16(
  //     IDS_REFUSE_TO_RUN_AS_ROOT_2,
  //     l10n_util::GetStringUTF16(IDS_PRODUCT_NAME));

  // chrome::ShowWarningMessageBox(nullptr, title, message);

  // Avoids gpu_process_transport_factory.cc(153)] Check failed:
  // per_compositor_data_.empty() when quit is chosen.
  base::RunLoop().RunUntilIdle();

  exit(EXIT_FAILURE);
#endif  // BUILDFLAG(IS_LINUX)
}

void RadiumBrowserMainExtraPartsViews::PostBrowserStart() {}

void RadiumBrowserMainExtraPartsViews::PostMainMessageLoopRun() {}

void RadiumBrowserMainExtraPartsViews::CreateUiDevTools() {
  DCHECK(!devtools_server_);
  DCHECK(!devtools_process_observer_);

  // Starts the UI Devtools server for browser UI (and Ash UI on Chrome OS).
  auto connector = std::make_unique<UiDevtoolsConnector>();
  base::FilePath output_dir;
  bool result = base::PathService::Get(radium::DIR_USER_DATA, &output_dir);
  DCHECK(result);
  devtools_server_ = ui_devtools::CreateUiDevToolsServerForViews(
      content::GetIOThreadTaskRunner(), std::move(connector), output_dir);
  devtools_process_observer_ = std::make_unique<DevtoolsProcessObserver>(
      devtools_server_->tracing_agent());
}

const ui_devtools::UiDevToolsServer*
RadiumBrowserMainExtraPartsViews::GetUiDevToolsServerInstance() {
  return devtools_server_.get();
}

void RadiumBrowserMainExtraPartsViews::DestroyUiDevTools() {
  devtools_process_observer_.reset();
  devtools_server_.reset();
}
