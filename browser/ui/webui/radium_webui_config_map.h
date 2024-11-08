// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_WEBUI_RADIUM_WEBUI_CONFIG_MAP_H_
#define RADIUM_BROWSER_UI_WEBUI_RADIUM_WEBUI_CONFIG_MAP_H_

#include <map>
#include <memory>
#include <vector>

#include "content/public/browser/webui_config_map.h"

// Class that holds all WebUIConfigs for the browser.
//
// Embedders wishing to register WebUIConfigs should use
// AddWebUIConfig and AddUntrustedWebUIConfig.
//
// Underneath it uses a WebUIControllerFactory to hook into the rest of the
// WebUI infra.
class RadiumWebUIConfigMap {
 public:
  static RadiumWebUIConfigMap& GetInstance();

  explicit RadiumWebUIConfigMap();
  RadiumWebUIConfigMap(const RadiumWebUIConfigMap&) = delete;
  RadiumWebUIConfigMap& operator=(const RadiumWebUIConfigMap&) = delete;

  ~RadiumWebUIConfigMap();

  // Adds a radium:// WebUIConfig. CHECKs if the WebUIConfig is for a
  // chrome-untrusted:// WebUIConfig.
  void AddWebUIConfig(std::unique_ptr<content::WebUIConfig> config);

  // Returns the WebUIConfig for |url| if it's registered and the WebUI is
  // enabled. (WebUIs can be disabled based on the profile or feature flags.)
  content::WebUIConfig* GetConfig(content::BrowserContext* browser_context,
                                  const GURL& url);

  // Removes and returns the WebUIConfig with |url|. Returns nullptr if
  // there is no WebUIConfig with |url|.
  std::unique_ptr<content::WebUIConfig> RemoveConfig(const GURL& url);

  // Gets a list of the origin (host + scheme) and enabled/disabled status of
  // all currently registered WebUIConfigs. If |browser_context| is null,
  // returns false for the enabled status for all UIs.
  std::vector<content::WebUIConfigInfo> GetWebUIConfigList(
      content::BrowserContext* browser_context);

 private:
  void AddWebUIConfigImpl(std::unique_ptr<content::WebUIConfig> config);

  using WebUIConfigMapImpl =
      std::map<url::Origin, std::unique_ptr<content::WebUIConfig>>;
  WebUIConfigMapImpl configs_map_;

  std::unique_ptr<content::WebUIControllerFactory> webui_controller_factory_;
};

#endif  // RADIUM_BROWSER_UI_WEBUI_RADIUM_WEBUI_CONFIG_MAP_H_
