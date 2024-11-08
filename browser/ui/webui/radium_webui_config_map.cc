// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/webui/radium_webui_config_map.h"

#include "base/no_destructor.h"
#include "base/strings/strcat.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_controller_factory.h"
#include "content/public/browser/webui_config.h"
#include "radium/common/webui_url_constants.h"
#include "url/gurl.h"

namespace {

// Owned by RadiumWebUIConfigMap. Used to hook up with the existing WebUI infra.
class WebUIConfigMapWebUIControllerFactory
    : public content::WebUIControllerFactory {
 public:
  explicit WebUIConfigMapWebUIControllerFactory(
      RadiumWebUIConfigMap& config_map)
      : config_map_(config_map) {}
  ~WebUIConfigMapWebUIControllerFactory() override = default;

  content::WebUI::TypeID GetWebUIType(content::BrowserContext* browser_context,
                                      const GURL& url) override {
    auto* config = config_map_->GetConfig(browser_context, url);
    return config ? reinterpret_cast<content::WebUI::TypeID>(config)
                  : content::WebUI::kNoWebUI;
  }

  bool UseWebUIForURL(content::BrowserContext* browser_context,
                      const GURL& url) override {
    return config_map_->GetConfig(browser_context, url);
  }

  std::unique_ptr<content::WebUIController> CreateWebUIControllerForURL(
      content::WebUI* web_ui,
      const GURL& url) override {
    auto* browser_context = web_ui->GetWebContents()->GetBrowserContext();
    auto* config = config_map_->GetConfig(browser_context, url);
    return config ? config->CreateWebUIController(web_ui, url) : nullptr;
  }

 private:
  // Keeping a reference should be safe since this class is owned by
  // RadiumWebUIConfigMap.
  const raw_ref<RadiumWebUIConfigMap> config_map_;
};

}  // namespace

// static
RadiumWebUIConfigMap& RadiumWebUIConfigMap::GetInstance() {
  static base::NoDestructor<RadiumWebUIConfigMap> instance;
  return *instance.get();
}

RadiumWebUIConfigMap::RadiumWebUIConfigMap()
    : webui_controller_factory_(
          std::make_unique<WebUIConfigMapWebUIControllerFactory>(*this)) {
  content::WebUIControllerFactory::RegisterFactory(
      webui_controller_factory_.get());
}

RadiumWebUIConfigMap::~RadiumWebUIConfigMap() = default;

void RadiumWebUIConfigMap::AddWebUIConfig(
    std::unique_ptr<content::WebUIConfig> config) {
  CHECK_EQ(config->scheme(), radium::kRadiumUIScheme);
  AddWebUIConfigImpl(std::move(config));
}

void RadiumWebUIConfigMap::AddWebUIConfigImpl(
    std::unique_ptr<content::WebUIConfig> config) {
  GURL url(base::StrCat(
      {config->scheme(), url::kStandardSchemeSeparator, config->host()}));
  auto it = configs_map_.emplace(url::Origin::Create(url), std::move(config));
  // CHECK if a content::WebUIConfig with the same host was already added.
  CHECK(it.second) << url;
}

content::WebUIConfig* RadiumWebUIConfigMap::GetConfig(
    content::BrowserContext* browser_context,
    const GURL& url) {
  // "filesystem:" and "blob:" get dropped by url::Origin::Create() below. We
  // don't want navigations to these URLs to have WebUI bindings, e.g.
  // chrome.send() or Mojo.bindInterface(), since some WebUIs currently expose
  // untrusted content via these schemes.
  if (url.scheme() != radium::kRadiumUIScheme) {
    return nullptr;
  }

  auto origin_and_config = configs_map_.find(url::Origin::Create(url));
  if (origin_and_config == configs_map_.end()) {
    return nullptr;
  }

  auto& config = origin_and_config->second;
  if (!config->IsWebUIEnabled(browser_context) ||
      !config->ShouldHandleURL(url)) {
    return nullptr;
  }

  return config.get();
}

std::unique_ptr<content::WebUIConfig> RadiumWebUIConfigMap::RemoveConfig(
    const GURL& url) {
  CHECK(url.scheme() == radium::kRadiumUIScheme);

  auto it = configs_map_.find(url::Origin::Create(url));
  if (it == configs_map_.end()) {
    return nullptr;
  }

  auto webui_config = std::move(it->second);
  configs_map_.erase(it);
  return webui_config;
}

std::vector<content::WebUIConfigInfo> RadiumWebUIConfigMap::GetWebUIConfigList(
    content::BrowserContext* browser_context) {
  std::vector<content::WebUIConfigInfo> origins;
  origins.reserve(configs_map_.size());
  for (auto& it : configs_map_) {
    auto& webui_config = it.second;
    origins.push_back({
        .origin = it.first,
        .enabled =
            browser_context && webui_config->IsWebUIEnabled(browser_context),
    });
  }
  return origins;
}
