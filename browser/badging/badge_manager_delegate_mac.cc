// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/badging/badge_manager_delegate_mac.h"

#include "radium/browser/badging/badge_manager.h"

namespace badging {

BadgeManagerDelegateMac::BadgeManagerDelegateMac(Profile* profile,
                                                 BadgeManager* badge_manager)
    : BadgeManagerDelegate(profile, badge_manager) {}

void BadgeManagerDelegateMac::OnAppBadgeUpdated(const webapps::AppId& app_id) {
  // const std::optional<BadgeManager::BadgeValue>& badge =
  //     badge_manager()->GetBadgeValue(app_id);

  // auto* shim_manager = apps::AppShimManager::Get();
  // if (!shim_manager) {
  //   return;
  // }

  // shim_manager->UpdateAppBadge(profile(), app_id, badge);
}

}  // namespace badging