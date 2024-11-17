// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/unload_controller.h"

#include <algorithm>
#include <iterator>

#include "base/containers/contains.h"
#include "base/task/single_thread_task_runner.h"
#include "content/public/browser/web_contents.h"
#include "radium/browser/lifetime/application_lifetime_desktop.h"
#include "radium/browser/ui/browser.h"

namespace DevToolsWindow {

constexpr bool InterceptPageBeforeUnload(content::WebContents*) {
  return false;
}

constexpr bool NeedsToInterceptBeforeUnload(content::WebContents*) {
  return false;
}

}  // namespace DevToolsWindow

UnloadController::UnloadController(Browser* browser)
    : web_contents_collection_(this) {
  browser_observer_.Observe(browser);
}

UnloadController::~UnloadController() = default;

bool UnloadController::BeforeUnloadFired(content::WebContents* contents,
                                         bool proceed) {
  if (!is_attempting_to_close_browser_) {
    if (!proceed) {
      contents->SetClosedByUserGesture(false);
    }
    return proceed;
  }

  if (!proceed) {
    CancelWindowClose();
    contents->SetClosedByUserGesture(false);
    return false;
  }

  if (RemoveFromSet(&tabs_needing_before_unload_fired_, contents)) {
    // Now that beforeunload has fired, put the tab on the queue to fire
    // unload.
    tabs_needing_unload_fired_.insert(contents);
    ProcessPendingTabs(false);
    // We want to handle firing the unload event ourselves since we want to
    // fire all the beforeunload events before attempting to fire the unload
    // events should the user cancel closing the browser.
    return false;
  }

  return true;
}

void UnloadController::CancelWindowClose() {
  tabs_needing_unload_fired_.clear();
  if (is_calling_before_unload_handlers()) {
    std::move(on_close_confirmed_).Run(false);
  }
  is_attempting_to_close_browser_ = false;

  radium::OnClosingAllBrowsers(false);
}

void UnloadController::OnWebContentsCreated(content::WebContents* content) {
  web_contents_collection_.StartObserving(content);
}

void UnloadController::OnWebContentsRemoved(content::WebContents* content) {
  web_contents_collection_.StopObserving(content);
}

void UnloadController::OnWebContentsEmpty() {
  // Set is_attempting_to_close_browser_ here, so that extensions, etc, do not
  // attempt to add tabs to the browser before it closes.
  is_attempting_to_close_browser_ = true;
}

void UnloadController::RenderProcessGone(content::WebContents* web_contents,
                                         base::TerminationStatus status) {
  if (is_attempting_to_close_browser_) {
    // See comment for ClearUnloadState().
    ClearUnloadState(web_contents, false);
  }
  web_contents_collection_.StopObserving(web_contents);
}

UnloadController::UnloadListenerSet
UnloadController::GetTabsNeedingBeforeUnloadFired() const {
  if (!is_attempting_to_close_browser_) {
    CHECK(tabs_needing_unload_fired_.empty());
  }

  UnloadListenerSet tabs_needing_beforeunload;
  std::ranges::for_each(
      browser_observer_.GetSource()->tabs(),
      [this, &tabs_needing_beforeunload](auto& contents) {
        const bool should_fire_beforeunload =
            contents->NeedToFireBeforeUnloadOrUnloadEvents() ||
            DevToolsWindow::NeedsToInterceptBeforeUnload(contents.get());
        if (!base::Contains(tabs_needing_unload_fired_, contents.get()) &&
            should_fire_beforeunload) {
          tabs_needing_beforeunload.insert(contents.get());
        }
      });

  return tabs_needing_beforeunload;
}

void UnloadController::ProcessPendingTabs(bool skip_beforeunload) {
  // Cancel posted/queued ProcessPendingTabs task if there is any.
  weak_factory_.InvalidateWeakPtrs();

  if (!is_attempting_to_close_browser_) {
    // Because we might invoke this after a delay it's possible for the value of
    // is_attempting_to_close_browser_ to have changed since we scheduled the
    // task.
    return;
  }

  if (HasCompletedUnloadProcessing()) {
    tabs_needing_before_unload_fired_ = GetTabsNeedingBeforeUnloadFired();
    if (tabs_needing_before_unload_fired_.empty()) {
      // We've finished all the unload events and can proceed to close the
      // browser.
      // browser_->OnWindowClosing();
      return;
    }
  }

  if (skip_beforeunload) {
    std::ranges::copy(tabs_needing_before_unload_fired_,
                      std::inserter(tabs_needing_unload_fired_,
                                    tabs_needing_unload_fired_.end()));
    tabs_needing_before_unload_fired_.clear();
  }

  // Process beforeunload tabs first. When that queue is empty, process
  // unload tabs.
  if (!tabs_needing_before_unload_fired_.empty()) {
    content::WebContents* const web_contents =
        *(tabs_needing_before_unload_fired_.begin());
    // Null check render_view_host here as this gets called on a PostTask and
    // the tab's render_view_host may have been nulled out.
    if (web_contents->GetPrimaryMainFrame()->GetRenderViewHost()) {
      // If there's a devtools window attached to |web_contents|,
      // we would like devtools to call its own beforeunload handlers first,
      // and then call beforeunload handlers for |web_contents|.
      // See DevToolsWindow::InterceptPageBeforeUnload for details.
      if (!DevToolsWindow::InterceptPageBeforeUnload(web_contents)) {
        web_contents->DispatchBeforeUnload(false /* auto_cancel */);
      }
    } else {
      ClearUnloadState(web_contents, true);
    }
    return;
  }
  if (is_calling_before_unload_handlers()) {
    base::RepeatingCallback<void(bool)> on_close_confirmed =
        on_close_confirmed_;
    // Reset |on_close_confirmed_| in case the callback tests
    // |is_calling_before_unload_handlers()|, we want to return that calling
    // is complete.
    if (tabs_needing_unload_fired_.empty()) {
      on_close_confirmed_.Reset();
    }
    if (!skip_beforeunload) {
      on_close_confirmed.Run(true);
    }
    return;
  }
  CHECK(!tabs_needing_unload_fired_.empty());
  // We've finished firing all beforeunload events and can proceed with unload
  // events.
  // TODO(ojan): We should add a call to browser_shutdown::OnShutdownStarting
  // somewhere around here so that we have accurate measurements of shutdown
  // time.
  std::ranges::for_each(
      tabs_needing_unload_fired_, [this](content::WebContents* web_contents) {
        // Null check render_view_host here as this gets called on a PostTask
        // and the tab's render_view_host may have been nulled out.
        if (web_contents->GetPrimaryMainFrame()->GetRenderViewHost()) {
          web_contents->ClosePage();
        } else {
          ClearUnloadState(web_contents, true);
        }
      });
}

bool UnloadController::HasCompletedUnloadProcessing() const {
  return is_attempting_to_close_browser_ &&
         tabs_needing_before_unload_fired_.empty() &&
         tabs_needing_unload_fired_.empty();
}

bool UnloadController::RemoveFromSet(UnloadListenerSet* set,
                                     content::WebContents* web_contents) {
  DCHECK(is_attempting_to_close_browser_);

  auto iter = base::ranges::find(*set, web_contents);
  if (iter != set->end()) {
    set->erase(iter);
    return true;
  }
  return false;
}

void UnloadController::ClearUnloadState(content::WebContents* web_contents,
                                        bool process_now) {
  if (is_attempting_to_close_browser_) {
    RemoveFromSet(&tabs_needing_before_unload_fired_, web_contents);
    RemoveFromSet(&tabs_needing_unload_fired_, web_contents);
    if (process_now) {
      ProcessPendingTabs(false);
    } else {
      // Do not post a new task if there is already any.
      if (weak_factory_.HasWeakPtrs()) {
        return;
      }
      base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
          FROM_HERE, base::BindOnce(&UnloadController::ProcessPendingTabs,
                                    weak_factory_.GetWeakPtr(), false));
    }
  }
}
