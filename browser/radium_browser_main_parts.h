// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_H_

#include "base/files/file_path.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/result_codes.h"
#include "radium/browser/buildflags.h"

class BrowserProcess;
class RadiumBrowserMainExtraParts;
class RadiumFeatureListCreator;
class StartupBrowserCreator;
class Profile;

namespace base {
class CommandLine;
class RunLoop;
}  // namespace base

class RadiumBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit RadiumBrowserMainParts(
      RadiumFeatureListCreator* radium_feature_list_creator);
  RadiumBrowserMainParts(const RadiumBrowserMainParts&) = delete;
  RadiumBrowserMainParts& operator=(const RadiumBrowserMainParts&) = delete;

  ~RadiumBrowserMainParts() override;

  // Add additional ChromeBrowserMainExtraParts.
  void AddParts(std::unique_ptr<RadiumBrowserMainExtraParts> parts);

#if BUILDFLAG(ENABLE_PROCESS_SINGLETON)
  // Handles notifications from other processes. The function receives the
  // command line and directory with which the other Chrome process was
  // launched. Return true if the command line will be handled within the
  // current browser instance or false if the remote process should handle it
  // (i.e., because the current process is shutting down).
  static bool ProcessSingletonNotificationCallback(
      base::CommandLine command_line,
      const base::FilePath& current_directory);
#endif  // BUILDFLAG(ENABLE_PROCESS_SINGLETON)

 protected:
  // content::BrowserMainParts:
  int PreEarlyInitialization() override;
  void ToolkitInitialized() override;
  void PreCreateMainMessageLoop() override;
  void PostCreateMainMessageLoop() override;
  int PreCreateThreads() override;
  void PostCreateThreads() override;
  int PreMainMessageLoopRun() override;
#if !BUILDFLAG(IS_ANDROID)
  bool ShouldInterceptMainMessageLoopRun() override;
#endif
  void WillRunMainMessageLoop(
      std::unique_ptr<base::RunLoop>& run_loop) override;
  void PostMainMessageLoopRun() override;

  // Additional stages for ChromeBrowserMainExtraParts. These stages are called
  // in order from PreMainMessageLoopRun(). See implementation for details.
  virtual void PreProfileInit();
  // `PostProfileInit()` is called for each regular profile that is created. The
  // first call has `is_initial_profile`=true, and subsequent calls have
  // `is_initial_profile`=false.
  // It may be called during startup if a profile is loaded immediately, or
  // later if the profile picker is shown.
  virtual void PostProfileInit(Profile* profile, bool is_initial_profile);
  virtual void PreBrowserStart();
  virtual void PostBrowserStart();

  // Methods for Main Message Loop -------------------------------------------

  int PreCreateThreadsImpl();
  int PreMainMessageLoopRunImpl();

  const raw_ptr<RadiumFeatureListCreator> radium_feature_list_creator_;

  int result_code_ = content::RESULT_CODE_NORMAL_EXIT;

  // Vector of additional ChromeBrowserMainExtraParts.
  // Parts are deleted in the inverse order they are added.
  std::vector<std::unique_ptr<RadiumBrowserMainExtraParts>> radium_extra_parts_;

  // Members initialized after / released before main_message_loop_ ------------
  std::unique_ptr<BrowserProcess> browser_process_;

#if !BUILDFLAG(IS_ANDROID)
  // Browser creation happens on the Java side in Android.
  std::unique_ptr<StartupBrowserCreator> browser_creator_;
#endif  // !BUILDFLAG(IS_ANDROID)

  base::FilePath user_data_dir_;
};

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_MAIN_PARTS_H_
