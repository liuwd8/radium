// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_SESSION_DATA_H_
#define RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_SESSION_DATA_H_

class DragSessionData final {
 public:
  explicit DragSessionData();
  DragSessionData(const DragSessionData&) = delete;
  DragSessionData& operator=(const DragSessionData&) = delete;

  ~DragSessionData();

 private:
};

#endif  // RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_SESSION_DATA_H_
