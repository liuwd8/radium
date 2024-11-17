// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_UNLOAD_CONTROLLER_H_
#define RADIUM_BROWSER_UI_UNLOAD_CONTROLLER_H_

#include <set>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/tab_contents/web_contents_collection.h"
#include "radium/browser/ui/browser_observer.h"

class Browser;

namespace content {
class WebContents;
}  // namespace content

class UnloadController : public BrowserObserver,
                         public WebContentsCollection::Observer {
 public:
  explicit UnloadController(Browser* browser);
  UnloadController(const UnloadController&) = delete;
  UnloadController& operator=(const UnloadController&) = delete;

  ~UnloadController() override;

  // Returns true if |contents| can be cleanly closed. When |browser_| is being
  // closed, this function will return false to indicate |contents| should not
  // be cleanly closed, since the fast shutdown path will just kill its
  // renderer.
  bool CanCloseContents(content::WebContents* contents);

  // Called when a BeforeUnload handler is fired for |contents|. |proceed|
  // indicates the user's response to the Y/N BeforeUnload handler dialog. If
  // this parameter is false, any pending attempt to close the whole browser
  // will be canceled. Returns true if Unload handlers should be fired. When the
  // |browser_| is being closed, Unload handlers for any particular WebContents
  // will not be run until every WebContents being closed has a chance to run
  // its BeforeUnloadHandler.
  bool BeforeUnloadFired(content::WebContents* contents, bool proceed);

  bool is_calling_before_unload_handlers() {
    return !on_close_confirmed_.is_null();
  }

  // Clears all the state associated with processing tabs' beforeunload/unload
  // events since the user cancelled closing the window.
  void CancelWindowClose();

 private:
  // BrowserObserver:
  void OnWebContentsAdded(content::WebContents*) override;
  void OnWebContentsRemoved(content::WebContents*) override;
  void OnWebContentsEmpty() override;

  // WebContentsCollection::Observer:
  void RenderProcessGone(content::WebContents* web_contents,
                         base::TerminationStatus status) override;

 private:
  typedef std::set<raw_ptr<content::WebContents>> UnloadListenerSet;

  UnloadListenerSet GetTabsNeedingBeforeUnloadFired() const;

  // Processes the next tab that needs it's beforeunload/unload event fired.
  void ProcessPendingTabs(bool skip_beforeunload);

  // Whether we've completed firing all the tabs' beforeunload/unload events.
  bool HasCompletedUnloadProcessing() const;

  // Removes |web_contents| from the passed |set|.
  // Returns whether the tab was in the set in the first place.
  bool RemoveFromSet(UnloadListenerSet* set,
                     content::WebContents* web_contents);

  // Cleans up state appropriately when we are trying to close the browser and
  // the tab has finished firing its unload handler. We also use this in the
  // cases where a tab crashes or hangs even if the beforeunload/unload haven't
  // successfully fired. If |process_now| is true |ProcessPendingTabs| is
  // invoked immediately, otherwise it is invoked after a delay (PostTask).
  //
  // Typically you'll want to pass in true for |process_now|. Passing in true
  // may result in deleting |tab|. If you know that shouldn't happen (because of
  // the state of the stack), pass in false.
  void ClearUnloadState(content::WebContents* web_contents, bool process_now);

  WebContentsCollection web_contents_collection_;

  // Tracks tabs that need their beforeunload event fired before we can
  // close the browser. Only gets populated when we try to close the browser.
  UnloadListenerSet tabs_needing_before_unload_fired_;

  // Tracks tabs that need their unload event fired before we can
  // close the browser. Only gets populated when we try to close the browser.
  UnloadListenerSet tabs_needing_unload_fired_;

  base::ScopedObservation<Browser, BrowserObserver> browser_observer_{this};

  // Whether we are processing the beforeunload and unload events of each tab
  // in preparation for closing the browser. UnloadController owns this state
  // rather than Browser because unload handlers are the only reason that a
  // Browser window isn't just immediately closed.
  bool is_attempting_to_close_browser_ = false;

  // A callback to call to report whether the user chose to close all tabs of
  // |browser_| that have beforeunload event handlers. This is set only if we
  // are currently confirming that the browser is closable. This can be called
  // more than once if a user confirms all the beforeunload prompts (at which
  // point it will be called with true) but the window close is later aborted
  // (at which point it will be called with false). This can happen when
  // multiple browser windows are being closed together. See
  // BrowserList::TryToCloseBrowserList.
  base::RepeatingCallback<void(bool)> on_close_confirmed_;

  base::WeakPtrFactory<UnloadController> weak_factory_{this};
};

#endif  // RADIUM_BROWSER_UI_UNLOAD_CONTROLLER_H_
