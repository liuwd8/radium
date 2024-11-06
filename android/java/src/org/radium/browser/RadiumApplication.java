// Copyright 2024 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser;

import android.app.Application;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Process;

import org.radium.Radium.ProductConfig;
import org.radium.browser.metrics.UmaUtils;

import org.chromium.base.ApplicationStatus;
import org.chromium.base.CommandLine;
import org.chromium.base.CommandLineInitUtil;
import org.chromium.base.ContextUtils;
import org.chromium.base.IntentUtils;
import org.chromium.base.JNIUtils;
import org.chromium.base.Log;
import org.chromium.base.PathUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.base.memory.MemoryPressureMonitor;
import org.chromium.base.task.AsyncTask;
import org.chromium.base.version_info.VersionConstants;
import org.chromium.build.BuildConfig;
import org.chromium.build.NativeLibraries;
import org.chromium.components.embedder_support.application.FontPreloadingWorkaround;
import org.chromium.ui.base.ResourceBundle;

public class RadiumApplication extends Application {
    private static final String TAG = "RadiumApplication";

    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "radium";
    private static final String COMMAND_LINE_FILE = "chrome-command-line";

    @Override
    protected void attachBaseContext(Context base) {
        boolean isIsolatedProcess = ContextUtils.isIsolatedProcess();
        boolean isBrowserProcess = isBrowserProcess();
        Log.i(TAG,
                "version=%s (%s) minSdkVersion=%s isBundle=%s processName=%s isIsolatedProcess=%s",
                VersionConstants.PRODUCT_VERSION, BuildConfig.VERSION_CODE,
                BuildConfig.MIN_SDK_VERSION, BuildConfig.IS_BUNDLE, ContextUtils.getProcessName(),
                isIsolatedProcess);
        if (isBrowserProcess) {
            UmaUtils.recordMainEntryPointTime();
        }
        super.attachBaseContext(base);
        ContextUtils.initApplicationContext(this);

        // if (isBrowserProcess) {
        // // This must come as early as possible to avoid early loading of the native
        // library from
        // // failing unnoticed.
        // LibraryLoader.sLoadFailedCallback =
        // unsatisfiedLinkError -> {
        // Intent newIntent = new Intent();
        // newIntent.setComponent(
        // new ComponentName(
        // ContextUtils.getApplicationContext(),
        // LAUNCH_FAILED_ACTIVITY_CLASS_NAME));
        // newIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        // IntentUtils.safeStartActivity(
        // ContextUtils.getApplicationContext(), newIntent);
        // if (cannotLoadIn64Bit()) {
        // throw new RuntimeException(
        // "Starting in 64-bit mode requires the 64-bit native library. If"
        // + " the device is 64-bit only, see alternatives here: "
        // + "https://crbug.com/1303857#c7.",
        // unsatisfiedLinkError);
        // } else if (cannotLoadIn32Bit()) {
        // throw new RuntimeException(
        // "Starting in 32-bit mode requires the 32-bit native library.",
        // unsatisfiedLinkError);
        // }
        // throw unsatisfiedLinkError;
        // };
        // }

        maybeInitProcessType();

        AsyncTask.takeOverAndroidThreadPool();
        JNIUtils.setClassLoader(getClassLoader());
        ResourceBundle.setAvailablePakLocales(ProductConfig.LOCALES);
        LibraryLoader.getInstance().setLinkerImplementation(ProductConfig.USE_CHROMIUM_LINKER);

        if (isBrowserProcess) {
            checkAppBeingReplaced();
            PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);

            // Renderer and GPU processes have command line passed to them via IPC
            // (see ChildProcessService.java).
            CommandLineInitUtil.initCommandLine(
                    COMMAND_LINE_FILE, RadiumApplication::shouldUseDebugFlags);

            ApplicationStatus.initialize(this);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        // These can't go in attachBaseContext because Context.getApplicationContext() (which
        // they use under-the-hood) does not work until after it returns.
        FontPreloadingWorkaround.maybeInstallWorkaround(this);
        MemoryPressureMonitor.INSTANCE.registerComponentCallbacks();
    }

    public static boolean isBrowserProcess() {
        return !ContextUtils.getProcessName().contains(":");
    }

    // public static boolean cannotLoadIn64Bit() {
    //     if (LibraryLoader.sOverrideNativeLibraryCannotBeLoadedForTesting) {
    //         return true;
    //     }
    //     return Process.is64Bit() && !NativeLibraries.sSupport64Bit;
    // }

    // public static boolean cannotLoadIn32Bit() {
    //     return !Process.is64Bit() && !NativeLibraries.sSupport32Bit;
    // }

    private void maybeInitProcessType() {
        if (isBrowserProcess()) {
            LibraryLoader.getInstance().setLibraryProcessType(LibraryProcessType.PROCESS_BROWSER);
            return;
        }

        // Child processes set their own process type when bound.
        String processName = ContextUtils.getProcessName();
        if (processName.contains("privileged_process")
                || processName.contains("sandboxed_process")) {
            return;
        }

        // We must be in an isolated service process.
        LibraryLoader.getInstance().setLibraryProcessType(LibraryProcessType.PROCESS_CHILD);
    }

    private static Boolean shouldUseDebugFlags() {
        return false;
    }

    /** Ensure this application object is not out-of-date. */
    private static void checkAppBeingReplaced() {
        // During app update the old apk can still be triggered by broadcasts and spin up an
        // out-of-date application. Kill old applications in this bad state. See
        // http://crbug.com/658130 for more context and http://b.android.com/56296 for the bug.
        if (ContextUtils.getApplicationContext().getAssets() == null) {
            throw new RuntimeException("App out of date, getResources() null, closing app.");
        }
    }
}
