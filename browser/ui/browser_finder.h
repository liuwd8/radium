// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_BROWSER_FINDER_H_
#define RADIUM_BROWSER_UI_BROWSER_FINDER_H_

#include <stddef.h>

#include <vector>

#include "ui/display/types/display_constants.h"
#include "ui/gfx/native_widget_types.h"

class Browser;
class Profile;
class SessionID;

namespace content {
class WebContents;
}

namespace tab_groups {
class TabGroupId;
}

namespace ui {
class ElementContext;
}

// Collection of functions to find Browsers based on various criteria.
//
// WARNING: Most functions in this file should not be used.
//
// Functions in this file typically create non-local control flow. This is hard
// to stub for tests, hard to debug, and hard to maintain due to imprecise API
// surfaces. See
// https://source.chromium.org/chromium/chromium/src/+/main:docs/chrome_browser_design_principles.md
// for details. See TabInterface and TabFeatures for a common structure that
// avoids these functions.
//
// There are a few functions that have valid use cases. For example, we can
// imagine a hypothetical feature of the task manager (which is not conceptually
// associated with any given browser window) which wants to display the number
// of browser windows to the user. This would be a valid use case for
// GetTotalBrowserCount().
//
// More colloquial explanation:
// There are two problems with FindBrowser(*) methods.
//  (1) It returns a Browser*. Browser is a god object which is an antipattern.
//  We should try not to use it.
//  (2) Inverts the control flow. We typically want to pass state into the
//  objects that need it. e.g.
//  std::make_unique<ClassFoo>(dependency_that_foo_requires)
// So in general we shouldn't be looking up a Browser* from a WebContents*, we
// should be passing in the state we need, to the class that owns the
// WebContents*.
//
// BrowserWindowInterface is intended to be a replacement for Browser*. The
// reason this exists as opposed to just passing in the relevant state in the
// constructor of the class is because we have 2 important primitives in chrome
// that have different lifetimes (tab & window) and the relationship is dynamic
// (tabs can move between browser windows).
//
// Example 1: let's say that we have a feature that is conceptually tied to the
// window (e.g. tab search). UserEducation is also conceptually tied to the
// window. If we want to use UserEducation from tab search, we should pass in
// some type of pointer in the constructor of TabSearch, e.g.
// std::make_unique<TabSearch>(user_education).
//
// Example 2: let's say that we have a feature that is conceptually tied to the
// tab (e.g. find in page). In this case, we shouldn't pass a UserEducation to
// the tab feature, since it's possible for the tab to be dragged into a new
// window, and then we'd be pointing to the wrong UserEducation. So instead we
// should pass in a TabInterface (which will always be valid). Then anytime
// UserEducation is needed, dynamically fetch the feature via
// TabInterface->GetBrowserWindowInterface()->GetUserEducation()

namespace radium {

// Returns the number of browsers across all profiles.
//
// WARNING: This function includes browsers scheduled for deletion whereas
// the majority of other functions do not.
size_t GetTotalBrowserCount();

}  // namespace radium

#endif  // RADIUM_BROWSER_UI_BROWSER_FINDER_H_
