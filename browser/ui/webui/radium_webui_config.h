// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_WEBUI_RADIUM_WEBUI_CONFIG_H_
#define RADIUM_BROWSER_UI_WEBUI_RADIUM_WEBUI_CONFIG_H_

#include "content/public/browser/webui_config.h"

class RadiumWebUIConfig : public content::WebUIConfig {
 public:
  explicit RadiumWebUIConfig(std::string_view scheme, std::string_view host);
  RadiumWebUIConfig(const RadiumWebUIConfig&) = delete;
  RadiumWebUIConfig& operator=(const RadiumWebUIConfig&) = delete;

  ~RadiumWebUIConfig() override;

  // Returns the WebUI name used for logging metrics.
  virtual std::string GetWebUIName() = 0;

  // Returns true if the host should automatically resize to fit the page size.
  virtual bool ShouldAutoResizeHost() = 0;

  // Returns true to allow preloading.
  //
  // Preloading runs the WebUI in an isolated WebContents managed by
  // WebUIContentsPreloadManager. This environment lacks knowledge of its
  // embedder (e.g., side panel) and only knows the associated profile. It's
  // similar to running in a tab but without TabHelpers.
  //
  // Implications for preloaded WebUIs:
  //
  // * Data Availability: Preloading may occur during startup when some data
  //   (e.g., bookmarks) is unavailable. Preloadable WebUIs must handle this.
  // * Context Availability: Some WebUIs rely on their embedder injecting a
  //   context (e.g., TabInterface) into the WebUI controller during WebUI
  //   construction. These WebUIs assume page handles are created after this
  //   injection. This assumption is invalid for preloaded WebUIs because at
  //   preload time, there is no embedder present, and by the time the injection
  //   happens at show time, the page handler could already be created.
  // * Testability: `GetCommandIdForTesting()` must return a non-null command
  //   ID for testing purposes. This ensures preloaded WebUIs can be triggered
  //   and tested for crashes.
  // * Visibility: Do not assume user visibility upon page load. Observe
  //   `OnVisibilityChanged()` on the WebContents to track visibility.
  virtual bool IsPreloadable(content::BrowserContext* browser_context) = 0;
};

template <typename T>
class DefaultRadiumWebUIConfig : public RadiumWebUIConfig {
 public:
  DefaultRadiumWebUIConfig(std::string_view scheme, std::string_view host)
      : RadiumWebUIConfig(scheme, host) {}

  bool IsWebUIEnabled(content::BrowserContext* browser_context) override {
    return true;
  }

  // DefaultRadiumWebUIConfig:
  std::string GetWebUIName() override { return T::GetWebUIName(); }
  bool ShouldAutoResizeHost() override { return false; }
  std::unique_ptr<content::WebUIController> CreateWebUIController(
      content::WebUI* web_ui,
      const GURL& url) override {
    // Disallow dual constructibility.
    // The controller can be constructed either by T(WebUI*) or
    // T(WebUI*, const GURL&), but not both.
    static_assert(std::is_constructible_v<T, content::WebUI*> ||
                  std::is_constructible_v<T, content::WebUI*, const GURL&>);
    static_assert(!(std::is_constructible_v<T, content::WebUI*> &&
                    std::is_constructible_v<T, content::WebUI*, const GURL&>));
    if constexpr (std::is_constructible_v<T, content::WebUI*>) {
      return std::make_unique<T>(web_ui);
    }
    if constexpr (std::is_constructible_v<T, content::WebUI*, const GURL&>) {
      return std::make_unique<T>(web_ui, url);
    }
  }

  bool IsPreloadable(content::BrowserContext* browser_context) override {
    return false;
  }
};

#endif  // RADIUM_BROWSER_UI_WEBUI_RADIUM_WEBUI_CONFIG_H_
