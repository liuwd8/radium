// Copyright 2024 The Chromium Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_LOGGING_RADIUM_H_
#define RADIUM_COMMON_LOGGING_RADIUM_H_

#include "base/logging.h"
#include "build/build_config.h"

namespace base {
class CommandLine;
class FilePath;
}  // namespace base

namespace logging {

// Call to initialize logging for Chrome. This sets up the chrome-specific
// logfile naming scheme and might do other things like log modules and
// setting levels in the future.
//
// The main process might want to delete any old log files on startup by
// setting `delete_old_log_file`, but child processes should not, or they
// will delete each others' logs.
void InitRadiumLogging(const base::CommandLine& command_line,
                       OldFileDeletionState delete_old_log_file);

// Call when done using logging for Chrome.
void CleanupRadiumLogging();

}  // namespace logging

#endif  // RADIUM_COMMON_LOGGING_RADIUM_H_
