/**
 * The Lantern — popup logic.
 *
 * The C++ side (HalcyonLanternMessageHandler) calls
 * window.halcyon.lanternData({domain, rows}) once when the popup is ready.
 * Each row has: text, severity ('info'|'caution'|'danger'), and an
 * optional action ({label, command}).
 */

(() => {
  'use strict';

  window.halcyon = window.halcyon || {};

  /** @param {{domain: string, rows: Array<{text: string, severity?: string, action?: {label: string, command: string}}>}} data */
  window.halcyon.lanternData = (data) => {
    const domainEl = document.getElementById('halcyon-lantern-domain');
    const rowsEl = document.getElementById('halcyon-lantern-rows');
    const emptyEl = document.getElementById('halcyon-lantern-empty');
    if (data.domain) domainEl.textContent = data.domain;

    rowsEl.innerHTML = '';
    if (!data.rows || data.rows.length === 0) {
      emptyEl.hidden = false;
      return;
    }
    emptyEl.hidden = true;
    for (const row of data.rows) {
      const li = document.createElement('li');
      li.className = 'halcyon-lantern-row';
      const dot = document.createElement('span');
      dot.className = 'halcyon-lantern-row-dot';
      if (row.severity === 'caution') dot.style.background = 'var(--halcyon-caution)';
      if (row.severity === 'danger') dot.style.background = 'var(--halcyon-danger)';
      const text = document.createElement('span');
      text.className = 'halcyon-lantern-row-text';
      text.textContent = row.text;
      li.appendChild(dot);
      li.appendChild(text);
      if (row.action) {
        const btn = document.createElement('button');
        btn.className = 'halcyon-lantern-row-action';
        btn.textContent = row.action.label;
        btn.addEventListener('click', () => {
          if (typeof chrome !== 'undefined' && chrome.send) {
            chrome.send('halcyonLanternAction', [row.action.command]);
          }
        });
        li.appendChild(btn);
      }
      rowsEl.appendChild(li);
    }
  };

  function init() {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonLanternRequest', []);
    } else {
      // Standalone demo:
      window.halcyon.lanternData({
        domain: 'example.com',
        rows: [
          {text: 'Third-party cookies being set', severity: 'info', action: {label: 'Block', command: 'block_3pc'}},
          {text: 'Local storage used: 3.4 MiB', severity: 'info'},
          {text: 'Notification permission requested', severity: 'caution', action: {label: 'Block', command: 'block_notify'}},
        ],
      });
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
