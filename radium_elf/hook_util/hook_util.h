// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_RADIUM_ELF_HOOK_UTIL_HOOK_UTIL_H_
#define RADIUM_RADIUM_ELF_HOOK_UTIL_HOOK_UTIL_H_

#include <windows.h>

#include <memory>

namespace elf_hook {

//------------------------------------------------------------------------------
// Import Address Table hooking support
//------------------------------------------------------------------------------
class IATHook {
 public:
  IATHook();

  IATHook(const IATHook&) = delete;
  IATHook& operator=(const IATHook&) = delete;

  ~IATHook();

  // Intercept a function in an import table of a specific
  // module. Saves everything needed to Unhook.
  //
  // NOTE: Hook can only be called once at a time, to enable Unhook().
  //
  // Arguments:
  // module                 Module to be intercepted
  // imported_from_module   Module that exports the 'function_name'
  // function_name          Name of the API to be intercepted
  // new_function           New function pointer
  //
  // Returns: Windows error code (winerror.h). NO_ERROR if successful.
  DWORD Hook(HMODULE module,
             const char* imported_from_module,
             const char* function_name,
             void* new_function);

  // Unhook the IAT entry.
  //
  // Returns: Windows error code (winerror.h). NO_ERROR if successful.
  DWORD Unhook();

 private:
  struct IATHookData;

  std::unique_ptr<IATHookData> data_;
};

}  // namespace elf_hook

#endif  // RADIUM_RADIUM_ELF_HOOK_UTIL_HOOK_UTIL_H_
