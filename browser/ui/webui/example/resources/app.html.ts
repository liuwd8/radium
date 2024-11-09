// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { html } from '//resources/lit/v3_0/lit.rollup.js';

import type { ExampleAppElement } from './app.js';

export function getHtml(this: ExampleAppElement) {
  return html`
<div id="sidebar">
  <div class="cr-row">
  </div>
</div>

<div id="main"></div>`;
}
