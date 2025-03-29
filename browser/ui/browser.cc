// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/browser.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "base/memory/ptr_util.h"
#include "base/supports_user_data.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_user_data.h"
#include "radium/browser/profiles/profile.h"
#include "radium/browser/ui/browser_list.h"
#include "radium/browser/ui/browser_observer.h"

namespace {

class TabImpl : public content::WebContentsUserData<TabImpl> {
 public:
  Browser* browser() const { return browser_; }
  void set_browser(Browser* browser) { browser_ = browser; }

 private:
  friend content::WebContentsUserData<TabImpl>;
  WEB_CONTENTS_USER_DATA_KEY_DECL();

  explicit TabImpl(content::WebContents* web_contents)
      : content::WebContentsUserData<TabImpl>(*web_contents) {}
  ~TabImpl() override {
    if (browser_) {
      browser_->RemoveWebContents(&GetWebContents());
    }
  }

  raw_ptr<Browser> browser_ = nullptr;
};

[[maybe_unused]] WEB_CONTENTS_USER_DATA_KEY_IMPL(TabImpl);

}  // namespace

// static
Browser* Browser::Create(CreateParams params) {
  return new Browser(std::move(params));
}

Browser::Browser(CreateParams params)
    : profile_(params.profile), unload_controller_(this) {
  window_ = params.new_window(base::WrapUnique(this));
  // This is the last line of statement. It is expected that this is fully
  // constructed when the observer is used
  BrowserList::AddBrowser(this);
}

Browser::~Browser() {
  // This is the first statement. It is expected that this is still a complete
  // object when the observer uses it.
  BrowserList::RemoveBrowser(this);
}

void Browser::RegisterKeepAlive() {
  keep_alive_ = std::make_unique<ScopedKeepAlive>(
      KeepAliveOrigin::BROWSER, KeepAliveRestartOption::DISABLED);
}

void Browser::UnregisterKeepAlive() {
  keep_alive_.reset();
}

void Browser::AddObserver(BrowserObserver* observer) {
  observers_.AddObserver(observer);
}

void Browser::RemoveObserver(BrowserObserver* observer) {
  observers_.RemoveObserver(observer);
}

content::WebContents* Browser::CreateWebContents() {
  content::WebContents::CreateParams params(profile_);
  std::unique_ptr<content::WebContents> web_contents =
      content::WebContents::Create(std::move(params));
  content::WebContents* web_contents_ptr = web_contents.get();
  AddWebContents(std::move(web_contents));

  return web_contents_ptr;
}

void Browser::AddWebContents(
    std::unique_ptr<content::WebContents> web_contents) {
  web_contents->SetDelegate(this);
  content::WebContents* web_contents_ptr = web_contents.get();
  tabs_.insert(std::move(web_contents));

  observers_.Notify(&BrowserObserver::OnWebContentsAdded, web_contents_ptr);
}

std::unique_ptr<content::WebContents> Browser::RemoveWebContents(
    content::WebContents* web_contents) {
  auto iter = std::ranges::find_if(tabs_, [web_contents](auto& contents) {
    return web_contents == contents.get();
  });

  if (iter == tabs_.end()) {
    return nullptr;
  }

  web_contents->SetDelegate(nullptr);
  std::unique_ptr<content::WebContents> value = std::move(*iter);
  tabs_.erase(iter);

  observers_.Notify(&BrowserObserver::OnWebContentsRemoved, value.get());

  if (tabs_.empty()) {
    observers_.Notify(&BrowserObserver::OnWebContentsEmpty);
  }
  return value;
}

void Browser::OnWindowClosing() {
  std::vector<content::WebContents*> tabs;
  tabs.reserve(tabs_.size());
  std::ranges::for_each(tabs_,
                        [&tabs](auto& tab) { tabs.push_back(tab.get()); });

  // Construct a map of processes to the number of associated tabs that are
  // closing.
  base::flat_map<content::RenderProcessHost*, size_t> processes;
  for (auto& contents : tabs) {
    if (contents->NeedToFireBeforeUnloadOrUnloadEvents() ||
        unload_controller_.ShouldRunUnloadEventsHelper(contents)) {
      continue;
    }
    content::RenderProcessHost* process =
        contents->GetPrimaryMainFrame()->GetProcess();
    ++processes[process];
  }

  // Try to fast shutdown the tabs that can close.
  for (const auto& pair : processes) {
    pair.first->FastShutdownIfPossible(pair.second, false);
  }

  for (auto& contents : tabs) {
    if (unload_controller_.RunUnloadEventsHelper(contents)) {
      continue;
    }
    contents->Close();
  }
}

void Browser::CloseContents(content::WebContents* source) {
  if (unload_controller_.CanCloseContents(source)) {
    RemoveWebContents(source);
  }
}

void Browser::BeforeUnloadFired(content::WebContents* web_contents,
                                bool proceed,
                                bool* proceed_to_fire_unload) {
  *proceed_to_fire_unload =
      unload_controller_.BeforeUnloadFired(web_contents, proceed);
}
