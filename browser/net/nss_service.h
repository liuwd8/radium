// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_NET_NSS_SERVICE_H_
#define RADIUM_BROWSER_NET_NSS_SERVICE_H_

#include <memory>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "build/buildflag.h"
#include "components/keyed_service/core/keyed_service.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace net {
class NSSCertDatabase;
}  // namespace net

// `NssCertDatabaseGetter` is a callback that MUST only be invoked on the IO
// thread, and will either synchronously return the associated
// `NSSCertDatabase*` (if available), or nullptr along with a commitment to
// asynchronously invoke the caller-supplied callback once the
// `NSSCertDatabase*` has been initialized.
// Ownership of the `NSSCertDatabase` is not transferred, and the lifetime
// should only be considered valid for the current Task.
//
// TODO(crbug.com/40753707): Provide better lifetime guarantees.
using NssCertDatabaseGetter = base::OnceCallback<net::NSSCertDatabase*(
    base::OnceCallback<void(net::NSSCertDatabase*)> callback)>;

// Service that owns and initializes the per-`BrowserContext` certificate
// database.
// On some platforms, this may be a per-`BrowserContext` `KeyedService` that
// returns a system-wide shared `NSSCertDatabase`, if the configuration is
// system-wide.
class NssService : public KeyedService {
 public:
  explicit NssService(content::BrowserContext* context);
  NssService(const NssService&) = delete;
  NssService& operator=(const NssService&) = delete;
  ~NssService() override;

  // Returns an `NssCertDatabaseGetter` that may only be invoked on the IO
  // thread. To avoid UAF, the getter must be immediately posted to the IO
  // thread and then invoked.  While the returned getter must be invoked on
  // the IO thread, this method itself may only be invoked on the UI thread,
  // where the NssService lives.
  virtual NssCertDatabaseGetter CreateNSSCertDatabaseGetterForIOThread();

  // Unsafely returns the `NssCertDatabase` directly to the caller (on the UI
  // thread). This is unsafe, because if the `content::BrowserContext` / this
  // `KeyedService` has begun shutting down, the `NssCertDatabase` may no
  // longer be valid. For unit tests, this is simply a convenience helper when
  // running everything on a single thread, but is not safe to use for
  // production.
  void UnsafelyGetNSSCertDatabaseForTesting(
      base::OnceCallback<void(net::NSSCertDatabase*)> callback);
};

#endif  // RADIUM_BROWSER_NET_NSS_SERVICE_H_
