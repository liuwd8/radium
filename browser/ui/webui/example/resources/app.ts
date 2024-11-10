
import { CrLitElement } from '//resources/lit/v3_0/lit.rollup.js';

import { getCss } from './app.css.js';
import { getHtml } from './app.html.js';


export class ExampleApp extends CrLitElement {
  protected header_: String = 'My app';
  protected logo_: String = new URL('./assets/open-wc-logo.svg', import.meta.url).href;

  static get is() {
    return 'example-app';
  }

  static override get styles() {
    return getCss();
  }

  static override get properties() {
    return {
      header_: {type: String},
      logo_: {type: String},
    };
  }

  override render() {
    return getHtml.bind(this)();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'example-app': ExampleApp;
  }
}

customElements.define(ExampleApp.is, ExampleApp);
