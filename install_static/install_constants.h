// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines the struct used to describe each of a brand's install modes; see
// install_modes.h for details. For brands that integrate with Google Update,
// each mode also describes a strategy for determining its update channel.

#ifndef RADIUM_INSTALL_STATIC_INSTALL_CONSTANTS_H_
#define RADIUM_INSTALL_STATIC_INSTALL_CONSTANTS_H_

#include <windows.h>

#include <stdint.h>

namespace install_static {

// Identifies different strategies for determining an update channel.
enum class ChannelStrategy {
  // The default update channel may be overridden by an explicit value. The
  // installer gets this value on the command line (--channel=name), whereas the
  // browser gets this value from the Windows registry. This is used by Google
  // Chrome's primary install mode to differentiate the extended stable, beta,
  // and dev channels from the default (stable) channel.
  FLOATING,

  // Update channel is a fixed value. This is used by to pin Google Chrome's
  // secondary install modes to their respective channels (e.g., the SxS mode
  // follows the canary channel).
  FIXED,
};

// A POD-struct defining constants for a brand's install mode. A brand has one
// primary and one or more secondary install modes. Refer to kInstallModes in
// chromium_install_modes.cc and google_chrome_install_modes.cc for examples of
// typical mode definitions.
struct InstallConstants {
  // The size (in bytes) of this structure. This serves to verify that all
  // modules in a process have the same definition of the struct.
  size_t size;

  // The brand-specific index/identifier of this instance (defined in a brand's
  // BRAND_install_modes.h file). Index 0 is reserved for a brand's primary
  // install mode.
  int index;

  // The command-line switch originally passed to the installer to select this
  // install mode.
  const char* install_switch;

  // The install suffix of a secondary mode (e.g., " SxS" for canary Chrome) or
  // an empty string for the primary mode. This suffix is appended to file and
  // registry paths used by the product.
  const wchar_t* install_suffix;

  // The suffix for the logos corresponding to this install mode. The
  // VisualElementsManifest generated by the installer will use this suffix to
  // reference the proper logos so that they appear in the Start Menu.
  const wchar_t* logo_suffix;

  // The app guid with which this mode is registered with Google Update, or an
  // empty string if the brand does not integrate with Google Update.
  const wchar_t* app_guid;

  // The unsuffixed application name of this program. This is the base of the
  // name registered with Default Programs on Windows.
  const wchar_t* base_app_name;

  // Used for the following:
  // * The unsuffixed portion of the AppUserModelId. The AppUserModelId is used
  // to group an app's windows together on the Windows taskbar along with its
  // corresponding shortcuts; see
  // https://msdn.microsoft.com/library/windows/desktop/dd378459.aspx for more
  // information. Use ShellUtil::GetBrowserModelId to get the suffixed value --
  // it is almost never correct to use the unsuffixed (base) portion of this id
  // directly.
  // * The prefix for the Elevation Service Name. See
  // install_static::GetElevationServiceDisplayName() and
  // install_static::GetElevationServiceName().
  // * The prefix of Web App ProgIds. See web_app::GetProgIdForApp. This means
  // |base_app_id| must only contain alphanumeric characters and
  // non-leading '.'s.
  const wchar_t* base_app_id;

  // The prefix for the browser's ProgID. This prefix may be no more than 11
  // characters long; see ShellUtil::GetBrowserProgId and
  // https://msdn.microsoft.com/library/windows/desktop/dd542719.aspx.
  const wchar_t* browser_prog_id_prefix;

  // A human-readable description of the browser, used when registering with
  // Windows.
  const wchar_t* browser_prog_id_description;

  // The prefix for the browser pdf viewer's ProgID.  This prefix may be no more
  // than 11 characters long; see ShellUtil::GetBrowserProgId and
  // https://msdn.microsoft.com/library/windows/desktop/dd542719.aspx.
  const wchar_t* pdf_prog_id_prefix;

  // A human-readable description of the pdf viewer, used when registering with
  // Windows.
  const wchar_t* pdf_prog_id_description;

  // The GUID to be used when registering this install mode for Active Setup.
  // Active Setup is used to perform certain operations in a user's context for
  // system-level installs.
  const wchar_t* active_setup_guid;

  // The legacy CommandExecuteImpl CLSID, or an empty string if this install
  // mode never included a DelegateExecute verb handler.
  const wchar_t* legacy_command_execute_clsid;

  // The CLSID of the COM object registered with the Widnows OS. This is for app
  // activation via user interaction with a toast notification in the Action
  // Center.
  CLSID toast_activator_clsid;

  // The CLSID of the COM server that provides silent elevation functionality.
  CLSID elevator_clsid;

  // The IID and the TypeLib of the IElevator interface that provides silent
  // elevation functionality.
  IID elevator_iid;

  // The CLSID of the COM server that provides ETW tracing functionality.
  CLSID tracing_service_clsid;

  // The IID and the TypeLib of the ISystemTraceSession interface that provides
  // ETW tracing functionality.
  IID tracing_service_iid;

  // The default name for this mode's update channel.
  const wchar_t* default_channel_name;

  // The strategy used to determine the mode's update channel, or UNSUPPORTED if
  // the brand does not integrate with Google Update.
  ChannelStrategy channel_strategy;

  // True if this mode supports system-level installs.
  bool supports_system_level;

  // True if this mode supports in-product mechanisms to make the browser the
  // user's chosen default browser. Features such as the "Make default" button
  // in chrome://settings are hidden when this is false.
  bool supports_set_as_default_browser;

  // The index of this mode's main application icon in the main executable.
  int app_icon_resource_index;

  // The resource id of this mode's main application icon.
  int16_t app_icon_resource_id;

  //  The index of this mode's html doc icon in the main executable.
  int html_doc_icon_resource_index;

  // The index of this mode's pdf doc icon in the main executable.
  int pdf_doc_icon_resource_index;

  // The app container sid prefix for sandbox.
  const wchar_t* sandbox_sid_prefix;
};

}  // namespace install_static

#endif  // RADIUM_INSTALL_STATIC_INSTALL_CONSTANTS_H_