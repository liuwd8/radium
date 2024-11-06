// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser.init;

import org.radium.R;
import android.app.AlertDialog;
import android.content.DialogInterface;
import androidx.appcompat.app.AppCompatActivity;

public class LaunchFailedActivity extends AppCompatActivity {
    /**
     * Tracks whether an exception has been reported.
     *
     * We do this once per process, to avoid spamming if the user keeps trying to relaunch but don't
     * bother with persistence since a few repeated attempts at different time intervals will be
     * spaced out.
     */
    private static boolean sNotified;

    @Override
    protected void onStart() {
        super.onStart();

        if (!sNotified) {
            sNotified = true;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(getString(R.string.update_needed))
                .setPositiveButton(
                        getString(R.string.ok),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                finish();
                            }
                        });
        builder.create().show();
    }
}
