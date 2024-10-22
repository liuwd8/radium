// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/frame/untitled_window_client_view.h"

#include "ui/base/metadata/metadata_impl_macros.h"

UntitledWindowClientView::UntitledWindowClientView()
    : views::ClientView(nullptr, nullptr) {}

UntitledWindowClientView::~UntitledWindowClientView() = default;

BEGIN_METADATA(UntitledWindowClientView)
END_METADATA
