/**
 * Tides — Halcyon history page logic.
 *
 * The page asks the C++ side for the user's browsing history grouped by day,
 * with per-site time-spent totals. The response shape is:
 *
 *   window.halcyon.tidesData([
 *     {
 *       date: '2026-11-03',
 *       sites: [
 *         {
 *           url: 'https://example.com',
 *           title: 'Example',
 *           faviconUrl: 'chrome://favicon/...,
 *           visitCount: 7,
 *           timeSpentSeconds: 1200,
 *           lastVisit: '2026-11-03T11:32:00Z',
 *         }, ...
 *       ]
 *     }, ...
 *   ]);
 *
 * The user can filter, wash (delete) individual sites, wash whole days, or
 * wash all. All wash actions are sent via chrome.send('halcyonTidesWash', ...).
 */

(() => {
  'use strict';

  /** @type {Array<{date: string, sites: Array<Object>}>} */
  let data = [];
  let filterText = '';

  function init() {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonTidesRequest', []);
    }

    document.getElementById('halcyon-tides-filter').addEventListener('input', (e) => {
      filterText = e.target.value.toLowerCase().trim();
      render();
    });

    document.getElementById('halcyon-tides-wash-all').addEventListener('click', () => {
      if (confirm('Wash all Tides? This can\'t be undone.')) {
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonTidesWashAll', []);
        }
        data = [];
        render();
      }
    });
  }

  /** @param {Array} newData */
  window.halcyon = window.halcyon || {};
  window.halcyon.tidesData = (newData) => {
    data = newData || [];
    render();
  };

  function render() {
    const main = document.getElementById('halcyon-tides-main');
    main.innerHTML = '';
    if (!data.length) {
      const p = document.getElementById('halcyon-tides-empty');
      p.hidden = false;
      return;
    }
    document.getElementById('halcyon-tides-empty').hidden = true;

    let anyVisible = false;
    for (const day of data) {
      const daySection = renderDay(day);
      if (daySection) {
        main.appendChild(daySection);
        anyVisible = true;
      }
    }
    if (!anyVisible) {
      const p = document.createElement('p');
      p.className = 'halcyon-tides-empty';
      p.textContent = 'Nothing matches the filter.';
      main.appendChild(p);
    }
  }

  /** @param {Object} day @return {HTMLElement} */
  function renderDay(day) {
    const section = document.createElement('section');
    section.className = 'halcyon-tides-day';

    const header = document.createElement('div');
    header.className = 'halcyon-tides-day-header';
    const title = document.createElement('h2');
    title.className = 'halcyon-tides-day-title';
    title.textContent = humanDay(day.date);
    const meta = document.createElement('span');
    meta.className = 'halcyon-tides-day-meta';
    meta.textContent = humanDayMeta(day);
    const wash = document.createElement('button');
    wash.className = 'halcyon-tides-day-wash';
    wash.textContent = 'Wash day';
    wash.addEventListener('click', () => {
      if (confirm(`Wash ${day.date}?`)) {
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonTidesWashDay', [day.date]);
        }
        section.remove();
      }
    });
    header.appendChild(title);
    header.appendChild(meta);
    header.appendChild(wash);
    section.appendChild(header);

    // Compute max time-spent for the bar widths.
    const maxTime = day.sites.reduce((m, s) => Math.max(m, s.timeSpentSeconds || 0), 1);

    let anySiteVisible = false;
    for (const site of day.sites) {
      if (!matchesFilter(site)) continue;
      anySiteVisible = true;
      section.appendChild(renderBar(site, maxTime));
    }
    return anySiteVisible ? section : null;
  }

  /** @param {Object} site @param {number} maxTime @return {HTMLElement} */
  function renderBar(site, maxTime) {
    const bar = document.createElement('div');
    bar.className = 'halcyon-tides-bar';
    bar.setAttribute('role', 'link');
    bar.tabIndex = 0;

    const fav = document.createElement('img');
    fav.className = 'halcyon-tides-favicon';
    fav.src = site.faviconUrl || '';
    fav.alt = '';

    const content = document.createElement('div');
    content.className = 'halcyon-tides-bar-content';
    const title = document.createElement('span');
    title.className = 'halcyon-tides-bar-title';
    title.textContent = site.title || site.url;
    const url = document.createElement('span');
    url.className = 'halcyon-tides-bar-url';
    url.textContent = hostnameOf(site.url);
    content.appendChild(title);
    content.appendChild(url);

    const flow = document.createElement('div');
    flow.className = 'halcyon-tides-bar-flow';
    const fill = document.createElement('div');
    fill.className = 'halcyon-tides-bar-flow-fill';
    const pct = Math.min(100, Math.max(2, ((site.timeSpentSeconds || 0) / maxTime) * 100));
    fill.style.width = pct + '%';
    flow.appendChild(fill);

    const meta = document.createElement('span');
    meta.className = 'halcyon-tides-bar-meta';
    meta.textContent = `${humanTime(site.timeSpentSeconds)} · ${site.visitCount || 1}×`;

    const washBtn = document.createElement('button');
    washBtn.className = 'halcyon-tides-bar-wash';
    washBtn.title = 'Wash away this site';
    washBtn.setAttribute('aria-label', 'Wash away this site from Tides');
    washBtn.innerHTML = '<halcyon-icon name="x" size="14"></halcyon-icon>';
    washBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      if (confirm(`Wash ${site.url}?\nRemoves every visit to this site from Tides.`)) {
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonTidesWashSite', [site.url]);
        }
        bar.remove();
      }
    });

    bar.appendChild(fav);
    bar.appendChild(content);
    bar.appendChild(flow);
    bar.appendChild(meta);
    bar.appendChild(washBtn);

    bar.addEventListener('click', () => openUrl(site.url));
    bar.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') openUrl(site.url);
    });
    return bar;
  }

  function openUrl(url) {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonTidesOpenUrl', [url]);
    }
  }

  // --- helpers ---------------------------------------------------------

  function matchesFilter(site) {
    if (!filterText) return true;
    return (
      (site.title || '').toLowerCase().includes(filterText) ||
      (site.url || '').toLowerCase().includes(filterText)
    );
  }

  function humanDay(dateStr) {
    // Show "Today", "Yesterday", weekday for the past week, then long date.
    const d = new Date(dateStr + 'T00:00:00');
    const today = new Date();
    today.setHours(0, 0, 0, 0);
    const diff = Math.floor((today - d) / 86400000);
    if (diff === 0) return 'Today';
    if (diff === 1) return 'Yesterday';
    if (diff < 7) return new Intl.DateTimeFormat(navigator.language || 'en', {weekday: 'long'}).format(d);
    return new Intl.DateTimeFormat(navigator.language || 'en', {month: 'long', day: 'numeric', year: 'numeric'}).format(d);
  }

  function humanDayMeta(day) {
    const total = day.sites.reduce((m, s) => m + (s.timeSpentSeconds || 0), 0);
    const visits = day.sites.reduce((m, s) => m + (s.visitCount || 1), 0);
    return `${humanTime(total)} across ${day.sites.length} sites, ${visits} visits`;
  }

  function humanTime(s) {
    if (!s) return '0s';
    if (s < 60) return Math.round(s) + 's';
    if (s < 3600) return Math.round(s / 60) + 'm';
    return (s / 3600).toFixed(1) + 'h';
  }

  function hostnameOf(url) {
    try { return new URL(url).hostname; } catch (_) { return ''; }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
