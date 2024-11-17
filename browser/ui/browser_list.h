// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_LIST_H_
#define RADIUM_BROWSER_UI_BROWSER_LIST_H_

#include "base/containers/flat_set.h"
#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"

class Browser;
class BrowserListObserver;
class Profile;

namespace base {
template <typename T>
class NoDestructor;
}

namespace base {
class FilePath;
}

class BrowserList {
 public:
  using BrowserSet = base::flat_set<raw_ptr<Browser>>;
  using BrowserVector = std::vector<raw_ptr<Browser>>;
  using BrowserWeakVector = std::vector<base::WeakPtr<Browser>>;
  using CloseCallback = base::RepeatingCallback<void(const base::FilePath&)>;
  using const_iterator = BrowserVector::const_iterator;
  using const_reverse_iterator = BrowserVector::const_reverse_iterator;

  static BrowserList* GetInstance();

  // Adds or removes |browser| from the list it is associated with. The browser
  // object should be valid BEFORE these calls (for the benefit of observers),
  // so notify and THEN delete the object.
  static void AddBrowser(Browser* browser);
  static void RemoveBrowser(Browser* browser);

  // Adds and removes |observer| from the observer list for all desktops.
  // Observers are responsible for making sure the notifying browser is relevant
  // to them (e.g., on the specific desktop they care about if any).
  static void AddObserver(BrowserListObserver* observer);
  static void RemoveObserver(BrowserListObserver* observer);

  BrowserList(const BrowserList&) = delete;
  BrowserList& operator=(const BrowserList&) = delete;

 private:
  friend base::NoDestructor<BrowserList>;

  explicit BrowserList();
  ~BrowserList();

  // Helper method to remove a browser instance from a list of browsers
  static void RemoveBrowserFrom(Browser* browser, BrowserVector* browser_list);

  // A vector of the browsers in this list, in the order they were added.
  BrowserVector browsers_;

  // A list of observers which will be notified of every browser addition and
  // removal across all BrowserLists.
  base::ObserverList<BrowserListObserver> observers_;
};

#endif  // RADIUM_BROWSER_UI_BROWSER_LIST_H_
