// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser.tab;

import org.chromium.components.embedder_support.delegate.WebContentsDelegateAndroid;
import org.chromium.content_public.browser.WebContents;

public abstract class TabWebContentsDelegateAndroid extends WebContentsDelegateAndroid {
    /**
     * @return Night mode enabled/disabled for this Tab. To be used to propagate
     *         the preferred color scheme to the renderer.
     */
    protected boolean isNightModeEnabled() {
      return false;
    }

    /**
     * @return True if auto-darkening may be applied to web contents per Chrome browser settings.
     */
    protected boolean isForceDarkWebContentEnabled() {
      return false;
    }
}
