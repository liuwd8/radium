// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_RADIUM_CONTENT_CLIENT_H_
#define RADIUM_COMMON_RADIUM_CONTENT_CLIENT_H_

#include "content/public/common/content_client.h"

class RadiumContentClient : public content::ContentClient {
 public:
  explicit RadiumContentClient();
  RadiumContentClient(const RadiumContentClient&) = delete;
  RadiumContentClient& operator=(const RadiumContentClient&) = delete;

  ~RadiumContentClient() override;

 private:
  void AddAdditionalSchemes(Schemes* schemes) override;
};

#endif  // RADIUM_COMMON_RADIUM_CONTENT_CLIENT_H_
