// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_FRAME_NATIVE_UNTITLED_FRAME_FACTORY_H_
#define RADIUM_BROWSER_UI_VIEWS_FRAME_NATIVE_UNTITLED_FRAME_FACTORY_H_

class UntitledWidget;
class NativeUntitledFrame;

class NativeUntitledFrameFactory {
 public:
  NativeUntitledFrameFactory(const NativeUntitledFrameFactory&) = delete;
  NativeUntitledFrameFactory& operator=(const NativeUntitledFrameFactory&) =
      delete;

  // Construct a platform-specific implementation of this interface.
  static NativeUntitledFrame* CreateNativeUntitledFrame(
      UntitledWidget* untitled_widget);

 protected:
  NativeUntitledFrameFactory() = default;
  virtual ~NativeUntitledFrameFactory() = default;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_FRAME_NATIVE_UNTITLED_FRAME_FACTORY_H_
