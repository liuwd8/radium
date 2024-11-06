// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import org.radium.R;

public class EmptyActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setTheme(R.style.Theme_Chromium_WithWindowAnimation);
        setContentView(R.layout.activity_empty);
    }
}
