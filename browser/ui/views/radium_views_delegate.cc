// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/radium_views_delegate.h"

#include "components/keep_alive_registry/keep_alive_registry.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "components/keep_alive_registry/scoped_keep_alive.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "radium/browser/browser_process.h"
#include "radium/common/pref_names.h"
#include "ui/color/color_id.h"
#include "ui/display/screen.h"

namespace {
base::Value::Dict& GetWindowPlacementDictionaryReadWrite(
    const std::string& window_name,
    PrefService* prefs,
    std::unique_ptr<ScopedDictPrefUpdate>& scoped_update) {
  if (prefs->FindPreference(window_name)) {
    scoped_update = std::make_unique<ScopedDictPrefUpdate>(prefs, window_name);
    return scoped_update->Get();
  }

  // The window placements for all apps are stored in a single dictionary
  // preference, with per-window-name nested dictionaries, so need to make
  // ScopedDictPrefUpdate and then find the relevant dictionary within it,
  // based on window name.
  scoped_update =
      std::make_unique<ScopedDictPrefUpdate>(prefs, prefs::kAppWindowPlacement);
  base::Value::Dict* this_app_dict =
      (*scoped_update)->FindDictByDottedPath(window_name);
  if (this_app_dict) {
    return *this_app_dict;
  }

  return (*scoped_update)
      ->SetByDottedPath(window_name, base::Value::Dict())
      ->GetDict();
}

}  // namespace

RadiumViewsDelegate::RadiumViewsDelegate() = default;
RadiumViewsDelegate::~RadiumViewsDelegate() = default;

void RadiumViewsDelegate::SaveWindowPlacement(
    const views::Widget* window,
    const std::string& window_name,
    const gfx::Rect& bounds,
    ui::mojom::WindowShowState show_state) {
  PrefService* prefs = BrowserProcess::Get()->local_state();
  if (!prefs) {
    return;
  }

  std::unique_ptr<ScopedDictPrefUpdate> pref_update;
  base::Value::Dict& window_preferences =
      GetWindowPlacementDictionaryReadWrite(window_name, prefs, pref_update);

  window_preferences.Set("left", bounds.x());
  window_preferences.Set("top", bounds.y());
  window_preferences.Set("right", bounds.right());
  window_preferences.Set("bottom", bounds.bottom());
  window_preferences.Set("maximized",
                         show_state == ui::mojom::WindowShowState::kMaximized);

  gfx::Rect work_area(display::Screen::GetScreen()
                          ->GetDisplayNearestView(window->GetNativeView())
                          .work_area());
  window_preferences.Set("work_area_left", work_area.x());
  window_preferences.Set("work_area_top", work_area.y());
  window_preferences.Set("work_area_right", work_area.right());
  window_preferences.Set("work_area_bottom", work_area.bottom());
}

bool RadiumViewsDelegate::GetSavedWindowPlacement(
    const views::Widget* widget,
    const std::string& window_name,
    gfx::Rect* bounds,
    ui::mojom::WindowShowState* show_state) const {
  PrefService* prefs = BrowserProcess::Get()->local_state();
  if (!prefs) {
    return false;
  }

  std::unique_ptr<ScopedDictPrefUpdate> pref_update;
  base::Value::Dict& dictionary =
      GetWindowPlacementDictionaryReadWrite(window_name, prefs, pref_update);
  std::optional<int> left = dictionary.FindInt("left");
  std::optional<int> top = dictionary.FindInt("top");
  std::optional<int> right = dictionary.FindInt("right");
  std::optional<int> bottom = dictionary.FindInt("bottom");
  if (!left || !top || !right || !bottom) {
    return false;
  }

  bounds->SetRect(*left, *top, *right - *left, *bottom - *top);

  const bool maximized = dictionary.FindBool("maximized").value_or(false);
  *show_state = maximized ? ui::mojom::WindowShowState::kMaximized
                          : ui::mojom::WindowShowState::kNormal;
  return true;
}

void RadiumViewsDelegate::AddRef() {
  if (ref_count_ == 0u) {
    keep_alive_ = std::make_unique<ScopedKeepAlive>(
        KeepAliveOrigin::CHROME_VIEWS_DELEGATE,
        KeepAliveRestartOption::DISABLED);
  }

  ++ref_count_;
}

void RadiumViewsDelegate::ReleaseRef() {
  DCHECK_NE(0u, ref_count_);

  if (--ref_count_ == 0u) {
    keep_alive_.reset();
  }
}

bool RadiumViewsDelegate::IsShuttingDown() const {
  return KeepAliveRegistry::GetInstance()->IsShuttingDown();
}

void RadiumViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // We need to determine opacity if it's not already specified.
  if (params->opacity == views::Widget::InitParams::WindowOpacity::kInferred) {
    params->opacity = views::Widget::InitParams::WindowOpacity::kOpaque;
  }

  if (params->type != views::Widget::InitParams::TYPE_POPUP &&
      !params->background_color) {
    params->background_color = ui::kColorWindowBackground;
  }

  // If we already have a native_widget, we don't have to try to come
  // up with one.
  if (params->native_widget) {
    return;
  }

  if (!native_widget_factory().is_null()) {
    params->native_widget = native_widget_factory().Run(*params, delegate);
    if (params->native_widget) {
      return;
    }
  }

  params->native_widget = CreateNativeWidget(params, delegate);
}
