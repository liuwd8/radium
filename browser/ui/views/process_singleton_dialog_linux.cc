// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/process_singleton_dialog_linux.h"

#include "base/run_loop.h"
#include "radium/browser/ui/simple_message_box.h"
#include "radium/grit/radium_strings.h"
#include "ui/base/l10n/l10n_util.h"

bool ShowProcessSingletonDialog(const std::u16string& message,
                                const std::u16string& relaunch_text) {
  bool result = radium::ShowMessageBoxWithButtonText(
                    nullptr, l10n_util::GetStringUTF16(IDS_PRODUCT_NAME),
                    message, relaunch_text,
                    l10n_util::GetStringUTF16(IDS_PROFILE_IN_USE_LINUX_QUIT)) ==
                radium::MESSAGE_BOX_RESULT_YES;

  // Avoids gpu_process_transport_factory.cc(153)] Check failed:
  // per_compositor_data_.empty() when quit is chosen.
  base::RunLoop().RunUntilIdle();

  return result;
}
