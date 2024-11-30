// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser.tab;

import androidx.annotation.NonNull;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.JniType;
import org.jni_zero.NativeMethods;

import org.chromium.base.TraceEvent;
import org.chromium.components.embedder_support.view.ContentView;
import org.chromium.content_public.browser.WebContents;

public class TabImpl {
    private long mNativeTabAndroid;

    // /** Unique id of this tab (within its container). */
    // private final int mId = 0;

    // /** {@link WebContents} showing the current page, or {@code null} if the tab is frozen. */
    // private WebContents mWebContents;

    // /** The parent view of the ContentView and the InfoBarContainer. */
    // private ContentView mContentView;

    // // Content layer Delegates
    // private TabWebContentsDelegateAndroidImpl mWebContentsDelegate;

    // /** Builds the native counterpart to this class. */
    // private void initializeNative() {
    // if (mNativeTabAndroid == 0) {
    // TabImplJni.get().init(TabImpl.this, mId);
    // }
    // assert mNativeTabAndroid != 0;
    // }

    /**
     * @return The native pointer representing the native side of this {@link TabImpl} object.
     */
    @CalledByNative
    private long getNativePtr() {
        return mNativeTabAndroid;
    }

    @CalledByNative
    private void clearNativePtr() {
        assert mNativeTabAndroid != 0;
        mNativeTabAndroid = 0;
    }

    @CalledByNative
    private void setNativePtr(long nativePtr) {
        assert nativePtr != 0;
        assert mNativeTabAndroid == 0;
        mNativeTabAndroid = nativePtr;
    }

    // /**
    // * Initializes the {@link WebContents}. Completes the browser content components
    // initialization
    // * around a native WebContents pointer.
    // * <p>
    // * {@link #getNativePage()} will still return the {@link NativePage} if there is one.
    // * All initialization that needs to reoccur after a web contents swap should be added here.
    // * <p />
    // * NOTE: If you attempt to pass a native WebContents that does not have the same incognito
    // * state as this tab this call will fail.
    // *
    // * @param webContents The WebContents object that will initialize all the browser components.
    // */
    // private void initWebContents(@NonNull WebContents webContents) {
    // try {
    // TraceEvent.begin("ChromeTab.initWebContents");
    // // WebContents oldWebContents = mWebContents;
    // mWebContents = webContents;

    // // ContentView cv = ContentView.createContentView(mThemedApplicationContext,
    // // webContents); cv.setContentDescription(
    // // mThemedApplicationContext.getString(R.string.accessibility_content_view));
    // // mContentView = cv;
    // // webContents.setDelegates(PRODUCT_VERSION, new TabViewAndroidDelegate(this, cv), cv,
    // // getWindowAndroid(), WebContents.createDefaultInternalsHolder());
    // // hideNativePage(false, null);

    // // if (oldWebContents != null) {
    // // oldWebContents.setImportance(ChildProcessImportance.NORMAL);
    // // getWebContentsAccessibility(oldWebContents).setObscuredByAnotherView(false);
    // // }

    // // mWebContents.setImportance(mImportance);

    // // ContentUtils.setUserAgentOverride(mWebContents,
    // // calculateUserAgentOverrideOption(null) == UserAgentOverrideOption.TRUE);

    // // mContentView.addOnAttachStateChangeListener(mAttachStateChangeListener);
    // // updateInteractableState();

    // // mWebContentsDelegate = createWebContentsDelegate();

    // // // TODO(crbug.com/40942165): Find a better way of indicating this is a background tab
    // // // (or move the logic elsewhere).
    // // boolean isBackgroundTab = isDetached();

    // // assert mNativeTabAndroid != 0;
    // // TabImplJni.get().initWebContents(mNativeTabAndroid, isOffTheRecord(),
    // // isBackgroundTab,
    // // webContents, mWebContentsDelegate,
    // // new TabContextMenuPopulatorFactory(
    // // mDelegateFactory.createContextMenuPopulatorFactory(this), this));

    // mWebContents.notifyRendererPreferenceUpdate();
    // // mContentView.setImportantForAutofill(prepareAutofillProvider(webContents)
    // // ? View.IMPORTANT_FOR_AUTOFILL_YES
    // // : View.IMPORTANT_FOR_AUTOFILL_NO_EXCLUDE_DESCENDANTS);
    // // TabHelpers.initWebContentsHelpers(this);
    // // notifyContentChanged();

    // // if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.VANILLA_ICE_CREAM
    // // && ChromeFeatureList.isEnabled(SensitiveContentFeatures.SENSITIVE_CONTENT)
    // // && ChromeFeatureList.isEnabled(
    // // SensitiveContentFeatures.SENSITIVE_CONTENT_WHILE_SWITCHING_TABS)) {
    // // // Adding the observation has to happen after the native `initWebContents`, so
    // // that
    // // // the {@link SensitiveContentClient} is properly initialized.
    // // SensitiveContentClient.fromWebContents(webContents).addObserver(this);
    // // }
    // } finally {
    // TraceEvent.end("ChromeTab.initWebContents");
    // }
    // }

    // private TabWebContentsDelegateAndroidImpl createWebContentsDelegate() {
    // // TabWebContentsDelegateAndroid delegate =
    // // mDelegateFactory.createWebContentsDelegate(this);
    // return new TabWebContentsDelegateAndroidImpl(this, null);
    // }

    @NativeMethods
    public interface Natives {
        TabImpl fromWebContents(WebContents webContents);

        void init(TabImpl caller, int id);
    }
}
