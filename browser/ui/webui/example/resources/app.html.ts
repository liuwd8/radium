// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { html } from '//resources/lit/v3_0/lit.rollup.js';

import type { ExampleApp } from './app.js';

export function getHtml(this: ExampleApp) {
  return html`
  <main>
    <div class="logo"><img alt="open-wc logo" src=${this.logo_} /></div>
    <h1>${this.header_}</h1>

    <p>Edit <code>src/ExampleApp.ts</code> and save to reload.</p>
    <a
      class="app-link"
      href="https://open-wc.org/guides/developing-components/code-examples"
      target="_blank"
      rel="noopener noreferrer"
    >
      Code examples
    </a>
  </main>

  <p class="app-footer">
    🚽 Made with love by
    <a
      target="_blank"
      rel="noopener noreferrer"
      href="https://github.com/open-wc"
      >open-wc</a
    >.
  </p>
`;
}
