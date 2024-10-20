// Copyright 2024 The Chromium Wechat Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_GLOBAL_FEATURES_H_
#define RADIUM_BROWSER_GLOBAL_FEATURES_H_

class GlobalFeatures {
 public:
  explicit GlobalFeatures();
  GlobalFeatures(const GlobalFeatures&) = delete;
  GlobalFeatures& operator=(const GlobalFeatures&) = delete;

  ~GlobalFeatures();

 private:
};

#endif  // RADIUM_BROWSER_GLOBAL_FEATURES_H_
