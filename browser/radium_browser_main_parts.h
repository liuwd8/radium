// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_H_

#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/result_codes.h"

class RadiumBrowserMainExtraParts;

class RadiumBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit RadiumBrowserMainParts();
  RadiumBrowserMainParts(const RadiumBrowserMainParts&) = delete;
  RadiumBrowserMainParts& operator=(const RadiumBrowserMainParts&) = delete;

  ~RadiumBrowserMainParts() override;

  // Add additional ChromeBrowserMainExtraParts.
  void AddParts(std::unique_ptr<RadiumBrowserMainExtraParts> parts);

 private:
  // content::BrowserMainParts:
  int PreMainMessageLoopRun() override;
  void ToolkitInitialized() override;
  void PreCreateMainMessageLoop() override;
  void PostCreateMainMessageLoop() override;
  int PreCreateThreads() override;

  // Methods for Main Message Loop -------------------------------------------

  int PreCreateThreadsImpl();
  int PreMainMessageLoopRunImpl();

  int result_code_ = content::RESULT_CODE_NORMAL_EXIT;

  // Vector of additional ChromeBrowserMainExtraParts.
  // Parts are deleted in the inverse order they are added.
  std::vector<std::unique_ptr<RadiumBrowserMainExtraParts>> radium_extra_parts_;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_H_
