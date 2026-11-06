/**
 * Halcyon icon custom element.
 *
 * <halcyon-icon name="lantern"></halcyon-icon>
 *
 * Inlines the SVG for the named icon into the page so CSS can color and
 * animate its strokes via `currentColor`. Names map to icons in
 * chrome/browser/resources/halcyon_shared/icons/icons.svg.
 *
 * The icons are bundled into a single SVG sprite (`icons.svg`) at build time
 * and shipped as a single small file. This element fetches the sprite once,
 * caches it, and inlines the relevant <svg> for each <halcyon-icon>.
 */

(() => {
  const ICONS_URL = 'chrome://resources/halcyon_shared/icons/icons.svg';

  /** @type {Promise<string>|null} */
  let spritePromise = null;

  /** @type {Map<string, string>} name -> raw <svg>...</svg> markup */
  const iconCache = new Map();

  /** @return {Promise<string>} */
  function fetchSprite() {
    if (spritePromise) return spritePromise;
    spritePromise = fetch(ICONS_URL)
      .then(r => r.ok ? r.text() : '')
      .catch(() => '');
    return spritePromise;
  }

  /** @param {string} name @return {string|null} */
  function lookupInSprite(spriteText, name) {
    // Parse once; cache per-name.
    if (iconCache.has(name)) return iconCache.get(name) || null;
    const parser = new DOMParser();
    const doc = parser.parseFromString(spriteText, 'image/svg+xml');
    const node = doc.querySelector(`svg[data-icon="${name}"]`);
    if (!node) {
      iconCache.set(name, '');
      return null;
    }
    // Strip the data-icon attribute (used only for lookup).
    node.removeAttribute('data-icon');
    const markup = node.outerHTML;
    iconCache.set(name, markup);
    return markup;
  }

  class HalcyonIcon extends HTMLElement {
    static get observedAttributes() { return ['name', 'size']; }

    constructor() {
      super();
      this.attachShadow({mode: 'open'});
    }

    /** @param {string} name @param {string=} _oldVal @param {string=} _newVal */
    async attributeChangedCallback(name, _oldVal, _newVal) {
      if (name === 'name' || name === 'size') {
        await this.render();
      }
    }

    async render() {
      const iconName = this.getAttribute('name') || '';
      const size = this.getAttribute('size') || '20';
      const sprite = await fetchSprite();
      const svg = lookupInSprite(sprite, iconName);
      if (!svg) {
        this.shadowRoot.innerHTML = `<span style="display:inline-block;width:${size}px;height:${size}px"></span>`;
        return;
      }
      // Inject size; the sprite SVGs are 24x24 with no width/height set.
      const sized = svg.replace('<svg ', `<svg width="${size}" height="${size}" `);
      this.shadowRoot.innerHTML = sized;
    }
  }

  customElements.define('halcyon-icon', HalcyonIcon);
})();
