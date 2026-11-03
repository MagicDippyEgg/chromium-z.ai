/**
 * Halcyon Settings — page logic.
 *
 * This script wires up every control on the Settings page to the underlying
 * preference store via chrome.send('halcyonSettingsPrefGet/Set').
 *
 * All controls carry a `data-pref` attribute naming the preference they bind
 * to. Toggles are <halcyon-toggle>; selects are <select>; buttons are
 * individual elements with IDs.
 *
 * The C++ side (HalcyonSettingsMessageHandler) is responsible for:
 *   - returning the current pref value when asked
 *   - persisting pref changes
 *   - dispatching a 'halcyon-settings-pref-changed' CustomEvent when any
 *     pref changes externally (e.g., from another window)
 *
 * The page re-renders the affected row on a pref-changed event.
 */

(() => {
  'use strict';

  // --- Halcyon toggle element -------------------------------------------

  class HalcyonToggle extends HTMLElement {
    static get observedAttributes() { return ['aria-pressed']; }

    constructor() {
      super();
      this.attachShadow({mode: 'open'});
      this.shadowRoot.innerHTML = `
        <style>
          :host { display: inline-block; }
          .halcyon-toggle {
            position: relative;
            width: 44px;
            height: 24px;
            border-radius: 12px;
            background: var(--halcyon-surface-3);
            border: 1px solid var(--halcyon-border);
            cursor: pointer;
            transition:
              background var(--halcyon-dur-fast) var(--halcyon-ease),
              border-color var(--halcyon-dur-fast) var(--halcyon-ease);
          }
          .halcyon-toggle-thumb {
            position: absolute;
            top: 2px;
            left: 2px;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--halcyon-text-3);
            transition:
              transform var(--halcyon-dur-fast) var(--halcyon-ease),
              background var(--halcyon-dur-fast) var(--halcyon-ease);
          }
          :host([aria-pressed="true"]) .halcyon-toggle {
            background: var(--halcyon-accent);
            border-color: var(--halcyon-accent);
          }
          :host([aria-pressed="true"]) .halcyon-toggle-thumb {
            transform: translateX(20px);
            background: #FFFFFF;
          }
          :host(:focus-visible) .halcyon-toggle {
            box-shadow: var(--halcyon-focus-ring);
          }
          .halcyon-toggle[disabled] {
            opacity: 0.4;
            cursor: not-allowed;
          }
        </style>
        <div class="halcyon-toggle" role="switch" tabindex="0">
          <div class="halcyon-toggle-thumb"></div>
        </div>
      `;
      this._toggle = this.shadowRoot.querySelector('.halcyon-toggle');
    }

    connectedCallback() {
      this._toggle.addEventListener('click', (e) => this._onClick(e));
      this._toggle.addEventListener('keydown', (e) => {
        if (e.key === ' ' || e.key === 'Enter') {
          e.preventDefault();
          this._onClick(e);
        }
      });
      // Read the initial state from the data-default attribute if no pref
      // has been loaded yet. The SettingsBridge will set the real value
      // asynchronously.
      const def = this.getAttribute('data-default');
      this._setPressed(def === 'true', /*announce=*/false);
    }

    _onClick(e) {
      this._setPressed(!(this.getAttribute('aria-pressed') === 'true'));
      this._emitChange();
    }

    _setPressed(on, announce = true) {
      this.setAttribute('aria-pressed', on ? 'true' : 'false');
      if (announce) {
        this.dispatchEvent(new CustomEvent('halcyon-toggle-change', {
          bubbles: true,
          detail: {value: on, pref: this.getAttribute('data-pref')},
        }));
      }
    }

    _emitChange() {
      this.dispatchEvent(new CustomEvent('halcyon-toggle-change', {
        bubbles: true,
        detail: {
          value: this.getAttribute('aria-pressed') === 'true',
          pref: this.getAttribute('data-pref'),
        },
      }));
    }
  }

  customElements.define('halcyon-toggle', HalcyonToggle);

  // --- Settings bridge ---------------------------------------------------

  const pendingPrefGets = new Map();  // pref name -> resolve fn

  /** @param {string} prefName @return {Promise<any>} */
  function getPref(prefName) {
    return new Promise((resolve) => {
      pendingPrefGets.set(prefName, resolve);
      if (typeof chrome !== 'undefined' && chrome.send) {
        chrome.send('halcyonSettingsPrefGet', [prefName]);
      } else {
        // Standalone: resolve with default (false / undefined)
        resolve(undefined);
      }
    });
  }

  /** @param {string} prefName @param {any} value */
  function setPref(prefName, value) {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonSettingsPrefSet', [prefName, value]);
    }
  }

  /** C++ calls window.halcyon.settingsPrefGetResult(pref, value). */
  window.halcyon = window.halcyon || {};
  window.halcyon.settingsPrefGetResult = (pref, value) => {
    const resolve = pendingPrefGets.get(pref);
    if (resolve) {
      pendingPrefGets.delete(pref);
      resolve(value);
    }
  };

  /** C++ calls window.halcyon.settingsPrefChanged(pref, value). */
  window.halcyon.settingsPrefChanged = (pref, value) => {
    applyPrefToControl(pref, value);
  };

  // --- Wiring -----------------------------------------------------------

  async function init() {
    // Wire toggles.
    for (const t of document.querySelectorAll('halcyon-toggle')) {
      const pref = t.getAttribute('data-pref');
      const value = await getPref(pref);
      t.setAttribute('aria-pressed', value ? 'true' : 'false');
      t.addEventListener('halcyon-toggle-change', (e) => {
        setPref(e.detail.pref, e.detail.value);
      });
    }

    // Wire selects.
    for (const s of document.querySelectorAll('select[data-pref]')) {
      const pref = s.getAttribute('data-pref');
      const value = await getPref(pref);
      if (value !== undefined) s.value = value;
      s.addEventListener('change', () => setPref(pref, s.value));
    }

    // Wire color swatches.
    const swatches = document.querySelectorAll('.halcyon-color-swatch');
    for (const sw of swatches) {
      sw.addEventListener('click', () => {
        for (const s of swatches) s.setAttribute('aria-pressed', 'false');
        sw.setAttribute('aria-pressed', 'true');
        setPref('halcyon.appearance.accent', sw.dataset.value);
      });
    }

    // Wire the ToC scroll-spy.
    wireScrollSpy();

    // Wire action buttons.
    document.getElementById('halcyon-wash-tides')?.addEventListener('click', () => {
      if (confirm('Wash all Tides? This can\'t be undone.')) {
        chrome.send('halcyonSettingsWashTides', []);
      }
    });
    document.getElementById('halcyon-choose-download-folder')?.addEventListener('click', () => {
      chrome.send('halcyonSettingsChooseDownloadFolder', []);
    });
    document.getElementById('halcyon-lantern-report')?.addEventListener('click', (e) => {
      e.preventDefault();
      chrome.send('halcyonSettingsOpenLanternReport', []);
    });
    document.getElementById('halcyon-open-dev-tools')?.addEventListener('click', (e) => {
      e.preventDefault();
      chrome.send('halcyonSettingsOpenDevTools', []);
    });
    document.getElementById('halcyon-about-release-notes')?.addEventListener('click', (e) => {
      e.preventDefault();
      chrome.send('halcyonSettingsOpenReleaseNotes', []);
    });
    document.getElementById('halcyon-about-report-problem')?.addEventListener('click', (e) => {
      e.preventDefault();
      chrome.send('halcyonSettingsOpenReportProblem', []);
    });

    // Listen for global pref changes (e.g., from the toolbar toggles).
    document.addEventListener('halcyon-settings-pref-changed', (e) => {
      applyPrefToControl(e.detail.pref, e.detail.value);
    });
  }

  function applyPrefToControl(pref, value) {
    const t = document.querySelector(`halcyon-toggle[data-pref="${pref}"]`);
    if (t) t.setAttribute('aria-pressed', value ? 'true' : 'false');
    const s = document.querySelector(`select[data-pref="${pref}"]`);
    if (s && value !== undefined) s.value = value;
  }

  function wireScrollSpy() {
    const toc = document.getElementById('halcyon-toc');
    const links = [...toc.querySelectorAll('a')];
    const sections = links.map(l => document.querySelector(l.getAttribute('href'))).filter(Boolean);

    const observer = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          const id = entry.target.id;
          links.forEach((l) => l.classList.toggle('active', l.getAttribute('href') === '#' + id));
        }
      });
    }, {rootMargin: '-80px 0px -60% 0px'});
    sections.forEach((s) => observer.observe(s));
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
