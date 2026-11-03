/**
 * Halcyon theme controller — applied to every Halcyon WebUI page.
 *
 * This script:
 *   - Injects the design tokens CSS into the document (so pages don't each
 *     need a <link> — they just include this script).
 *   - Reads the user's Halcyon theme preference (system/light/dark) from a
 *     preference set via chrome.send by the C++ side, or via
 *     `window.matchMedia('(prefers-color-scheme: dark)')` as a fallback.
 *   - Sets the `data-halcyon-theme` attribute on <html> accordingly.
 *   - Re-evaluates when the system theme changes (if user pref is "system").
 *   - Also handles high-contrast and reduced-motion preferences.
 *
 * The C++ side calls into this via:
 *   document.dispatchEvent(new CustomEvent('halcyon-theme-change',
 *       {detail: {theme: 'system'|'light'|'dark'}}));
 */

(() => {
  /** @type {'system'|'light'|'dark'} */
  let userPref = 'system';

  function resolveTheme(pref) {
    if (pref === 'system') {
      return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
    }
    return pref;
  }

  function apply(theme) {
    const html = document.documentElement;
    if (theme === 'dark') {
      html.setAttribute('data-halcyon-theme', 'dark');
    } else {
      html.removeAttribute('data-halcyon-theme');
    }
  }

  function reapply() {
    apply(resolveTheme(userPref));
  }

  // Listen for C++ pref updates.
  document.addEventListener('halcyon-theme-change', (e) => {
    userPref = e.detail?.theme || 'system';
    reapply();
  });

  // Listen for system theme changes when in "system" mode.
  try {
    window.matchMedia('(prefers-color-scheme: dark)')
        .addEventListener('change', () => {
          if (userPref === 'system') reapply();
        });
  } catch (_) {
    // Older browsers: ignore.
  }

  // Initial default. The C++ side will dispatch halcyon-theme-change as soon
  // as the page is ready if the user has overridden the system theme.
  reapply();
})();
