/**
 * The Plume — Halcyon new tab page logic.
 *
 * Responsibilities:
 *   - Set the greeting based on local time-of-day.
 *   - Set the date in long format (English: "Tuesday, the third of November";
 *     other locales use locale-appropriate long format).
 *   - Pick today's quote (one per local-day, from a curated JSON file).
 *   - Wire up the search field to the omnibox Autocomplete API.
 *   - Persist the "today's intention" to local storage, keyed by date.
 *   - Render the recently-visited list, fading in on hover.
 *   - Wire up the Lantern and Cove footer links.
 *
 * Communication with C++:
 *   - chrome.send('halcyonSearchBoxSubmit', [query])  — user hit Enter
 *   - chrome.send('halcyonSearchBoxRequestSuggestions', [input])  — input changed
 *   - chrome.send('halcyonOpenUrl', [url, disposition])  — suggestion clicked
 *   - chrome.send('halcyonOpenCove', [])  — Cove link clicked
 *   - chrome.send('halcyonIntentionChanged', [date, intention])  — debounced
 *
 *   - chrome.send('halcyonRecentVisitedRequest', [])  — request the list
 *   - C++ calls window.halcyon.recentVisitedResult(items) with the response.
 *   - chrome.send('halcyonSuggestionsResult', [query, items])
 *     C++ calls window.halcyon.suggestionsResult(query, items) with results.
 *
 * The page itself does NOT do any networking. It is purely a view layer.
 */

(() => {
  'use strict';

  // --- Time-of-day greeting ---------------------------------------------

  function greetingText() {
    const h = new Date().getHours();
    if (h >= 5 && h < 12) return 'Good morning';
    if (h >= 12 && h < 17) return 'Good afternoon';
    if (h >= 17 && h < 22) return 'Good evening';
    return 'Good evening';
  }

  // --- Long date ---------------------------------------------------------

  function longDate() {
    const d = new Date();
    const fmt = new Intl.DateTimeFormat(navigator.language || 'en', {
      weekday: 'long',
      day: 'numeric',
      month: 'long',
    });
    return fmt.format(d);
  }

  // --- Quote of the day --------------------------------------------------

  let quotesCache = null;

  /** @return {Promise<{text: string, attribution: string}>} */
  async function quoteForToday() {
    if (!quotesCache) {
      try {
        const r = await fetch('chrome://resources/halcyon_ntp/quotes.json');
        if (!r.ok) throw new Error('quotes fetch failed');
        quotesCache = await r.json();
      } catch (e) {
        return {
          text: 'Attention is the rarest and purest form of generosity.',
          attribution: 'Simone Weil',
        };
      }
    }
    // Deterministic per-day pick so the same user sees the same quote all day.
    const d = new Date();
    const dayIndex = Math.floor(Date.UTC(d.getFullYear(), d.getMonth(), d.getDate()) / 86400000);
    const idx = dayIndex % quotesCache.length;
    return quotesCache[idx];
  }

  // --- Intention ---------------------------------------------------------

  const INTENTION_PREFIX = 'halcyon-intention-';

  function intentionKey() {
    const d = new Date();
    return INTENTION_PREFIX + d.toISOString().slice(0, 10);
  }

  function loadIntention() {
    try {
      return localStorage.getItem(intentionKey()) || '';
    } catch (_) {
      return '';
    }
  }

  function saveIntention(value) {
    try {
      localStorage.setItem(intentionKey(), value);
    } catch (_) {
      // Quota or privacy mode — silently ignore.
    }
  }

  // Clean up intentions older than 30 days.
  function cleanOldIntentions() {
    try {
      const cutoff = Date.now() - 30 * 86400000;
      for (let i = localStorage.length - 1; i >= 0; i--) {
        const k = localStorage.key(i);
        if (!k || !k.startsWith(INTENTION_PREFIX)) continue;
        const dateStr = k.slice(INTENTION_PREFIX.length);
        const t = Date.parse(dateStr);
        if (!isNaN(t) && t < cutoff) localStorage.removeItem(k);
      }
    } catch (_) {}
  }

  // --- Search field ------------------------------------------------------

  /** @type {HTMLInputElement} */
  let searchInput;
  /** @type {HTMLUListElement} */
  let suggestionsList;

  let suggestionDebounce = null;
  let activeSuggestionIdx = -1;

  function wireSearch() {
    searchInput = document.getElementById('halcyon-search-input');
    suggestionsList = document.getElementById('halcyon-search-suggestions');

    searchInput.addEventListener('input', () => {
      const value = searchInput.value;
      if (suggestionDebounce) clearTimeout(suggestionDebounce);
      suggestionDebounce = setTimeout(() => requestSuggestions(value), 80);
      if (!value) hideSuggestions();
    });

    searchInput.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') {
        e.preventDefault();
        submitSearch(searchInput.value);
      } else if (e.key === 'ArrowDown' && suggestionsList.children.length) {
        e.preventDefault();
        activeSuggestionIdx = Math.min(activeSuggestionIdx + 1, suggestionsList.children.length - 1);
        highlightSuggestion();
      } else if (e.key === 'ArrowUp' && suggestionsList.children.length) {
        e.preventDefault();
        activeSuggestionIdx = Math.max(activeSuggestionIdx - 1, 0);
        highlightSuggestion();
      } else if (e.key === 'Escape') {
        hideSuggestions();
        searchInput.select();
      }
    });

    searchInput.addEventListener('focus', () => {
      if (searchInput.value) requestSuggestions(searchInput.value);
    });

    // Click outside closes suggestions.
    document.addEventListener('click', (e) => {
      if (!e.target.closest('#halcyon-search-section')) hideSuggestions();
    });

    // "/" focuses search.
    document.addEventListener('keydown', (e) => {
      if (e.key === '/' && document.activeElement !== searchInput) {
        e.preventDefault();
        searchInput.focus();
      }
    });
  }

  function requestSuggestions(value) {
    if (!value || !value.trim()) {
      hideSuggestions();
      return;
    }
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonSearchBoxRequestSuggestions', [value]);
    }
    // Otherwise (running standalone), no suggestions. The field still works.
  }

  function submitSearch(value) {
    if (!value || !value.trim()) return;
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonSearchBoxSubmit', [value.trim()]);
    }
  }

  /** @param {string} query @param {Array<{text: string, type: string, url: string}>} items */
  window.halcyon = window.halcyon || {};
  window.halcyon.suggestionsResult = (query, items) => {
    if (searchInput.value !== query) return;  // stale
    if (!items || !items.length) { hideSuggestions(); return; }
    suggestionsList.innerHTML = '';
    items.slice(0, 8).forEach((item, i) => {
      const li = document.createElement('li');
      li.setAttribute('role', 'option');
      li.dataset.url = item.url;
      li.innerHTML = `<span class="halcyon-suggestion-text">${escapeHtml(item.text)}</span>` +
                     `<span class="halcyon-suggestion-type">${escapeHtml(item.type || '')}</span>`;
      li.addEventListener('mouseenter', () => {
        activeSuggestionIdx = i;
        highlightSuggestion();
      });
      li.addEventListener('click', () => {
        openUrl(item.url, 'currentTab');
      });
      suggestionsList.appendChild(li);
    });
    suggestionsList.hidden = false;
    searchInput.setAttribute('aria-expanded', 'true');
    activeSuggestionIdx = -1;
  };

  function hideSuggestions() {
    suggestionsList.hidden = true;
    suggestionsList.innerHTML = '';
    searchInput.setAttribute('aria-expanded', 'false');
    activeSuggestionIdx = -1;
  }

  function highlightSuggestion() {
    for (const li of suggestionsList.children) {
      li.setAttribute('aria-selected', 'false');
    }
    const li = suggestionsList.children[activeSuggestionIdx];
    if (li) {
      li.setAttribute('aria-selected', 'true');
      li.scrollIntoView({block: 'nearest'});
    }
  }

  function openUrl(url, disposition) {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonOpenUrl', [url, disposition]);
    }
  }

  // --- Recently visited --------------------------------------------------

  function requestRecentVisited() {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonRecentVisitedRequest', []);
    } else {
      // Standalone demo: nothing.
      const list = document.getElementById('halcyon-recent-list');
      list.innerHTML = '<li><span class="halcyon-ntp-recent-title">—</span></li>';
    }
  }

  /** @param {Array<{title: string, url: string, faviconUrl: string}>} items */
  window.halcyon.recentVisitedResult = (items) => {
    const list = document.getElementById('halcyon-recent-list');
    if (!items || !items.length) {
      list.innerHTML = '';
      document.getElementById('halcyon-recent').style.display = 'none';
      return;
    }
    list.innerHTML = '';
    items.slice(0, 8).forEach((item) => {
      const li = document.createElement('li');
      li.setAttribute('role', 'listitem');
      li.tabIndex = 0;
      const fav = document.createElement('img');
      fav.className = 'halcyon-ntp-recent-favicon';
      if (item.faviconUrl) {
        fav.src = item.faviconUrl;
        fav.alt = '';
      } else {
        fav.style.visibility = 'hidden';
      }
      const title = document.createElement('span');
      title.className = 'halcyon-ntp-recent-title';
      title.textContent = item.title || item.url;
      const url = document.createElement('span');
      url.className = 'halcyon-ntp-recent-url';
      url.textContent = hostnameOf(item.url);
      li.appendChild(fav);
      li.appendChild(title);
      li.appendChild(url);
      li.addEventListener('click', () => openUrl(item.url, 'currentTab'));
      li.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') openUrl(item.url, 'currentTab');
      });
      list.appendChild(li);
    });
  };

  function hostnameOf(url) {
    try { return new URL(url).hostname; } catch (_) { return ''; }
  }

  // --- Footer links ------------------------------------------------------

  function wireFooter() {
    const cove = document.getElementById('halcyon-open-cove');
    cove.addEventListener('click', (e) => {
      e.preventDefault();
      if (typeof chrome !== 'undefined' && chrome.send) {
        chrome.send('halcyonOpenCove', []);
      }
    });
  }

  // --- Helpers -----------------------------------------------------------

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, (c) => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
    }[c]));
  }

  // --- Init --------------------------------------------------------------

  async function init() {
    document.getElementById('halcyon-greeting').textContent = greetingText();
    document.getElementById('halcyon-date').textContent = longDate();

    const quote = await quoteForToday();
    document.getElementById('halcyon-quote-text').textContent = quote.text;
    document.getElementById('halcyon-quote-attribution').textContent = '— ' + quote.attribution;

    const intentionInput = document.getElementById('halcyon-intention-input');
    intentionInput.value = loadIntention();
    intentionInput.addEventListener('input', () => {
      saveIntention(intentionInput.value);
      if (typeof chrome !== 'undefined' && chrome.send) {
        chrome.send('halcyonIntentionChanged', [intentionKey(), intentionInput.value]);
      }
    });
    cleanOldIntentions();

    wireSearch();
    requestRecentVisited();
    wireFooter();
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
