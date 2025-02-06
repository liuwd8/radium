// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/browser_list.h"

#include "base/no_destructor.h"
#include "base/observer_list.h"
#include "radium/browser/ui/browser.h"
#include "radium/browser/ui/browser_list_observer.h"

// static
BrowserList* BrowserList::GetInstance() {
  static base::NoDestructor<BrowserList> g_browser_list;
  return g_browser_list.get();
}

void BrowserList::AddBrowser(Browser* browser) {
  // Remove |browser| from the appropriate list instance.
  BrowserList* browser_list = GetInstance();

  browser_list->browsers_.push_back(browser);

  browser->RegisterKeepAlive();

  for (BrowserListObserver& observer : browser_list->observers_) {
    observer.OnBrowserAdded(browser);
  }
}

void BrowserList::RemoveBrowser(Browser* browser) {
  // Remove |browser| from the appropriate list instance.
  BrowserList* browser_list = GetInstance();
  RemoveBrowserFrom(browser, &browser_list->browsers_);

  for (BrowserListObserver& observer : browser_list->observers_) {
    observer.OnBrowserRemoved(browser);
  }

  browser->UnregisterKeepAlive();
}

// static
void BrowserList::AddObserver(BrowserListObserver* observer) {
  GetInstance()->observers_.AddObserver(observer);
}

// static
void BrowserList::RemoveObserver(BrowserListObserver* observer) {
  GetInstance()->observers_.RemoveObserver(observer);
}

BrowserList::BrowserList()
    : observers_(base::ObserverListPolicy::EXISTING_ONLY) {}
BrowserList::~BrowserList() = default;

// static
void BrowserList::RemoveBrowserFrom(Browser* browser,
                                    BrowserVector* browser_list) {
  auto remove_browser = std::ranges::find(*browser_list, browser);
  if (remove_browser != browser_list->end()) {
    browser_list->erase(remove_browser);
  }
}
