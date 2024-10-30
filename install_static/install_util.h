// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains helper functions which provide information about the
// current version of Chrome. This includes channel information, version
// information etc. This functionality is provided by using functions in
// kernel32 and advapi32. No other dependencies are allowed in this file.

#ifndef RADIUM_INSTALL_STATIC_INSTALL_UTIL_H_
#define RADIUM_INSTALL_STATIC_INSTALL_UTIL_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "base/win/windows_types.h"

namespace install_static {
struct InstallConstants;

extern const wchar_t kCrashpadHandler[];
extern const wchar_t kFallbackHandler[];
extern const wchar_t kProcessType[];

// Used for suppressing warnings.
template <typename T>
inline void IgnoreUnused(T) {}

// Returns the path
// "Software\[kCompanyPathName\]kProductPathName[install_suffix]". This subkey
// of HKEY_CURRENT_USER can be used to save and restore state. With the
// exception of data that is used by third parties (e.g., a subkey that
// specifies the location of a native messaging host's manifest), state stored
// in this key is removed during uninstall when the user chooses to also delete
// their browsing data.
std::wstring GetRegistryPath();

// Appends "[kCompanyPathName\]kProductPathName[install_suffix]" to |path|,
// returning a reference to |path|.
std::wstring& AppendChromeInstallSubDirectory(const InstallConstants& mode,
                                              bool include_suffix,
                                              std::wstring* path);

// Initializes |g_process_type| which stores whether or not the current
// process is the main browser process.
void InitializeProcessType();

// Returns true if the process type is initialized. False otherwise.
bool IsProcessTypeInitialized();

// Returns true if invoked in the main browser process; false, otherwise.
bool IsBrowserProcess();

// Populates |crash_dir| with the crash dump location, respecting modifications
// to user-data-dir.
// TODO(ananta)
// http://crbug.com/604923
// Unify this with the Browser Distribution code.
std::wstring GetCrashDumpLocation();

// Returns the contents of the specified |variable_name| from the environment
// block of the calling process. Returns an empty string if the variable does
// not exist.
std::string GetEnvironmentString(const std::string& variable_name);
std::wstring GetEnvironmentString(const wchar_t* variable_name);

// Gets the exe version details like the |product_name|, |version|,
// |special_build|, and |channel_name| from the browser executable at
// |exe_path|. |channel_name| will be "extended" for clients that follow the
// extended stable update channel.
void GetExecutableVersionDetails(const std::wstring& exe_path,
                                 std::wstring* product_name,
                                 std::wstring* version,
                                 std::wstring* special_build,
                                 std::wstring* channel_name);

// Gets the channel for the current Chrome process. Unless
// `with_extended_stable` is true, extended stable will be reported as regular
// stable (i.e., an empty string).
std::wstring GetChromeChannelName(bool with_extended_stable);

// UTF8 to Wide and vice versa conversion helpers.
std::wstring UTF8ToWide(const std::string& source);

std::string WideToUTF8(const std::wstring& source);

// Tokenizes |command_line| in the same way as CommandLineToArgvW() in
// shell32.dll, handling quoting, spacing etc. Normally only used from
// GetCommandLineSwitch(), but exposed for testing.
std::vector<std::wstring> TokenizeCommandLineToArray(
    const std::wstring& command_line);

// Returns the value of a switch of the form "--<switch name>=<switch value>" in
// |command_line|. If the switch has no value, returns an empty string. If the
// switch is not present returns std::nullopt. An empty switch in |command_line|
// ("--") denotes the end of switches and the beginning of args. Anything
// following the "--" switch is ignored.
std::optional<std::wstring> GetCommandLineSwitch(
    const std::wstring& command_line,
    std::wstring_view switch_name);

// Returns the value of the specified switch or an empty string if there is no
// such switch in |command_line| or the switch has no value.
std::wstring GetCommandLineSwitchValue(const std::wstring& command_line,
                                       std::wstring_view switch_name);

}  // namespace install_static

#endif  // RADIUM_INSTALL_STATIC_INSTALL_UTIL_H_
