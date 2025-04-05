// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/dragging/drag_context.h"

#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/widget.h"

DragContext::DragContext() {
  SetProperty(views::kViewIgnoredByLayoutKey, true);
}

DragContext::~DragContext() = default;

void DragContext::AddedToWidget() {
  // Stow a pointer to this object onto the window handle so that we can get at
  // it later when all we have is a native view.
  GetWidget()->SetNativeWindowProperty(DragContext::kDragContext, this);
}

BEGIN_METADATA(DragContext)
END_METADATA
