// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_RADIUM_BROWSER_APPLICATION_MAC_H_
#define RADIUM_BROWSER_RADIUM_BROWSER_APPLICATION_MAC_H_

#ifdef __OBJC__

#import <AppKit/AppKit.h>
#include <stddef.h>

#import "base/mac/scoped_sending_event.h"
#import "base/message_loop/message_pump_apple.h"

@interface BrowserCrApplication
    : NSApplication <CrAppProtocol, CrAppControlProtocol>

// Our implementation of |-terminate:| only attempts to terminate the
// application, i.e., begins a process which may lead to termination. This
// method cancels that process.
- (void)cancelTerminate:(id)sender;

- (BOOL)voiceOverStateForTesting;

@end

#endif  // __OBJC__

namespace radium_browser_application_mac {

// To be used to instantiate BrowserCrApplication from C++ code.
void RegisterBrowserCrApp();

// Calls -[NSApp terminate:].
void Terminate();

// Cancels a termination started by |Terminate()|.
void CancelTerminate();

}  // namespace radium_browser_application_mac

#endif  // RADIUM_BROWSER_RADIUM_BROWSER_APPLICATION_MAC_H_
