/**
 * Margin Notes — overlay logic.
 *
 * This file is injected into every http(s) page (see margin_notes_bootstrap.js
 * and the integration patch). It runs in an isolated world, so it sees the
 * page DOM but cannot be touched by the page's JS. It builds a Shadow DOM
 * overlay that hosts the rail and the notes column.
 *
 * The notes are per-URL (not per-origin like Quill). The URL is read from
 * the C++ side, never from the page (otherwise a compromised page could
 * spoof the URL and read another page's notes).
 *
 * Communication with C++ (via the chrome.send injected into the isolated
 * world by the integration patch):
 *   chrome.send('halcyonMarginRequestUrl', [])
 *   chrome.send('halcyonMarginRequestNotes', [])
 *   chrome.send('halcyonMarginNewNote', [quotedText])
 *   chrome.send('halcyonMarginSaveNote', [id, body])
 *   chrome.send('halcyonMarginDeleteNote', [id])
 *
 *   C++ calls back:
 *     window.halcyon.marginUrl(url)
 *     window.halcyon.marginNotes(notes)
 *     window.halcyon.marginNoteCreated(id, quotedText)
 *     window.halcyon.marginNoteSaved(id, modifiedEpoch)
 *     window.halcyon.marginNoteDeleted(id)
 *
 * Right-click anywhere on the page → "Add a margin note" creates a new note
 * with the current text selection as the quote. The "Add" button in the
 * column header creates a note with an empty quote.
 */

(() => {
  'use strict';

  // --- Bootstrap ---------------------------------------------------------
  //
  // The C++ side injects a `<link rel="stylesheet">` for margin_notes.css and
  // a `<script>` for this file as the LAST children of <html>, in an isolated
  // world. We attach the overlay to the documentElement as a Shadow DOM.

  const host = document.createElement('div');
  host.id = 'halcyon-margin-host';
  host.style.all = 'initial';
  host.style.position = 'fixed';
  host.style.inset = '0 auto 0 auto';
  host.style.width = '0';
  host.style.zIndex = '2147483646';
  host.style.pointerEvents = 'none';
  document.documentElement.appendChild(host);

  const shadow = host.attachShadow({mode: 'open', delegatesFocus: true});

  // Fetch the bundled HTML via fetch() — the bootstrap script defines a
  // global __HALCYON_MARGIN_HTML__ with the contents of margin_notes.html.
  // (Avoids an extra network round-trip.)
  shadow.innerHTML = window.__HALCYON_MARGIN_HTML__ || '';

  /** @type {string} */ let url = '';
  /** @type {Array<{id, body, quotedText, modifiedEpoch}>} */ let notes = [];
  /** @const {number} */ const SAVE_DEBOUNCE_MS = 400;
  /** @const {number} */ const TIME_REFRESH_MS = 60_000;

  // --- DOM (inside the shadow root) ---------------------------------------

  const rail = shadow.getElementById('halcyon-margin-rail');
  const tab = shadow.getElementById('halcyon-margin-rail-tab');
  const column = shadow.getElementById('halcyon-margin-column');
  const addBtn = shadow.getElementById('halcyon-margin-add');
  const notesList = shadow.getElementById('halcyon-margin-notes');
  const countEl = shadow.getElementById('halcyon-margin-rail-count');

  // --- Time formatting (mirrors quill.js, kept inline to stay self-contained)

  function plainTime(epochMs) {
    const now = Date.now();
    const diff = Math.max(0, now - epochMs);
    const min = 60_000, hour = 60 * min, day = 24 * hour;
    if (diff < min) return 'just now';
    if (diff < hour) {
      const m = Math.round(diff / min);
      return `${m} min${m === 1 ? '' : 's'} ago`;
    }
    if (diff < day) {
      const h = Math.round(diff / hour);
      return `${h} hour${h === 1 ? '' : 's'} ago`;
    }
    if (diff < 2 * day) return 'Yesterday';
    if (diff < 7 * day) {
      const d = Math.round(diff / day);
      return `${d} days ago`;
    }
    const d = new Date(epochMs);
    const sameYear = d.getFullYear() === new Date(now).getFullYear();
    const month = d.toLocaleString('en-US', {month: 'short'});
    return sameYear ? `${month} ${d.getDate()}` : `${month} ${d.getDate()}, ${d.getFullYear()}`;
  }

  // --- Rendering ---------------------------------------------------------

  function setRailVisibility() {
    host.style.display = notes.length > 0 ? '' : 'none';
    if (countEl) {
      countEl.textContent = notes.length > 0 ? `${notes.length}` : '';
    }
  }

  /**
   * @param {{id, body, quotedText, modifiedEpoch}} note
   * @return {Node}
   */
  function renderNote(note) {
    const li = document.createElement('li');
    li.className = 'halcyon-margin-note';
    li.dataset.noteId = note.id;

    if (note.quotedText) {
      const quote = document.createElement('blockquote');
      quote.className = 'halcyon-margin-note-quote';
      quote.textContent = note.quotedText;
      li.appendChild(quote);
    }

    const textarea = document.createElement('textarea');
    textarea.className = 'halcyon-margin-note-textarea';
    textarea.placeholder = 'Write a note about this passage…';
    textarea.value = note.body || '';
    textarea.spellcheck = true;
    li.appendChild(textarea);

    const footer = document.createElement('footer');
    footer.className = 'halcyon-margin-note-footer';

    const time = document.createElement('span');
    time.className = 'halcyon-margin-note-time';
    time.textContent = plainTime(note.modifiedEpoch);
    time.dataset.epoch = String(note.modifiedEpoch);

    const del = document.createElement('button');
    del.type = 'button';
    del.className = 'halcyon-margin-note-delete';
    del.title = 'Delete this note';
    del.textContent = 'Delete';
    del.addEventListener('click', () => {
      chrome.send('halcyonMarginDeleteNote', [note.id]);
    });

    footer.appendChild(time);
    footer.appendChild(del);
    li.appendChild(footer);

    // Debounced autosave.
    let saveTimer = null;
    textarea.addEventListener('input', () => {
      if (saveTimer) clearTimeout(saveTimer);
      saveTimer = setTimeout(() => {
        saveTimer = null;
        chrome.send('halcyonMarginSaveNote', [note.id, textarea.value]);
      }, SAVE_DEBOUNCE_MS);
    });

    if (!note.body) requestAnimationFrame(() => textarea.focus());

    return li;
  }

  function renderAll() {
    while (notesList.firstChild) notesList.removeChild(notesList.firstChild);
    for (const note of notes) {
      notesList.appendChild(renderNote(note));
    }
    setRailVisibility();
  }

  // --- Mutations ---------------------------------------------------------

  function newNote(quotedText) {
    chrome.send('halcyonMarginNewNote', [quotedText || '']);
  }

  // --- C++ callbacks -----------------------------------------------------

  window.halcyon = window.halcyon || {};

  window.halcyon.marginUrl = (u) => { url = u; };

  window.halcyon.marginNotes = (incoming) => {
    notes = (incoming || []).slice().sort((a, b) => b.modifiedEpoch - a.modifiedEpoch);
    renderAll();
  };

  window.halcyon.marginNoteCreated = (id, quotedText) => {
    notes.unshift({id, body: '', quotedText: quotedText || '', modifiedEpoch: Date.now()});
    renderAll();
    // Auto-expand the column so the user can type right away.
    if (rail) rail.dataset.state = 'expanded';
  };

  window.halcyon.marginNoteSaved = (id, modifiedEpoch) => {
    const n = notes.find((x) => x.id === id);
    if (!n) return;
    n.modifiedEpoch = modifiedEpoch;
    const li = notesList.querySelector(`[data-note-id="${CSS.escape(id)}"]`);
    if (li) {
      const time = li.querySelector('.halcyon-margin-note-time');
      if (time) {
        time.textContent = plainTime(modifiedEpoch);
        time.dataset.epoch = String(modifiedEpoch);
      }
    }
  };

  window.halcyon.marginNoteDeleted = (id) => {
    notes = notes.filter((n) => n.id !== id);
    renderAll();
  };

  // --- UI wiring ---------------------------------------------------------

  // Rail click → expand the column.
  tab.addEventListener('click', () => {
    const expanded = rail.dataset.state === 'expanded';
    rail.dataset.state = expanded ? 'collapsed' : 'expanded';
    tab.setAttribute('aria-expanded', String(!expanded));
  });

  // Click outside the column (but not on the rail) → collapse.
  document.addEventListener('click', (e) => {
    if (!rail) return;
    if (rail.dataset.state !== 'expanded') return;
    const path = e.composedPath();
    if (path.includes(column)) return;
    if (path.includes(rail)) return;
    rail.dataset.state = 'collapsed';
    tab.setAttribute('aria-expanded', 'false');
  });

  // "Add" button.
  addBtn.addEventListener('click', () => {
    const quotedText = String(window.getSelection() || '').trim();
    newNote(quotedText);
  });

  // --- Periodic refresh of plain-language times --------------------------

  setInterval(() => {
    for (const time of notesList.querySelectorAll('.halcyon-margin-note-time')) {
      const epoch = Number(time.dataset.epoch);
      if (epoch) time.textContent = plainTime(epoch);
    }
  }, TIME_REFRESH_MS);

  // --- Boot --------------------------------------------------------------

  // The chrome.send bridge is injected by the integration patch (a custom
  // content script). It is on `chrome.send` in the isolated world.
  function boot() {
    if (typeof chrome === 'undefined' || !chrome.send) {
      // Bridge not ready; retry shortly.
      setTimeout(boot, 100);
      return;
    }
    chrome.send('halcyonMarginRequestUrl', []);
    chrome.send('halcyonMarginRequestNotes', []);
  }

  boot();
})();
