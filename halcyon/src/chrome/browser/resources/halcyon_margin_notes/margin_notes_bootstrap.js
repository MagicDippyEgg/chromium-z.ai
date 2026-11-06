/**
 * Margin Notes bootstrap.
 *
 * This is the script that the user_scripts system injects into every
 * http(s) page (see patches/user_scripts_inject_margin_notes.patch). It runs
 * in an isolated world — it can see the page DOM but the page cannot see
 * its JS scope.
 *
 * The bootstrap:
 *   1. Injects the overlay HTML+CSS+JS (the margin_notes.* files) into the
 *      page as a Shadow DOM child of <html>.
 *   2. Sets up a `chrome.send` bridge that the overlay JS uses to talk to
 *      the browser process. The bridge is a small wrapper around the
 *      HalcyonMarginNotesHost Mojo interface exposed by the browser process
 *      (the integration patch registers it as a WebContents->page
 *      property so the isolated world can call it).
 *
 * The bridge does NOT grant the host page any new capabilities. It only
 * accepts messages whose names start with `halcyonMargin`; everything else
 * is dropped.
 */

(() => {
  'use strict';

  // Don't run on chrome:// pages, file:// pages, or the Cove. The
  // integration patch already filters by URL, but we double-check here.
  if (!location || !location.protocol.startsWith('http')) return;

  // Don't run on pages that opted out via <meta name="halcyon-margin-notes"
  // content="off">. (Some sites — privacy-focused PDF viewers, exam
  // portals — want no overlay at all.)
  const meta = document.querySelector('meta[name="halcyon-margin-notes"]');
  if (meta && meta.content && meta.content.toLowerCase() === 'off') return;

  // Don't run twice.
  if (window.__halcyonMarginBootstrapped) return;
  window.__halcyonMarginBootstrapped = true;

  // --- Provide a `chrome.send` shim in the isolated world ---------------

  // In Chromium, isolated-world content scripts have access to a limited
  // chrome.* API surface, but not chrome.send (which is a WebUI thing).
  // The Halcyon integration patch registers a `halcyonMarginSend(name, args)`
  // function on the isolated world's window object that posts a Mojo
  // message to the browser process. We map chrome.send to it for the
  // overlay JS, which is written against chrome.send to match the WebUI
  // convention.
  if (!window.chrome) window.chrome = {};
  if (!window.chrome.send && typeof window.halcyonMarginSend === 'function') {
    window.chrome.send = (name, args) => window.halcyonMarginSend(name, args || []);
  }

  // --- Inject the overlay ----------------------------------------------

  // The overlay HTML+CSS+JS is bundled as a single text resource. The
  // integration patch sets `window.__HALCYON_MARGIN_HTML__` to the contents
  // of margin_notes.html (which itself references margin_notes.css and
  // margin_notes.js, inlined by the grit resource packer).
  //
  // margin_notes.js (the overlay logic) attaches the rest.
  const overlayScript = document.createElement('script');
  overlayScript.textContent = `(${marginNotesBootstrap.toString()})();`;
  // The script is added to the page's main world; the overlay itself is
  // a Shadow DOM so it's style-isolated either way.
  (document.head || document.documentElement).appendChild(overlayScript);
  overlayScript.remove();

  function marginNotesBootstrap() {
    // margin_notes.js is the actual overlay logic. It expects the global
    // window.__HALCYON_MARGIN_HTML__ to be set with the overlay HTML.
    if (typeof window.__halcyonMarginRun === 'function') {
      window.__halcyonMarginRun();
    }
  }
})();
