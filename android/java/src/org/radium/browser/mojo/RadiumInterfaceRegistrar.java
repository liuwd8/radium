// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.radium.browser.mojo;

import org.jni_zero.CalledByNative;

import org.chromium.blink.mojom.Authenticator;
import org.chromium.content_public.browser.InterfaceRegistrar;
import org.chromium.content_public.browser.RenderFrameHost;
import org.chromium.content_public.browser.WebContents;
import org.chromium.services.service_manager.InterfaceRegistry;

/** Registers mojo interface implementations exposed to C++ code at the Chrome layer. */
class RadiumInterfaceRegistrar {
    @CalledByNative
    private static void registerMojoInterfaces() {
        // InterfaceRegistrar.Registry.addWebContentsRegistrar(
        //         new ChromeWebContentsInterfaceRegistrar());
        // InterfaceRegistrar.Registry.addRenderFrameHostRegistrar(
        //         new ChromeRenderFrameHostInterfaceRegistrar());
    }

    // private static class ChromeWebContentsInterfaceRegistrar
    //         implements InterfaceRegistrar<WebContents> {
    //     @Override
    //     public void registerInterfaces(InterfaceRegistry registry, final WebContents webContents)
    //     {
    //         registry.addInterface(
    //                 ShareService.MANAGER, new ShareServiceImplementationFactory(webContents));
    //     }
    // }

    // private static class ChromeRenderFrameHostInterfaceRegistrar
    //         implements InterfaceRegistrar<RenderFrameHost> {
    //     @Override
    //     public void registerInterfaces(
    //             InterfaceRegistry registry, final RenderFrameHost renderFrameHost) {
    //         registry.addInterface(
    //                 PaymentRequest.MANAGER, new ChromePaymentRequestFactory(renderFrameHost));
    //         registry.addInterface(
    //                 InstalledAppProvider.MANAGER, new
    //                 InstalledAppProviderFactory(renderFrameHost));
    //         registry.addInterface(
    //                 Authenticator.MANAGER, new ChromeAuthenticatorFactory(renderFrameHost));
    //         registry.addInterface(
    //                 DigitalGoodsFactory.MANAGER, new
    //                 DigitalGoodsFactoryFactory(renderFrameHost));
    //     }
    // }
}
