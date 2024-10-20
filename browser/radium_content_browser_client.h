// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_H_
#define RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_H_

#include "content/public/browser/content_browser_client.h"

class RadiumContentBrowserClient : public content::ContentBrowserClient {
 public:
  explicit RadiumContentBrowserClient();
  RadiumContentBrowserClient(const RadiumContentBrowserClient&) = delete;
  RadiumContentBrowserClient& operator=(const RadiumContentBrowserClient&) =
      delete;

  ~RadiumContentBrowserClient() override;

 private:
  // content::ContentBrowserClient
  std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(
      bool is_integration_test) override;
};

#endif  // RADIUM_BROWSER_RADIUM_CONTENT_BROWSER_CLIENT_H_
