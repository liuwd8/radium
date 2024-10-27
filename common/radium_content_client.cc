// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/common/radium_content_client.h"

#include "components/crash/core/common/crash_key.h"
#include "config/gpu_util.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

RadiumContentClient::RadiumContentClient() = default;
RadiumContentClient::~RadiumContentClient() = default;

void RadiumContentClient::SetActiveURL(const GURL& url,
                                       std::string top_origin) {
  static crash_reporter::CrashKeyString<1024> active_url("url-chunk");
  active_url.Set(url.possibly_invalid_spec());

  // Use a large enough size for Origin::GetDebugString.
  static crash_reporter::CrashKeyString<128> top_origin_key("top-origin");
  top_origin_key.Set(top_origin);
}

void RadiumContentClient::SetGpuInfo(const gpu::GPUInfo& gpu_info) {
  gpu::SetKeysForCrashLogging(gpu_info);
}

void RadiumContentClient::AddAdditionalSchemes(Schemes* schemes) {}

std::u16string RadiumContentClient::GetLocalizedString(int message_id) {
  return l10n_util::GetStringUTF16(message_id);
}

std::u16string RadiumContentClient::GetLocalizedString(
    int message_id,
    const std::u16string& replacement) {
  return l10n_util::GetStringFUTF16(message_id, replacement);
}

bool RadiumContentClient::HasDataResource(int resource_id) const {
  return ui::ResourceBundle::GetSharedInstance().HasDataResource(resource_id);
}

std::string_view RadiumContentClient::GetDataResource(
    int resource_id,
    ui::ResourceScaleFactor scale_factor) {
  return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedMemory* RadiumContentClient::GetDataResourceBytes(
    int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
      resource_id);
}

std::string RadiumContentClient::GetDataResourceString(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
      resource_id);
}

gfx::Image& RadiumContentClient::GetNativeImageNamed(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(
      resource_id);
}

std::string RadiumContentClient::GetProcessTypeNameInEnglish(int type) {
  return "Unknown";
}
