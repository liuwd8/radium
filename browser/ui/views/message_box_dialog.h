// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_MESSAGE_BOX_DIALOG_H_
#define RADIUM_BROWSER_UI_VIEWS_MESSAGE_BOX_DIALOG_H_

#include <string_view>

#include "base/memory/raw_ptr.h"
#include "radium/browser/ui/simple_message_box.h"
#include "ui/views/widget/widget_observer.h"
#include "ui/views/window/dialog_delegate.h"

namespace views {
class MessageBoxView;
}

class MessageBoxDialog : public views::DialogDelegate,
                         public views::WidgetObserver {
 public:
  using MessageBoxResultCallback =
      base::OnceCallback<void(radium::MessageBoxResult result)>;

  MessageBoxDialog(const MessageBoxDialog&) = delete;
  MessageBoxDialog& operator=(const MessageBoxDialog&) = delete;

  static radium::MessageBoxResult Show(
      gfx::NativeWindow parent,
      std::u16string_view title,
      std::u16string_view message,
      radium::MessageBoxType type,
      std::u16string_view yes_text,
      std::u16string_view no_text,
      std::u16string_view checkbox_text,
      MessageBoxResultCallback callback = MessageBoxResultCallback());

  // views::DialogDelegate:
  std::u16string GetWindowTitle() const override;
  views::View* GetContentsView() override;
  bool ShouldShowCloseButton() const override;

  // views::WidgetObserver:
  void OnWidgetActivationChanged(views::Widget* widget, bool active) override;
  void OnWidgetDestroying(views::Widget* widget) override;

 private:
  MessageBoxDialog(std::u16string_view title,
                   std::u16string_view message,
                   radium::MessageBoxType type,
                   std::u16string_view yes_text,
                   std::u16string_view no_text,
                   std::u16string_view checkbox_text);
  ~MessageBoxDialog() override;

  void Run(MessageBoxResultCallback result_callback);
  void Done(radium::MessageBoxResult result);

  void OnDialogAccepted();

  // Widget:
  views::Widget* GetWidget() override;
  const views::Widget* GetWidget() const override;

  const std::u16string window_title_;
  const radium::MessageBoxType type_;
  raw_ptr<views::MessageBoxView> message_box_view_;
  MessageBoxResultCallback result_callback_;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_MESSAGE_BOX_DIALOG_H_
