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
  void SetActiveURL(const GURL& url, std::string top_origin) override;
  void SetGpuInfo(const gpu::GPUInfo& gpu_info) override;
  void AddAdditionalSchemes(Schemes* schemes) override;
  std::u16string GetLocalizedString(int message_id) override;
  std::u16string GetLocalizedString(int message_id,
                                    const std::u16string& replacement) override;
  bool HasDataResource(int resource_id) const override;
  std::string_view GetDataResource(
      int resource_id,
      ui::ResourceScaleFactor scale_factor) override;
  base::RefCountedMemory* GetDataResourceBytes(int resource_id) override;
  std::string GetDataResourceString(int resource_id) override;
  gfx::Image& GetNativeImageNamed(int resource_id) override;
  std::string GetProcessTypeNameInEnglish(int type) override;
};

#endif  // RADIUM_COMMON_RADIUM_CONTENT_CLIENT_H_
