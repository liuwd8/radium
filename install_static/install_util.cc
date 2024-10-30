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
  *product_name = L"Chrome";
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

}  // namespace install_static
