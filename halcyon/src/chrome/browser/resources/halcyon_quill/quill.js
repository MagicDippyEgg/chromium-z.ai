/**
 * Quill — Halcyon note-taking side panel logic.
 *
 * Responsibilities:
 *   - On load, ask the C++ side for the current origin and the notes for it.
 *   - Render the notes (newest first) as <article class="halcyon-quill-note">.
 *   - Create a new note when the user clicks "New note" or presses ⌘N.
 *   - Autosave each note on input (debounced 400ms).
 *   - Update the "X notes" count in the footer.
 *   - Render plain-language timestamps ("just now", "5 min ago",
 *     "Yesterday", "Nov 3") and refresh them every 60 seconds.
 *
 * Communication with C++:
 *   - chrome.send('halcyonQuillRequestOrigin', [])
 *       C++ replies via window.halcyon.quillOrigin(origin)
 *   - chrome.send('halcyonQuillRequestNotes', [])
 *       C++ replies via window.halcyon.quillNotes(notes)
 *       notes = [{id, body, modifiedEpoch}]
 *   - chrome.send('halcyonQuillSaveNote', [id, body])
 *       C++ replies via window.halcyon.quillNoteSaved(id, modifiedEpoch)
 *   - chrome.send('halcyonQuillDeleteNote', [id])
 *       C++ replies via window.halcyon.quillNoteDeleted(id)
 *   - chrome.send('halcyonQuillNewNote', [])
 *       C++ replies via window.halcyon.quillNoteCreated(id)
 *
 * All chrome.send names are registered in HalcyonQuillMessageHandler.
 *
 * The page is purely a view layer. It does not store anything outside the
 * chrome.send bridge; it does not make any network requests; it does not
 * read cookies or other page data.
 */

(() => {
  'use strict';

  /** @type {string} */ let origin = '';
  /** @type {Array<{id: string, body: string, modifiedEpoch: number}>} */ let notes = [];
  /** @const {number} */ const SAVE_DEBOUNCE_MS = 400;
  /** @const {number} */ const TIME_REFRESH_MS = 60_000;

  // --- DOM ----------------------------------------------------------------

  const originText = document.getElementById('halcyon-quill-origin-text');
  const list = document.getElementById('halcyon-quill-list');
  const empty = document.getElementById('halcyon-quill-empty');
  const newBtn = document.getElementById('halcyon-quill-new');
  const count = document.getElementById('halcyon-quill-count');

  // --- Time formatting ----------------------------------------------------

  /**
   * Plain-language timestamp, matching halcyon/branding/voice.md §Numbers.
   * @param {number} epochMs
   * @return {string}
   */
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
    // Same year — "Nov 3"; different year — "Nov 3, 2024".
    const d = new Date(epochMs);
    const sameYear = d.getFullYear() === new Date(now).getFullYear();
    const month = d.toLocaleString('en-US', {month: 'short'});
    return sameYear ? `${month} ${d.getDate()}` : `${month} ${d.getDate()}, ${d.getFullYear()}`;
  }

  // --- Rendering ----------------------------------------------------------

  /**
   * @param {{id: string, body: string, modifiedEpoch: number}} note
   * @return {HTMLElement}
   */
  function renderNote(note) {
    const article = document.createElement('article');
    article.className = 'halcyon-quill-note';
    article.dataset.noteId = note.id;

    const textarea = document.createElement('textarea');
    textarea.className = 'halcyon-quill-note-textarea';
    textarea.placeholder = 'Write a note for this site…';
    textarea.value = note.body || '';
    textarea.setAttribute('aria-label', `Note, last edited ${plainTime(note.modifiedEpoch)}`);
    textarea.spellcheck = true;
    textarea.rows = 3;

    const footer = document.createElement('footer');
    footer.className = 'halcyon-quill-note-footer';

    const time = document.createElement('span');
    time.className = 'halcyon-quill-note-time';
    time.textContent = plainTime(note.modifiedEpoch);
    time.dataset.epoch = String(note.modifiedEpoch);

    const actions = document.createElement('span');
    actions.className = 'halcyon-quill-note-actions';

    const del = document.createElement('button');
    del.type = 'button';
    del.className = 'halcyon-quill-note-action';
    del.title = 'Delete this note';
    del.setAttribute('aria-label', 'Delete this note');
    del.innerHTML = '<halcyon-icon name="trash" size="14"></halcyon-icon>';
    del.addEventListener('click', () => deleteNote(note.id));

    actions.appendChild(del);
    footer.appendChild(time);
    footer.appendChild(actions);

    article.appendChild(textarea);
    article.appendChild(footer);

    // Debounced autosave.
    let saveTimer = null;
    textarea.addEventListener('input', () => {
      if (saveTimer) clearTimeout(saveTimer);
      saveTimer = setTimeout(() => {
        saveTimer = null;
        chrome.send('halcyonQuillSaveNote', [note.id, textarea.value]);
      }, SAVE_DEBOUNCE_MS);
    });

    // Focus the new (empty) note.
    if (!note.body) {
      requestAnimationFrame(() => textarea.focus());
    }

    return article;
  }

  function renderAll() {
    // Clear existing note cards (keep the empty-state placeholder).
    for (const el of list.querySelectorAll('.halcyon-quill-note')) el.remove();

    const hasNotes = notes.length > 0;
    list.dataset.empty = hasNotes ? 'false' : 'true';
    document.body.dataset.empty = hasNotes ? 'false' : 'true';
    empty.style.display = hasNotes ? 'none' : '';

    for (const note of notes) {
      list.appendChild(renderNote(note));
    }

    updateCount();
  }

  function updateCount() {
    if (!count) return;
    if (notes.length === 0) {
      count.textContent = '';
    } else {
      count.textContent = `${notes.length} note${notes.length === 1 ? '' : 's'}`;
    }
  }

  // --- Mutations ----------------------------------------------------------

  function newNote() {
    chrome.send('halcyonQuillNewNote', []);
  }

  function deleteNote(id) {
    chrome.send('halcyonQuillDeleteNote', [id]);
  }

  // --- C++ callbacks (window.halcyon.*) -----------------------------------

  window.halcyon = window.halcyon || {};

  window.halcyon.quillOrigin = (originStr) => {
    origin = originStr || '—';
    if (originText) originText.textContent = origin;
  };

  window.halcyon.quillNotes = (incoming) => {
    notes = (incoming || []).slice().sort((a, b) => b.modifiedEpoch - a.modifiedEpoch);
    renderAll();
  };

  window.halcyon.quillNoteCreated = (id) => {
    notes.unshift({id, body: '', modifiedEpoch: Date.now()});
    renderAll();
  };

  window.halcyon.quillNoteSaved = (id, modifiedEpoch) => {
    const n = notes.find((x) => x.id === id);
    if (!n) return;
    n.modifiedEpoch = modifiedEpoch;
    // Update just that footer's time, not the whole list.
    const article = list.querySelector(`[data-note-id="${CSS.escape(id)}"]`);
    if (article) {
      const time = article.querySelector('.halcyon-quill-note-time');
      if (time) {
        time.textContent = plainTime(modifiedEpoch);
        time.dataset.epoch = String(modifiedEpoch);
      }
    }
  };

  window.halcyon.quillNoteDeleted = (id) => {
    notes = notes.filter((n) => n.id !== id);
    renderAll();
  };

  // --- Periodic refresh of plain-language times ---------------------------

  setInterval(() => {
    for (const time of list.querySelectorAll('.halcyon-quill-note-time')) {
      const epoch = Number(time.dataset.epoch);
      if (epoch) time.textContent = plainTime(epoch);
    }
  }, TIME_REFRESH_MS);

  // --- Boot ---------------------------------------------------------------

  document.addEventListener('DOMContentLoaded', () => {
    newBtn.addEventListener('click', newNote);

    // ⌘N inside the side panel creates a new note. We do not interfere with
    // ⌘N at the browser level — the binding is owned by the BrowserView
    // accelerator table and routed here only when the side panel has focus.
    document.addEventListener('keydown', (e) => {
      if ((e.metaKey || e.ctrlKey) && !e.shiftKey && !e.altKey && e.key.toLowerCase() === 'n') {
        e.preventDefault();
        newNote();
      }
    });

    chrome.send('halcyonQuillRequestOrigin', []);
    chrome.send('halcyonQuillRequestNotes', []);
  });
})();
