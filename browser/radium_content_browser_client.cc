// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "build/build_config.h"
#include "radium/browser/radium_browser_main_parts.h"
#include "radium/browser/radium_content_browser_client.h"

#if BUILDFLAG(IS_LINUX)
#include "radium/browser/radium_browser_main_extra_parts_linux.h"
#include "radium/browser/ui/views/radium_browser_main_extra_parts_views_linux.h"
#elif BUILDFLAG(IS_OZONE)
#include "radium/browser/radium_browser_main_extra_parts_ozone.h"
#endif

RadiumContentBrowserClient::RadiumContentBrowserClient() = default;
RadiumContentBrowserClient::~RadiumContentBrowserClient() = default;

std::unique_ptr<content::BrowserMainParts>
RadiumContentBrowserClient::CreateBrowserMainParts(bool is_integration_test) {
  std::unique_ptr<RadiumBrowserMainParts> main_parts =
      std::make_unique<RadiumBrowserMainParts>();

  // Construct additional browser parts. Stages are called in the order in
  // which they are added.
#if defined(TOOLKIT_VIEWS)
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  main_parts->AddParts(
      std::make_unique<ChromeBrowserMainExtraPartsViewsLacros>());
// TODO(crbug.com/40118868): Revisit the macro expression once build flag switch
// of lacros-chrome is complete.
#elif BUILDFLAG(IS_LINUX)
  main_parts->AddParts(
      std::make_unique<RadiumBrowserMainExtraPartsViewsLinux>());
#else
  main_parts->AddParts(std::make_unique<ChromeBrowserMainExtraPartsViews>());
#endif
#endif

#if BUILDFLAG(IS_LINUX)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsLinux>());
#elif BUILDFLAG(IS_OZONE)
  main_parts->AddParts(std::make_unique<RadiumBrowserMainExtraPartsOzone>());
#endif

  return main_parts;
}
