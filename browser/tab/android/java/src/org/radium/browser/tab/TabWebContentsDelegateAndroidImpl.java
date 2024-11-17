// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser.tab;

import org.jni_zero.CalledByNative;
import org.jni_zero.JniType;
import org.jni_zero.NativeMethods;

public class TabWebContentsDelegateAndroidImpl extends TabWebContentsDelegateAndroid {
    // private final TabImpl mTab;

    private final TabWebContentsDelegateAndroid mDelegate;

    public TabWebContentsDelegateAndroidImpl(TabImpl tab, TabWebContentsDelegateAndroid delegate) {
        // mTab = tab;
        mDelegate = delegate;
    }

    /**
     * @return Night mode enabled/disabled for this Tab. To be used to propagate
     *         the preferred color scheme to the renderer.
     */
    @CalledByNative
    @Override
    protected boolean isNightModeEnabled() {
        return mDelegate.isNightModeEnabled();
    }

    /**
     * @return web preference for force dark mode.
     */
    @CalledByNative
    @Override
    protected boolean isForceDarkWebContentEnabled() {
        return mDelegate.isForceDarkWebContentEnabled();
    }
}
