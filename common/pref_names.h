// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_COMMON_PREF_NAMES_H_
#define RADIUM_COMMON_PREF_NAMES_H_

#include <iterator>

#include "build/build_config.h"

namespace prefs {

// A boolean indicating whether Access-Control-Allow-Methods matching in CORS
// preflights is fixed according to the spec. https://crbug.com/1228178
inline constexpr char
    kAccessControlAllowMethodsInCORSPreflightSpecConformant[] =
        "access_control_allow_methods_in_cors_preflight_spec_conformant";

// Boolean that specifies whether additional DNS query types (e.g. HTTPS) may be
// queried alongside the traditional A and AAAA queries.
inline constexpr char kAdditionalDnsQueryTypesEnabled[] =
    "async_dns.additional_dns_query_types_enabled";

// List of origin schemes that allow the supported HTTP authentication schemes
// from "auth.schemes".
inline constexpr char kAllHttpAuthSchemesAllowedForOrigins[] =
    "auth.http_auth_allowed_for_origins";

// Boolean that specifies whether to allow basic auth prompting on cross-
// domain sub-content requests.
inline constexpr char kAllowCrossOriginAuthPrompt[] =
    "auth.allow_cross_origin_prompt";

// Integer specifying the cases where ambient authentication is enabled.
// 0 - Only allow ambient authentication in regular sessions
// 1 - Only allow ambient authentication in regular and incognito sessions
// 2 - Only allow ambient authentication in regular and guest sessions
// 3 - Allow ambient authentication in regular, incognito and guest sessions
inline constexpr char kAmbientAuthenticationInPrivateModesEnabled[] =
    "auth.ambient_auth_in_private_modes";

// A collection of position, size, and other data relating to app windows to
// restore on startup.
inline constexpr char kAppWindowPlacement[] = "browser.app_window_placement";

// Allowlist containing servers Chrome is allowed to do Kerberos delegation
// with. Note that this used to be `kAuthNegotiateDelegateWhitelist`, hence the
// difference between the variable name and the string value.
inline constexpr char kAuthNegotiateDelegateAllowlist[] =
    "auth.negotiate_delegate_whitelist";

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_CHROMEOS)
// Boolean that specifies whether OK-AS-DELEGATE flag from KDC is respected
// along with kAuthNegotiateDelegateAllowlist.
inline constexpr char kAuthNegotiateDelegateByKdcPolicy[] =
    "auth.negotiate_delegate_by_kdc_policy";
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_CHROMEOS)

// String that lists supported HTTP authentication schemes.
inline constexpr char kAuthSchemes[] = "auth.schemes";

// Allowlist containing servers for which Integrated Authentication is enabled.
// This pref should match |android_webview::prefs::kAuthServerAllowlist|.
inline constexpr char kAuthServerAllowlist[] = "auth.server_allowlist";

// Boolean that specifies whether HTTP Basic authentication is allowed for HTTP
// requests.
inline constexpr char kBasicAuthOverHttpEnabled[] =
    "auth.basic_over_http_enabled";

// Boolean that specifies whether the built-in asynchronous DNS client is used.
inline constexpr char kBuiltInDnsClientEnabled[] = "async_dns.enabled";

// A list of base64 encoded certificates that are to be trusted as root certs.
// Only specifiable as an enterprise policy.
inline constexpr char kCACertificates[] = "certificates.ca_certificates";

// A list of objects. Each object contains a base64 encoded certificates that
// are to be trusted as root certs, but with constraints specified outside of
// the certificate in the object.
// Only specifiable as an enterprise policy.
inline constexpr char kCACertificatesWithConstraints[] =
    "certificates.ca_certificates_with_constraints";

// A list of base64 encoded certificates containing SPKIs that are not to be
// trusted.
// Only specifiable as an enterprise policy.
inline constexpr char kCADistrustedCertificates[] =
    "certificates.ca_distrusted_certificates";

// A list of base64 certificates that are to be used as hints for path
// building. Only specifiable as an enterprise policy.
inline constexpr char kCAHintCertificates[] =
    "certificates.ca_hint_certificates";

// Boolean that specifies whether to use user-added certificates that are in the
// platform trust stores.
inline constexpr char kCAPlatformIntegrationEnabled[] =
    "certificates.ca_platform_integration_enabled";

// Boolean that specifies whether to enable revocation checking (best effort)
// by default.
inline constexpr char kCertRevocationCheckingEnabled[] =
    "ssl.rev_checking.enabled";

// Boolean that specifies whether to require a successful revocation check if
// a certificate path ends in a locally-trusted (as opposed to publicly
// trusted) trust anchor.
inline constexpr char kCertRevocationCheckingRequiredLocalAnchors[] =
    "ssl.rev_checking.required_for_local_anchors";

// String specifying the TLS ciphersuites to disable. Ciphersuites are
// specified as a comma-separated list of 16-bit hexadecimal values, with
// the values being the ciphersuites assigned by the IANA registry (e.g.
// "0x0004,0x0005").
inline constexpr char kCipherSuiteBlacklist[] = "ssl.cipher_suites.blacklist";

// A boolean indicating support of "CORS non-wildcard request header name".
// https://fetch.spec.whatwg.org/#cors-non-wildcard-request-header-name
inline constexpr char kCorsNonWildcardRequestHeadersSupport[] =
    "cors_non_wildcard_request_headers_support";

// Policy setting whether default browser check should be disabled and default
// browser registration should take place.
inline constexpr char kDefaultBrowserSettingEnabled[] =
    "browser.default_browser_setting_enabled";

// Defines administrator-set availability of developer tools remote debugging.
inline constexpr char kDevToolsRemoteDebuggingAllowed[] =
    "devtools.remote_debugging.allowed";

// Boolean that specifies whether to disable CNAME lookups when generating
// Kerberos SPN.
inline constexpr char kDisableAuthNegotiateCnameLookup[] =
    "auth.disable_negotiate_cname_lookup";

// String which specifies where to store the disk cache.
inline constexpr char kDiskCacheDir[] = "browser.disk_cache_dir";
// Pref name for the policy specifying the maximal cache size.
inline constexpr char kDiskCacheSize[] = "browser.disk_cache_size";

// String specifying the secure DNS mode to use. Any string other than
// "secure" or "automatic" will be mapped to the default "off" mode.
inline constexpr char kDnsOverHttpsMode[] = "dns_over_https.mode";

// String containing a space-separated list of DNS over HTTPS templates to use
// in secure mode or automatic mode. If no templates are specified in automatic
// mode, we will attempt discovery of DoH servers associated with the configured
// insecure resolvers.
inline constexpr char kDnsOverHttpsTemplates[] = "dns_over_https.templates";

// Boolean that specifies whether to include the port in a generated Kerberos
// SPN.
inline constexpr char kEnableAuthNegotiatePort[] = "auth.enable_negotiate_port";

// Whether to enable sending referrers.
inline constexpr char kEnableReferrers[] = "enable_referrers";

// If false, disable Encrypted ClientHello (ECH) in TLS connections.
inline constexpr char kEncryptedClientHelloEnabled[] = "ssl.ech_enabled";

// A comma-separated list of ports on which outgoing connections will be
// permitted even if they would otherwise be blocked.
inline constexpr char kExplicitlyAllowedNetworkPorts[] =
    "net.explicitly_allowed_network_ports";

// Boolean that specifies whether cached (server) auth credentials are separated
// by NetworkAnonymizationKey.
inline constexpr char kGloballyScopeHTTPAuthCacheEnabled[] =
    "auth.globally_scoped_http_auth_cache_enabled";

// String that specifies the name of a custom GSSAPI library to load.
inline constexpr char kGSSAPILibraryName[] = "auth.gssapi_library_name";

// List of strings specifying which hosts are allowed to have H2 connections
// coalesced when client certs are also used. This follows rules similar to
// the URLBlocklist format for hostnames: a pattern with a leading dot (e.g.
// ".example.net") matches exactly the hostname following the dot (i.e. only
// "example.net"), and a pattern with no leading dot (e.g. "example.com")
// matches that hostname and all subdomains.
inline constexpr char kH2ClientCertCoalescingHosts[] =
    "ssl.client_certs.h2_coalescing_hosts";

// List of single-label hostnames that will skip the check to possibly upgrade
// from http to https.
inline constexpr char kHSTSPolicyBypassList[] =
    "hsts.policy.upgrade_bypass_list";

// Boolean that specifies whether IPv6 reachability check override is enabled.
inline constexpr char kIPv6ReachabilityOverrideEnabled[] =
    "net.ipv6_reachability_override_enabled";

// Preference to store proxy settings.
inline constexpr char kMaxConnectionsPerProxy[] =
    "net.max_connections_per_proxy";

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_LINUX)
// A boolean pref to determine whether or not the network service is running
// sandboxed.
inline constexpr char kNetworkServiceSandboxEnabled[] =
    "net.network_service_sandbox";
#endif  // BUILDFLAG(IS_WIN) || BUILDFLAG(IS_LINUX)

#if BUILDFLAG(IS_POSIX)
// Boolean that specifies whether NTLMv2 is enabled.
inline constexpr char kNtlmV2Enabled[] = "auth.ntlm_v2_enabled";
#endif  // BUILDFLAG(IS_POSIX)

// If false, disable post-quantum key agreement in TLS connections.
inline constexpr char kPostQuantumKeyAgreementEnabled[] =
    "ssl.post_quantum_enabled";

// Directory of the last profile used.
inline constexpr char kProfileLastUsed[] = "profile.last_used";

// Controls if the QUIC protocol is allowed.
inline constexpr char kQuicAllowed[] = "net.quic_allowed";

// Whether DNS Quick Check is disabled in proxy resolution.
//
// This is a performance optimization for WPAD (Web Proxy
// Auto-Discovery) which places a 1 second timeout on resolving the
// DNS for PAC script URLs.
//
// It is on by default, but can be disabled via the Policy option
// "WPADQuickCheckEnbled". There is no other UI for changing this
// preference.
//
// For instance, if the DNS resolution for 'wpad' takes longer than 1
// second, auto-detection will give up and fallback to the next proxy
// configuration (which could be manually configured proxy server
// rules, or an implicit fallback to DIRECT connections).
inline constexpr char kQuickCheckEnabled[] = "proxy.quick_check_enabled";

#if BUILDFLAG(IS_LINUX)
// Records whether the user has seen an HTTP auth "negotiate" header.
inline constexpr char kReceivedHttpAuthNegotiateHeader[] =
    "net.received_http_auth_negotiate_headers";
#endif  // BUILDFLAG(IS_LINUX)

// A dictionary pref that can contain a list of configured endpoints for
// reports to be sent to.
inline constexpr char kReportingEndpoints[] =
    "enterprise_reporting.reporting_endpoints";

// String specifying the minimum TLS version to negotiate. Supported values
// are "tls1.2", "tls1.3".
inline constexpr char kSSLVersionMin[] = "ssl.version_min";

// String specifying the maximum TLS version to negotiate. Supported values
// are "tls1.2", "tls1.3"
inline constexpr char kSSLVersionMax[] = "ssl.version_max";

}  // namespace prefs

#endif  // RADIUM_COMMON_PREF_NAMES_H_
