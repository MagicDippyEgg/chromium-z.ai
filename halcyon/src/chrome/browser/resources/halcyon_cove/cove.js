/**
 * The Cove — Halcyon private browsing greeting page logic.
 *
 * Very little happens here. The page is mostly text. The only interaction
 * is the "Open the Plume" link, which opens a new tab pointing at the Plume
 * (in the same Cove window).
 */

(() => {
  'use strict';

  function init() {
    const openPlume = document.getElementById('halcyon-cove-open-plume');
    if (openPlume) {
      openPlume.addEventListener('click', (e) => {
        e.preventDefault();
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonCoveOpenPlume', []);
        }
      });
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
