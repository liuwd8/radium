// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_PROCESS_SINGLETON_LOCK_POSIX_H_
#define RADIUM_COMMON_PROCESS_SINGLETON_LOCK_POSIX_H_

#include <string>

#include "base/files/file_path.h"

// Extract the hostname and pid from the lock symlink. Returns true if the lock
// existed. See ProcessSingleton for additional details.
bool ParseProcessSingletonLock(const base::FilePath& path,
                               std::string* hostname,
                               int* pid);

extern const char kProcessSingletonLockDelimiter;

#endif  // RADIUM_COMMON_PROCESS_SINGLETON_LOCK_POSIX_H_
