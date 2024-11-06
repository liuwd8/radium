// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_APP_RADIUM_MAIN_DELEGATE_H_
#define RADIUM_APP_RADIUM_MAIN_DELEGATE_H_

#include "components/memory_system/memory_system.h"
#include "content/public/app/content_main_delegate.h"
#include "radium/app/startup_timestamps.h"
#include "radium/common/radium_content_client.h"

namespace tracing {
class TracingSamplerProfiler;
}

class RadiumContentBrowserClient;

class RadiumMainDelegate : public content::ContentMainDelegate {
 public:
  static const char* const kNonWildcardDomainNonPortSchemes[];
  static const size_t kNonWildcardDomainNonPortSchemesSize;

#if BUILDFLAG(IS_ANDROID)
  RadiumMainDelegate();
#endif

  explicit RadiumMainDelegate(const StartupTimestamps& timestamps);
  RadiumMainDelegate(const RadiumMainDelegate&) = delete;
  RadiumMainDelegate& operator=(const RadiumMainDelegate&) = delete;

  ~RadiumMainDelegate() override;

 protected:
  std::optional<int> BasicStartupComplete() override;
  void PreSandboxStartup() override;
  std::optional<int> PreBrowserMain() override;
  void SandboxInitialized(const std::string& process_type) override;
  void ProcessExiting(const std::string& process_type) override;
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  void ZygoteForked() override;
#endif
  bool ShouldCreateFeatureList(InvokedIn invoked_in) override;
  bool ShouldInitializeMojo(InvokedIn invoked_in) override;
  std::optional<int> PostEarlyInitialization(InvokedIn invoked_in) override;

  content::ContentClient* CreateContentClient() override;
  content::ContentBrowserClient* CreateContentBrowserClient() override;

  // Initialization that happens in all process types.
  void CommonEarlyInitialization(InvokedIn invoked_in);

  // Initializes |tracing_sampler_profiler_|. Deletes any existing
  // |tracing_sampler_profiler_| as well.
  void SetupTracing();

  void InitializeMemorySystem();

  std::unique_ptr<RadiumContentBrowserClient> radium_content_browser_client_;
  std::unique_ptr<tracing::TracingSamplerProfiler> tracing_sampler_profiler_;

  RadiumContentClient radium_content_client_;
  memory_system::MemorySystem memory_system_;
};

#endif  // RADIUM_APP_RADIUM_MAIN_DELEGATE_H_
