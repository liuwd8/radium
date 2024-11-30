// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "radium/install_static/install_util.h"

#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <ranges>
#include <sstream>

#include "radium/install_static/install_details.h"
#include "radium/install_static/install_modes.h"
#include "radium/install_static/user_data_dir.h"

namespace install_static {

enum class ProcessType {
  UNINITIALIZED,
  OTHER_PROCESS,
  BROWSER_PROCESS,
  CRASHPAD_HANDLER_PROCESS,
};

// Caches the |ProcessType| of the current process.
ProcessType g_process_type = ProcessType::UNINITIALIZED;

const wchar_t kCrashpadHandler[] = L"crashpad-handler";
const wchar_t kFallbackHandler[] = L"fallback-handler";

const wchar_t kProcessType[] = L"type";

namespace {

void Trace(const wchar_t* format_string, ...) {
  static const int kMaxLogBufferSize = 1024;
  static wchar_t buffer[kMaxLogBufferSize] = {};

  va_list args = {};

  va_start(args, format_string);
  vswprintf(buffer, kMaxLogBufferSize, format_string, args);
  OutputDebugStringW(buffer);
  va_end(args);
}

bool GetLanguageAndCodePageFromVersionResource(const char* version_resource,
                                               WORD* language,
                                               WORD* code_page) {
  if (!version_resource) {
    return false;
  }

  struct LanguageAndCodePage {
    WORD language;
    WORD code_page;
  };

  LanguageAndCodePage* translation_info = nullptr;
  uint32_t data_size_in_bytes = 0;
  BOOL query_result = VerQueryValueW(
      version_resource, L"\\VarFileInfo\\Translation",
      reinterpret_cast<void**>(&translation_info), &data_size_in_bytes);
  if (!query_result) {
    return false;
  }

  *language = translation_info->language;
  *code_page = translation_info->code_page;
  return true;
}

bool GetValueFromVersionResource(const char* version_resource,
                                 const std::wstring& name,
                                 std::wstring* value_str) {
  assert(value_str);
  value_str->clear();

  // TODO(ananta)
  // It may be better in the long run to enumerate the languages and code pages
  // in the version resource and return the value from the first match.
  WORD language = 0;
  WORD code_page = 0;
  if (!GetLanguageAndCodePageFromVersionResource(version_resource, &language,
                                                 &code_page)) {
    return false;
  }

  const size_t array_size = 8;
  WORD lang_codepage[array_size] = {};
  size_t i = 0;
  // Use the language and codepage
  lang_codepage[i++] = language;
  lang_codepage[i++] = code_page;
  // Use the default language and codepage from the resource.
  lang_codepage[i++] = ::GetUserDefaultLangID();
  lang_codepage[i++] = code_page;
  // Use the language from the resource and Latin codepage (most common).
  lang_codepage[i++] = language;
  lang_codepage[i++] = 1252;
  // Use the default language and Latin codepage (most common).
  lang_codepage[i++] = ::GetUserDefaultLangID();
  lang_codepage[i++] = 1252;

  static_assert((array_size % 2) == 0,
                "Language code page size should be a multiple of 2");
  assert(array_size == i);

  for (i = 0; i < array_size;) {
    wchar_t sub_block[MAX_PATH];
    language = lang_codepage[i++];
    code_page = lang_codepage[i++];
    _snwprintf_s(sub_block, MAX_PATH, MAX_PATH,
                 L"\\StringFileInfo\\%04hx%04hx\\%ls", language, code_page,
                 name.c_str());
    void* value = nullptr;
    uint32_t size = 0;
    BOOL r = ::VerQueryValueW(version_resource, sub_block, &value, &size);
    if (r && value) {
      value_str->assign(static_cast<wchar_t*>(value));
      return true;
    }
  }
  return false;
}

// Converts a process type specified as a string to the ProcessType enum.
ProcessType GetProcessType(const std::wstring& process_type) {
  if (process_type.empty()) {
    return ProcessType::BROWSER_PROCESS;
  }
  if (process_type == kCrashpadHandler) {
    return ProcessType::CRASHPAD_HANDLER_PROCESS;
  }
  return ProcessType::OTHER_PROCESS;
}

bool DirectoryExists(const std::wstring& path) {
  DWORD file_attributes = ::GetFileAttributes(path.c_str());
  if (file_attributes == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// Returns the user's temporary directory, or an empty string in case of
// failure.
std::wstring GetTempDir() {
  constexpr DWORD kBufferLength = MAX_PATH + 1U;
  wchar_t temp_path[kBufferLength];
  DWORD temp_path_len = ::GetTempPath(kBufferLength, temp_path);
  if (temp_path_len == 0 || temp_path_len > kBufferLength) {
    return {};
  }

  std::wstring temp_dir(temp_path, temp_path_len);

  // Strip the trailing slashes if any to duplicate //base method behavior.
  while (!temp_dir.empty() &&
         (temp_dir.back() == '\\' || temp_dir.back() == '/')) {
    temp_dir.pop_back();
  }

  return temp_dir;
}

}  // namespace

std::wstring GetCrashDumpLocation() {
  // In order to be able to start crash handling very early and in chrome_elf,
  // we cannot rely on chrome's PathService entries (for DIR_CRASH_DUMPS) being
  // available on Windows. See https://crbug.com/564398.
  std::wstring user_data_dir;
  bool ret = GetUserDataDirectory(&user_data_dir, nullptr);
  assert(ret);
  (void)ret;
  return user_data_dir.append(L"\\Crashpad");
}

std::wstring GetRegistryPath() {
  std::wstring result(L"Software\\");
  AppendChromeInstallSubDirectory(InstallDetails::Get().mode(),
                                  true /* include_suffix */, &result);
  return result;
}

// Appends "[kCompanyPathName\]kProductPathName[install_suffix]" to |path|,
// returning a reference to |path|.
std::wstring& AppendChromeInstallSubDirectory(const InstallConstants& mode,
                                              bool include_suffix,
                                              std::wstring* path) {
  if (*kCompanyPathName) {
    path->append(kCompanyPathName);
    path->push_back(L'\\');
  }
  path->append(kProductPathName, kProductPathNameLength);
  if (!include_suffix) {
    return *path;
  }
  return path->append(mode.install_suffix);
}

void InitializeProcessType() {
  assert(g_process_type == ProcessType::UNINITIALIZED);
  g_process_type = GetProcessType(
      GetCommandLineSwitchValue(::GetCommandLine(), kProcessType));
}

bool IsProcessTypeInitialized() {
  return g_process_type != ProcessType::UNINITIALIZED;
}

bool IsBrowserProcess() {
  assert(g_process_type != ProcessType::UNINITIALIZED);
  return g_process_type == ProcessType::BROWSER_PROCESS;
}

bool IsCrashpadHandlerProcess() {
  assert(g_process_type != ProcessType::UNINITIALIZED);
  return g_process_type == ProcessType::CRASHPAD_HANDLER_PROCESS;
}

void GetExecutableVersionDetails(const std::wstring& exe_path,
                                 std::wstring* product_name,
                                 std::wstring* version,
                                 std::wstring* special_build,
                                 std::wstring* channel_name) {
  assert(product_name);
  assert(version);
  assert(special_build);
  assert(channel_name);

  // Default values in case we don't find a version resource.
  *product_name = L"Radium";
  *version = L"0.0.0.0-devel";
  special_build->clear();

  DWORD dummy = 0;
  DWORD length = ::GetFileVersionInfoSize(exe_path.c_str(), &dummy);
  if (length) {
    std::unique_ptr<char[]> data(new char[length]);
    if (::GetFileVersionInfo(exe_path.c_str(), dummy, length, data.get())) {
      GetValueFromVersionResource(data.get(), L"ProductVersion", version);

      std::wstring official_build;
      GetValueFromVersionResource(data.get(), L"Official Build",
                                  &official_build);
      if (official_build != L"1") {
        version->append(L"-devel");
      }
      GetValueFromVersionResource(data.get(), L"ProductShortName",
                                  product_name);
      GetValueFromVersionResource(data.get(), L"SpecialBuild", special_build);
    }
  }
  *channel_name = GetChromeChannelName(/*with_extended_stable=*/true);
}

std::string GetEnvironmentString(const std::string& variable_name) {
  return WideToUTF8(GetEnvironmentString(UTF8ToWide(variable_name).c_str()));
}

std::wstring GetEnvironmentString(const wchar_t* variable_name) {
  DWORD value_length = ::GetEnvironmentVariableW(variable_name, nullptr, 0);
  if (!value_length) {
    return std::wstring();
  }
  std::wstring value(value_length, L'\0');
  value_length =
      ::GetEnvironmentVariableW(variable_name, &value[0], value_length);
  if (!value_length || value_length >= value.size()) {
    return std::wstring();
  }
  value.resize(value_length);
  return value;
}

std::string WideToUTF8(const std::wstring& source) {
  if (source.empty() ||
      static_cast<int>(source.size()) > std::numeric_limits<int>::max()) {
    return std::string();
  }
  int size = ::WideCharToMultiByte(CP_UTF8, 0, &source[0],
                                   static_cast<int>(source.size()), nullptr, 0,
                                   nullptr, nullptr);
  std::string result(size, '\0');
  if (::WideCharToMultiByte(CP_UTF8, 0, &source[0],
                            static_cast<int>(source.size()), &result[0], size,
                            nullptr, nullptr) != size) {
    assert(false);
    return std::string();
  }
  return result;
}

std::wstring GetChromeChannelName(bool with_extended_stable) {
  return L"stable";
}

std::wstring UTF8ToWide(const std::string& source) {
  if (source.empty() ||
      static_cast<int>(source.size()) > std::numeric_limits<int>::max()) {
    return std::wstring();
  }
  int size = ::MultiByteToWideChar(CP_UTF8, 0, &source[0],
                                   static_cast<int>(source.size()), nullptr, 0);
  std::wstring result(size, L'\0');
  if (::MultiByteToWideChar(CP_UTF8, 0, &source[0],
                            static_cast<int>(source.size()), &result[0],
                            size) != size) {
    assert(false);
    return std::wstring();
  }
  return result;
}

std::vector<std::wstring> TokenizeCommandLineToArray(
    const std::wstring& command_line) {
  // This is baroquely complex to do properly, see e.g.
  // https://blogs.msdn.microsoft.com/oldnewthing/20100917-00/?p=12833
  // http://www.windowsinspired.com/how-a-windows-programs-splits-its-command-line-into-individual-arguments/
  // and many others. We cannot use CommandLineToArgvW() in chrome_elf, because
  // it's in shell32.dll. Previously, __wgetmainargs() in the CRT was available,
  // and it's still documented for VS 2015 at
  // https://msdn.microsoft.com/en-us/library/ff770599.aspx but unfortunately,
  // isn't actually available.
  //
  // This parsing matches CommandLineToArgvW()s for arguments, rather than the
  // CRTs. These are different only in the most obscure of cases and will not
  // matter in any practical situation. See the windowsinspired.com post above
  // for details.
  //
  // Indicates whether or not space and tab are interpreted as token separators.
  enum class SpecialChars {
    // Space or tab, if encountered, delimit tokens.
    kInterpret,

    // Space or tab, if encountered, are part of the current token.
    kIgnore,
  } state;

  static constexpr wchar_t kSpaceTab[] = L" \t";

  std::vector<std::wstring> result;
  const wchar_t* p = command_line.c_str();

  // The first argument (the program) is delimited by whitespace or quotes based
  // on its first character.
  size_t argv0_length = 0;
  if (p[0] == L'"') {
    const wchar_t* closing = wcschr(++p, L'"');
    if (!closing) {
      argv0_length = command_line.size() - 1;  // Skip the opening quote.
    } else {
      argv0_length = closing - (command_line.c_str() + 1);
    }
  } else {
    argv0_length = wcscspn(p, kSpaceTab);
  }
  result.emplace_back(p, argv0_length);
  if (p[argv0_length] == 0) {
    return result;
  }
  p += argv0_length + 1;

  std::wstring token;
  // This loops the entire string, with a subloop for each argument.
  for (;;) {
    // Advance past leading whitespace (only space and tab are handled).
    p += wcsspn(p, kSpaceTab);

    // End of arguments.
    if (p[0] == 0) {
      break;
    }

    state = SpecialChars::kInterpret;

    // Scan an argument.
    for (;;) {
      // Count and advance past collections of backslashes, which have special
      // meaning when followed by a double quote.
      int num_backslashes = wcsspn(p, L"\\");
      p += num_backslashes;

      if (p[0] == L'"') {
        // Emit a backslash for each pair of backslashes found. A non-paired
        // "extra" backslash is handled below.
        token.append(num_backslashes / 2, L'\\');

        if (num_backslashes % 2 == 1) {
          // An odd number of backslashes followed by a quote is treated as
          // pairs of protected backslashes, followed by the protected quote.
          token += L'"';
        } else if (p[1] == L'"' && state == SpecialChars::kIgnore) {
          // Special case for consecutive double quotes within a quoted string:
          // emit one for the pair, and switch back to interpreting special
          // characters.
          ++p;
          token += L'"';
          state = SpecialChars::kInterpret;
        } else {
          state = state == SpecialChars::kInterpret ? SpecialChars::kIgnore
                                                    : SpecialChars::kInterpret;
        }
      } else {
        // Emit backslashes that do not precede a quote verbatim.
        token.append(num_backslashes, L'\\');
        if (p[0] == 0 ||
            (state == SpecialChars::kInterpret && wcschr(kSpaceTab, p[0]))) {
          result.push_back(token);
          token.clear();
          break;
        }

        token += *p;
      }

      ++p;
    }
  }

  return result;
}

std::optional<std::wstring> GetCommandLineSwitch(
    const std::wstring& command_line,
    std::wstring_view switch_name) {
  assert(!command_line.empty());
  assert(!switch_name.empty());

  std::vector<std::wstring> switches = TokenizeCommandLineToArray(command_line);

  // Stop scanning if lone '--' switch prefix is found.
  auto cend = std::ranges::find(switches, L"--");

  std::wstring switch_with_prefix = L"--" + std::wstring(switch_name);
  for (auto it = switches.cbegin(); it != cend; ++it) {
    if (it->starts_with(switch_with_prefix)) {
      if (it->length() == switch_with_prefix.length()) {
        return std::wstring();
      }
      if ((*it)[switch_with_prefix.length()] == L'=') {
        return it->substr(switch_with_prefix.length() + 1);
      }
    }
  }

  return std::nullopt;
}

std::wstring GetCommandLineSwitchValue(const std::wstring& command_line,
                                       std::wstring_view switch_name) {
  assert(!command_line.empty());
  assert(!switch_name.empty());
  return GetCommandLineSwitch(command_line, switch_name)
      .value_or(std::wstring());
}

bool RecursiveDirectoryCreate(const std::wstring& full_path) {
  // If the path exists, we've succeeded if it's a directory, failed otherwise.
  const wchar_t* full_path_str = full_path.c_str();
  DWORD file_attributes = ::GetFileAttributes(full_path_str);
  if (file_attributes != INVALID_FILE_ATTRIBUTES) {
    if ((file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      Trace(L"%hs( %ls directory exists )\n", __func__, full_path_str);
      return true;
    }
    Trace(L"%hs( %ls directory conflicts with an existing file. )\n", __func__,
          full_path_str);
    return false;
  }

  // Invariant:  Path does not exist as file or directory.

  // Attempt to create the parent recursively.  This will immediately return
  // true if it already exists, otherwise will create all required parent
  // directories starting with the highest-level missing parent.
  std::wstring parent_path;
  std::size_t pos = full_path.find_last_of(L"/\\");
  if (pos != std::wstring::npos) {
    parent_path = full_path.substr(0, pos);
    if (!RecursiveDirectoryCreate(parent_path)) {
      Trace(L"Failed to create one of the parent directories");
      return false;
    }
  }
  if (!::CreateDirectory(full_path_str, nullptr)) {
    DWORD error_code = ::GetLastError();
    if (error_code == ERROR_ALREADY_EXISTS && DirectoryExists(full_path_str)) {
      // This error code ERROR_ALREADY_EXISTS doesn't indicate whether we
      // were racing with someone creating the same directory, or a file
      // with the same path.  If the directory exists, we lost the
      // race to create the same directory.
      return true;
    } else {
      Trace(L"Failed to create directory %ls, last error is %d\n",
            full_path_str, error_code);
      return false;
    }
  }
  return true;
}

std::wstring CreateUniqueTempDirectory(std::wstring_view prefix) {
  std::wstring temp_dir = GetTempDir();
  if (temp_dir.empty()) {
    Trace(L"Failed to retrieve temporary directory");
    return {};
  }

  // The following code uses std::to_wstring() which is banned in Chrome,
  // however, since the //base alternative is not available here, we use it as
  // the last resort. Please DO NOT copy/paste this code outside install_static!
  temp_dir.push_back(L'\\');
  temp_dir.append(prefix);
  temp_dir.append(kProductPathName, kProductPathNameLength);
  temp_dir.append(std::to_wstring(::GetCurrentProcessId()));
  temp_dir.append(std::to_wstring(::GetTickCount()));
  size_t temp_dir_length = temp_dir.length();

  // Try to create a new temporary directory. If the one exists, keep trying
  // adding a randomized suffix to the path until we reach some limit.
  for (int count = 0; count < 50; ++count) {
    if (::CreateDirectory(temp_dir.c_str(), /*lpSecurityAttributes*/ nullptr)) {
      return temp_dir;
    }

    // Seed the rand() once.
    [[maybe_unused]] static bool once = []() {
      srand(::GetTickCount());
      return true;
    }();

    temp_dir.erase(temp_dir_length);
    temp_dir.append(std::to_wstring(rand()));
  }

  Trace(L"Failed to create unique temporary directory %ls", temp_dir.c_str());

  return {};
}

// This function takes these inputs rather than accessing the module's
// InstallDetails instance since it is used to bootstrap InstallDetails.
DetermineChannelResult DetermineChannel(const InstallConstants& mode,
                                        bool system_level,
                                        const wchar_t* channel_override,
                                        std::wstring* update_ap,
                                        std::wstring* update_cohort_name) {
  return {std::wstring(), ChannelOrigin::kInstallMode,
          /*is_extended_stable=*/false};
}

bool VersionMismatch() {
  return install_static::InstallDetails::Get().VersionMismatch();
}

}  // namespace install_static
