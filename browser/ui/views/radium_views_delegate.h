// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_RADIUM_VIEWS_DELEGATE_H_
#define RADIUM_BROWSER_UI_VIEWS_RADIUM_VIEWS_DELEGATE_H_

#include <map>

#include "ui/views/views_delegate.h"

class ScopedKeepAlive;

class RadiumViewsDelegate : public views::ViewsDelegate {
 public:
  explicit RadiumViewsDelegate();
  RadiumViewsDelegate(const RadiumViewsDelegate&) = delete;
  RadiumViewsDelegate& operator=(const RadiumViewsDelegate&) = delete;

  ~RadiumViewsDelegate() override;

  // views::ViewsDelegate:
  void SaveWindowPlacement(const views::Widget* window,
                           const std::string& window_name,
                           const gfx::Rect& bounds,
                           ui::mojom::WindowShowState show_state) override;
  bool GetSavedWindowPlacement(
      const views::Widget* widget,
      const std::string& window_name,
      gfx::Rect* bounds,
      ui::mojom::WindowShowState* show_state) const override;

#if BUILDFLAG(IS_WIN)
  int GetAppbarAutohideEdges(HMONITOR monitor,
                             base::OnceClosure callback) override;
#endif

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
  bool WindowManagerProvidesTitleBar(bool maximized) override;
#endif

  void AddRef() override;
  void ReleaseRef() override;
  bool IsShuttingDown() const override;
  void OnBeforeWidgetInit(
      views::Widget::InitParams* params,
      views::internal::NativeWidgetDelegate* delegate) override;
#if BUILDFLAG(IS_MAC)
  ui::ContextFactory* GetContextFactory() override;
#endif

 private:
#if BUILDFLAG(IS_WIN)
  typedef std::map<HMONITOR, int> AppbarAutohideEdgeMap;

  // Callback on main thread with the edges. |returned_edges| is the value that
  // was returned from the call to GetAutohideEdges() that initiated the lookup.
  void OnGotAppbarAutohideEdges(base::OnceClosure callback,
                                HMONITOR monitor,
                                int returned_edges,
                                int edges);
#endif

  views::NativeWidget* CreateNativeWidget(
      views::Widget::InitParams* params,
      views::internal::NativeWidgetDelegate* delegate);

  // |ChromeViewsDelegate| exposes a |RefCounted|-like interface, but //chrome
  // uses |ScopedKeepAlive|s to manage lifetime. We manage an internal counter
  // to do that translation.
  unsigned int ref_count_ = 0u;

  // Prevents BrowserProcess teardown while |ref_count_| is non-zero.
  std::unique_ptr<ScopedKeepAlive> keep_alive_;

#if BUILDFLAG(IS_WIN)
  AppbarAutohideEdgeMap appbar_autohide_edge_map_;
  // If true we're in the process of notifying a callback from
  // GetAutohideEdges().start a new query.
  bool in_autohide_edges_callback_ = false;

  base::WeakPtrFactory<RadiumViewsDelegate> weak_factory_{this};
#endif
};

#endif  // RADIUM_BROWSER_UI_VIEWS_RADIUM_VIEWS_DELEGATE_H_
