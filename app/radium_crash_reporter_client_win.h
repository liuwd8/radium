// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_APP_RADIUM_CRASH_REPORTER_CLIENT_WIN_H_
#define RADIUM_APP_RADIUM_CRASH_REPORTER_CLIENT_WIN_H_

#include "components/crash/core/app/crash_reporter_client.h"

class RadiumCrashReporterClient : public crash_reporter::CrashReporterClient {
 public:
  // Instantiates a process wide instance of the RadiumCrashReporterClient
  // class and initializes crash reporting for the process. The instance is
  // leaked.
  static void InitializeCrashReportingForProcess();

  RadiumCrashReporterClient();

  RadiumCrashReporterClient(const RadiumCrashReporterClient&) = delete;
  RadiumCrashReporterClient& operator=(const RadiumCrashReporterClient&) =
      delete;

  ~RadiumCrashReporterClient() override;

  // crash_reporter::CrashReporterClient implementation.
  bool GetAlternativeCrashDumpLocation(std::wstring* crash_dir) override;
  void GetProductNameAndVersion(const std::wstring& exe_path,
                                std::wstring* product_name,
                                std::wstring* version,
                                std::wstring* special_build,
                                std::wstring* channel_name) override;
  void GetProductInfo(ProductInfo* product_info) override;
  bool GetShouldDumpLargerDumps() override;

  bool GetCrashDumpLocation(std::wstring* crash_dir) override;
  bool GetCrashMetricsLocation(std::wstring* metrics_dir) override;

  bool IsRunningUnattended() override;

  bool GetCollectStatsConsent() override;

  bool GetCollectStatsInSample() override;

  bool ReportingIsEnforcedByPolicy(bool* breakpad_enabled) override;

  bool ShouldMonitorCrashHandlerExpensively() override;

  bool EnableBreakpadForProcess(const std::string& process_type) override;

  std::wstring GetWerRuntimeExceptionModule() override;
};

#endif  // RADIUM_APP_RADIUM_CRASH_REPORTER_CLIENT_WIN_H_
