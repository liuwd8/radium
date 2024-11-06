// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser;

import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import org.radium.R;
import org.radium.ShellManager;

import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.content_public.browser.BrowserStartupController;
import org.chromium.ui.base.ActivityWindowAndroid;
import org.chromium.ui.base.IntentRequestTracker;

public class RadiumShellActivity extends AppCompatActivity {
    private static final String TAG = "RadiumShellActivity";
    private static final String ACTIVE_SHELL_URL_KEY = "activeUrl";

    private ShellManager mShellManager;
    private ActivityWindowAndroid mWindowAndroid;
    private IntentRequestTracker mIntentRequestTracker;
    private String mStartupUrl;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LibraryLoader.getInstance().ensureInitialized();
        setContentView(R.layout.radium_shell_activity);

        mShellManager = findViewById(R.id.shell_container);
        final boolean listenToActivityState = true;
        mIntentRequestTracker = IntentRequestTracker.createFromActivity(this);
        mWindowAndroid =
                new ActivityWindowAndroid(this, listenToActivityState, mIntentRequestTracker,
                        /* insetObserver= */ null);
        mIntentRequestTracker.restoreInstanceState(savedInstanceState);
        mShellManager.setWindow(mWindowAndroid);
        // Set up the animation placeholder to be the SurfaceView. This disables the
        // SurfaceView's 'hole' clipping during animations that are notified to the window.
        mWindowAndroid.setAnimationPlaceholderView(
                mShellManager.getContentViewRenderView().getSurfaceView());

        mStartupUrl = "https://pc.weixin.qq.com";

        BrowserStartupController.getInstance().startBrowserProcessesAsync(
                LibraryProcessType.PROCESS_BROWSER, true, false,
                new BrowserStartupController.StartupCallback() {
                    @Override
                    public void onSuccess() {
                        finishInitialization(savedInstanceState);
                    }

                    @Override
                    public void onFailure() {
                        initializationFailed();
                    }
                });
    }

    private void finishInitialization(Bundle savedInstanceState) {
        String shellUrl;
        if (!TextUtils.isEmpty(mStartupUrl)) {
            shellUrl = mStartupUrl;
        } else {
            shellUrl = ShellManager.DEFAULT_SHELL_URL;
        }
        if (savedInstanceState != null && savedInstanceState.containsKey(ACTIVE_SHELL_URL_KEY)) {
            shellUrl = savedInstanceState.getString(ACTIVE_SHELL_URL_KEY);
        }
        mShellManager.launchShell(shellUrl);
    }

    private void initializationFailed() {
        Log.e(TAG, "ContentView initialization failed.");
        Toast.makeText(RadiumShellActivity.this, R.string.browser_process_initialization_failed,
                     Toast.LENGTH_SHORT)
                .show();
        finish();
    }
}
