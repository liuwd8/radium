// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_H_
#define RADIUM_BROWSER_UI_BROWSER_H_

#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "radium/browser/ui/unload_controller.h"

class BrowserObserver;
class BrowserWindow;
class ScopedKeepAlive;

class Browser : public content::WebContentsDelegate,
                public base::SupportsUserData {
 public:
  using WebContentsSet = base::flat_set<std::unique_ptr<content::WebContents>>;
  typedef BrowserWindow* (*CreateBrowserWindow)(std::unique_ptr<Browser>);

  struct CreateParams {
    // The associated profile.
    raw_ptr<Profile> profile = nullptr;

    // Function used to create an associated window.
    CreateBrowserWindow new_window = nullptr;
  };

  static Browser* Create(CreateParams params);

  Browser(const Browser&) = delete;
  Browser& operator=(const Browser&) = delete;

  ~Browser() override;

  // Tells the browser whether it should skip showing any dialogs that ask the
  // user to confirm that they want to close the browser when it is being
  // closed.
  void set_force_skip_warning_user_on_close(
      bool force_skip_warning_user_on_close) {
    force_skip_warning_user_on_close_ = force_skip_warning_user_on_close;
  }

  Profile* profile() const { return profile_; }
  BrowserWindow* window() const { return window_; }

  const WebContentsSet& tabs() const { return tabs_; }

  // Used to register a KeepAlive to affect the Chrome lifetime. The KeepAlive
  // is registered when the browser is added to the browser list, and unregisted
  // when it is removed from it.
  void RegisterKeepAlive();
  void UnregisterKeepAlive();

  void AddObserver(BrowserObserver* observer);
  void RemoveObserver(BrowserObserver* observer);

  // Create WebContents on the current browser object. This will associate the
  // WebContents with the lifecycle of the current browser. When the browser
  // object is destroyed, the corresponding WebContents will also be destroyed.
  content::WebContents* CreateWebContents();

  void AddWebContents(std::unique_ptr<content::WebContents> web_contents);
  std::unique_ptr<content::WebContents> RemoveWebContents(
      content::WebContents* web_contents);

  // Begins the process of confirming whether the associated browser can be
  // closed. If there are no tabs with beforeunload handlers it will immediately
  // return false. If |skip_beforeunload| is true, all beforeunload
  // handlers will be skipped and window closing will be confirmed without
  // showing the prompt, the function will return false as well.
  // Otherwise, it starts prompting the user, returns true and will call
  // |on_close_confirmed| with the result of the user's decision.
  // After calling this function, if the window will not be closed, call
  // ResetBeforeUnloadHandlers() to reset all beforeunload handlers; calling
  // this function multiple times without an intervening call to
  // ResetTryToCloseWindow() will run only the beforeunload handlers
  // registered since the previous call.
  // Note that if the browser window has been used before, users should always
  // have a chance to save their work before the window is closed without
  // triggering beforeunload event.
  bool TryToCloseWindow(
      bool skip_beforeunload,
      const base::RepeatingCallback<void(bool)>& on_close_confirmed);

  // Clears the results of any beforeunload confirmation dialogs triggered by a
  // TryToCloseWindow call.
  void ResetTryToCloseWindow();

  // Figure out if there are tabs that have beforeunload handlers.
  bool TabsNeedBeforeUnloadFired() const;

  // Invoked when the window containing us is closing. Performs the necessary
  // cleanup.
  void OnWindowClosing();

  bool ShouldRunUnloadListenerBeforeClosing(content::WebContents* web_contents);
  bool RunUnloadListenerBeforeClosing(content::WebContents* web_contents);

 private:
  // content::WebContentsDelegate:
  void BeforeUnloadFired(content::WebContents* source,
                         bool proceed,
                         bool* proceed_to_fire_unload) override;
  void CloseContents(content::WebContents* source) override;
  bool ShouldFocusLocationBarByDefault(content::WebContents* source) override;
  void SetFocusToLocationBar() override;

 private:
  explicit Browser(CreateParams params);

  // In-progress download termination handling /////////////////////////////////

  enum CancelDownloadConfirmationState {
    NOT_PROMPTED,          // We have not asked the user.
    WAITING_FOR_RESPONSE,  // We have asked the user and have not received a
                           // reponse yet.
    RESPONSE_RECEIVED      // The user was prompted and made a decision already.
  };

  // State used to figure-out whether we should prompt the user for confirmation
  // when the browser is closed with in-progress downloads.
  CancelDownloadConfirmationState cancel_download_confirmation_state_;

  // The associated profile.
  const raw_ptr<Profile> profile_;

  // Must be constructed before `unload_controller_`. Because unload_controller_
  // will add itself to observers_ in the constructor
  base::ObserverList<BrowserObserver> observers_;

  std::unique_ptr<ScopedKeepAlive> keep_alive_;

  // Tells if the browser should skip warning the user when closing the window.
  bool force_skip_warning_user_on_close_ = false;

  WebContentsSet tabs_;

  raw_ptr<BrowserWindow> window_ = nullptr;

  UnloadController unload_controller_;
};

#endif  // RADIUM_BROWSER_UI_BROWSER_H_
