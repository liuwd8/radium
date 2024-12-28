// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/webui/example/example_ui.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"
#include "radium/browser/ui/webui/webui_util.h"
#include "radium/common/webui_url_constants.h"
#include "radium/grit/example_resources.h"
#include "radium/grit/example_resources_map.h"

ExampleUI::ExampleUI(content::WebUI* web_ui) : ui::MojoWebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      radium::kRadiumUIExampleHost);

  radium::webui::SetupWebUIDataSource(source, kExampleResources,
                                      IDR_EXAMPLE_EXAMPLE_HTML);
}

ExampleUI::~ExampleUI() = default;
