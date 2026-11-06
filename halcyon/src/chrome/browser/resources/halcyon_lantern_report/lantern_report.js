/**
 * Lantern Report — chrome://halcyon-lantern-report
 *
 * The Lantern Report is a weekly, on-demand, local-only summary of what
 * the browser blocked or observed this week. The data is sourced entirely
 * from the user's local profile (the Lantern log, the cookie log, the
 * permissions log). No data is sent to a server.
 *
 * Communication with C++:
 *   chrome.send('halcyonLanternReportRequest', [rangeStartEpoch, rangeEndEpoch])
 *     C++ calls back: window.halcyon.lanternReport(report)
 *
 *   report = {
 *     rangeStart: 1698364800000,
 *     rangeEnd:   1698969600000,
 *     summary:    "This week, Halcyon blocked 47 trackers and 12 cookies.",
 *     breakdown: [
 *       {label, count, weight},
 *       ...
 *     ],
 *     sites: [
 *       {origin, total, items: ["12 trackers", "3 cookies", ...]},
 *       ...
 *     ],
 *   }
 */

(() => {
  'use strict';

  /** @const {number} */ const WEEK_MS = 7 * 24 * 60 * 60 * 1000;

  // --- DOM ----------------------------------------------------------------

  const range = document.getElementById('halcyon-lantern-report-range');
  const recompute = document.getElementById('halcyon-lantern-report-recompute');
  const summary = document.getElementById('halcyon-lantern-report-summary');
  const breakdownList = document.getElementById('halcyon-lantern-report-breakdown-list');
  const sitesList = document.getElementById('halcyon-lantern-report-sites-list');

  // --- Helpers -----------------------------------------------------------

  /**
   * Plain-language date range, e.g. "Oct 27 – Nov 3".
   * @param {number} startEpoch
   * @param {number} endEpoch
   * @return {string}
   */
  function plainRange(startEpoch, endEpoch) {
    const fmt = (e) => {
      const d = new Date(e);
      const m = d.toLocaleString('en-US', {month: 'short'});
      return `${m} ${d.getDate()}`;
    };
    return `${fmt(startEpoch)} – ${fmt(endEpoch)}`;
  }

  // --- Rendering ---------------------------------------------------------

  function renderBreakdown(breakdown) {
    while (breakdownList.firstChild) breakdownList.removeChild(breakdownList.firstChild);
    const max = Math.max(1, ...breakdown.map((b) => b.count));
    for (const row of breakdown) {
      const li = document.createElement('li');
      li.className = 'halcyon-lantern-report-row';

      const label = document.createElement('span');
      label.className = 'halcyon-lantern-report-row-label';
      label.textContent = row.label;

      const bar = document.createElement('span');
      bar.className = 'halcyon-lantern-report-row-bar';
      const fill = document.createElement('span');
      fill.className = 'halcyon-lantern-report-row-bar-fill';
      fill.style.width = `${Math.round((row.count / max) * 100)}%`;
      bar.appendChild(fill);

      const count = document.createElement('span');
      count.className = 'halcyon-lantern-report-row-count';
      count.textContent = String(row.count);

      li.appendChild(label);
      li.appendChild(bar);
      li.appendChild(count);
      breakdownList.appendChild(li);
    }
  }

  function renderSites(sites) {
    while (sitesList.firstChild) sitesList.removeChild(sitesList.firstChild);
    for (const site of sites) {
      const li = document.createElement('li');
      li.className = 'halcyon-lantern-report-site';
      li.setAttribute('aria-expanded', 'false');

      const summaryBtn = document.createElement('button');
      summaryBtn.type = 'button';
      summaryBtn.className = 'halcyon-lantern-report-site-summary';
      summaryBtn.setAttribute('aria-controls', `halcyon-lantern-report-site-detail-${site.origin}`);
      const origin = document.createElement('span');
      origin.className = 'halcyon-lantern-report-site-origin';
      origin.textContent = site.origin;
      const meta = document.createElement('span');
      meta.className = 'halcyon-lantern-report-site-meta';
      meta.textContent = `${site.total} ${site.total === 1 ? 'item' : 'items'}`;
      summaryBtn.appendChild(origin);
      summaryBtn.appendChild(meta);

      const detail = document.createElement('div');
      detail.className = 'halcyon-lantern-report-site-detail';
      detail.id = `halcyon-lantern-report-site-detail-${site.origin}`;
      const ul = document.createElement('ul');
      ul.className = 'halcyon-lantern-report-site-detail-list';
      for (const item of site.items) {
        const il = document.createElement('li');
        il.className = 'halcyon-lantern-report-site-detail-row';
        il.textContent = item;
        ul.appendChild(il);
      }
      detail.appendChild(ul);

      summaryBtn.addEventListener('click', () => {
        const expanded = li.getAttribute('aria-expanded') === 'true';
        li.setAttribute('aria-expanded', String(!expanded));
        summaryBtn.setAttribute('aria-expanded', String(!expanded));
      });

      li.appendChild(summaryBtn);
      li.appendChild(detail);
      sitesList.appendChild(li);
    }
  }

  // --- C++ callback ------------------------------------------------------

  window.halcyon = window.halcyon || {};

  window.halcyon.lanternReport = (report) => {
    if (!report) return;
    if (range) range.textContent = plainRange(report.rangeStart, report.rangeEnd);
    if (summary) summary.textContent = report.summary || '';
    renderBreakdown(report.breakdown || []);
    renderSites(report.sites || []);
  };

  // --- Boot --------------------------------------------------------------

  function request() {
    const end = Date.now();
    const start = end - WEEK_MS;
    chrome.send('halcyonLanternReportRequest', [start, end]);
  }

  document.addEventListener('DOMContentLoaded', () => {
    recompute.addEventListener('click', request);
    request();
  });
})();
