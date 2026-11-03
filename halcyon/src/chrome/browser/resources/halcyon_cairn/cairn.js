/**
 * Cairn — Halcyon bookmarks page logic.
 *
 * The C++ side calls window.halcyon.cairnData({cairns: [{id, name, bookmarks: [{title, url, faviconUrl}]}]})
 * once when the page is ready.
 *
 * User actions:
 *   chrome.send('halcyonCairnNew', [name])              — create a new cairn
 *   chrome.send('halcyonCairnRename', [id, name])       — rename a cairn
 *   chrome.send('halcyonCairnDelete', [id])             — delete a cairn
 *   chrome.send('halcyonCairnOpenBookmark', [url])      — open a bookmark
 *   chrome.send('halcyonCairnMoveBookmark', [bmId, fromId, toId])  — drag between
 *   chrome.send('halcyonCairnRemoveBookmark', [bmId])   — remove from cairn
 */

(() => {
  'use strict';

  /** @type {Array<{id: string, name: string, bookmarks: Array<{id, title, url, faviconUrl}>}>} */
  let cairns = [];

  window.halcyon = window.halcyon || {};
  window.halcyon.cairnData = (data) => {
    cairns = data.cairns || [];
    render();
  };

  function init() {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonCairnRequest', []);
    }
    document.getElementById('halcyon-cairn-new').addEventListener('click', () => {
      const name = prompt('Name this cairn', 'Reading');
      if (name && name.trim()) {
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonCairnNew', [name.trim()]);
        }
      }
    });
  }

  function render() {
    const main = document.getElementById('halcyon-cairn-main');
    main.innerHTML = '';
    if (!cairns.length) {
      main.innerHTML = '<p class="halcyon-cairn-empty">No cairns yet. Create one to start a stack.</p>';
      return;
    }
    for (const cairn of cairns) {
      main.appendChild(renderCairn(cairn));
    }
  }

  function renderCairn(cairn) {
    const card = document.createElement('article');
    card.className = 'halcyon-cairn-card';
    if (!cairn.bookmarks || !cairn.bookmarks.length) {
      card.classList.add('halcyon-cairn-empty-card');
    }

    const header = document.createElement('div');
    header.className = 'halcyon-cairn-card-header';
    const icon = document.createElement('halcyon-icon');
    icon.setAttribute('name', 'cairn');
    icon.setAttribute('size', '18');
    icon.className = 'halcyon-cairn-card-icon';
    const title = document.createElement('h3');
    title.className = 'halcyon-cairn-card-title';
    title.textContent = cairn.name;
    const count = document.createElement('span');
    count.className = 'halcyon-cairn-card-count';
    count.textContent = (cairn.bookmarks?.length || 0) + (cairn.bookmarks?.length === 1 ? ' item' : ' items');
    const actions = document.createElement('div');
    actions.className = 'halcyon-cairn-card-actions';
    const rename = document.createElement('button');
    rename.className = 'halcyon-cairn-card-action';
    rename.innerHTML = '<halcyon-icon name="settings" size="14"></halcyon-icon>';
    rename.title = 'Rename';
    rename.addEventListener('click', () => {
      const name = prompt('Rename cairn', cairn.name);
      if (name && name.trim() && name !== cairn.name) {
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonCairnRename', [cairn.id, name.trim()]);
        }
      }
    });
    const del = document.createElement('button');
    del.className = 'halcyon-cairn-card-action danger';
    del.innerHTML = '<halcyon-icon name="x" size="14"></halcyon-icon>';
    del.title = 'Delete cairn (items move to Unstacked)';
    del.addEventListener('click', () => {
      if (confirm(`Delete the "${cairn.name}" cairn? Items inside move to Unstacked.`)) {
        if (typeof chrome !== 'undefined' && chrome.send) {
          chrome.send('halcyonCairnDelete', [cairn.id]);
        }
      }
    });
    actions.appendChild(rename);
    actions.appendChild(del);
    header.appendChild(icon);
    header.appendChild(title);
    header.appendChild(count);
    header.appendChild(actions);

    const list = document.createElement('ol');
    list.className = 'halcyon-cairn-card-list';
    if (cairn.bookmarks?.length) {
      for (const bm of cairn.bookmarks) {
        list.appendChild(renderItem(bm, cairn.id));
      }
    }

    card.appendChild(header);
    card.appendChild(list);
    return card;
  }

  function renderItem(bm, cairnId) {
    const li = document.createElement('li');
    li.className = 'halcyon-cairn-item';
    li.setAttribute('role', 'link');
    li.tabIndex = 0;
    const fav = document.createElement('img');
    fav.className = 'halcyon-cairn-item-favicon';
    if (bm.faviconUrl) fav.src = bm.faviconUrl;
    const title = document.createElement('span');
    title.className = 'halcyon-cairn-item-title';
    title.textContent = bm.title || bm.url;
    li.appendChild(fav);
    li.appendChild(title);
    li.addEventListener('click', () => openBookmark(bm.url));
    li.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') openBookmark(bm.url);
    });
    return li;
  }

  function openBookmark(url) {
    if (typeof chrome !== 'undefined' && chrome.send) {
      chrome.send('halcyonCairnOpenBookmark', [url]);
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
