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

  // Invoked when the window containing us is closing. Performs the necessary
  // cleanup.
  void OnWindowClosing();

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

  // The associated profile.
  const raw_ptr<Profile> profile_;

  // Must be constructed before `unload_controller_`. Because unload_controller_
  // will add itself to observers_ in the constructor
  base::ObserverList<BrowserObserver> observers_;

  std::unique_ptr<ScopedKeepAlive> keep_alive_;

  WebContentsSet tabs_;

  raw_ptr<BrowserWindow> window_ = nullptr;

  UnloadController unload_controller_;
};

#endif  // RADIUM_BROWSER_UI_BROWSER_H_
