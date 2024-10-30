// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_RADIUM_SWITCHES_H_
#define RADIUM_COMMON_RADIUM_SWITCHES_H_

#include "build/build_config.h"

namespace switches {

// All switches in alphabetical order. The switches should be documented
// alongside the definition of their values in the .cc file.

#if BUILDFLAG(IS_WIN)
inline constexpr char kNoPreReadMainDll[] = "no-pre-read-main-dll";
#endif  // BUILDFLAG(IS_WIN)

// TLS 1.2 mode for |kSSLVersionMax| and |kSSLVersionMin| switches.
inline constexpr char kSSLVersionTLSv12[] = "tls1.2";

// TLS 1.3 mode for |kSSLVersionMax| and |kSSLVersionMin| switches.
inline constexpr char kSSLVersionTLSv13[] = "tls1.3";

// Uses WinHttp to resolve proxies instead of using Chromium's normal proxy
// resolution logic. This is only supported in Windows.
//
// TODO(crbug.com/40111093): Only use WinHttp whenever Chrome is
// exclusively using system proxy configs.
inline constexpr char kUseSystemProxyResolver[] = "use-system-proxy-resolver";

// Uses WinHTTP to fetch and evaluate PAC scripts. Otherwise the default is to
// use Chromium's network stack to fetch, and V8 to evaluate.
inline constexpr char kWinHttpProxyResolver[] = "winhttp-proxy-resolver";

}  // namespace switches

#endif  // RADIUM_COMMON_RADIUM_SWITCHES_H_
