// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_QUILL_MESSAGE_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_QUILL_MESSAGE_HANDLER_H_

#include <map>
#include <string>

#include "base/files/file_path.h"
#include "base/time/time.h"
#include "content/public/browser/web_ui_message_handler.h"

class Profile;

namespace base {
class SequencedTaskRunner;
class Value;
}  // namespace base

namespace halcyon {

// HalcyonQuillMessageHandler is the chrome.send bridge for quill.js.
//
// quill.js calls:
//   chrome.send('halcyonQuillRequestOrigin', [])
//   chrome.send('halcyonQuillRequestNotes', [])
//   chrome.send('halcyonQuillNewNote', [])
//   chrome.send('halcyonQuillSaveNote', [id, body])
//   chrome.send('halcyonQuillDeleteNote', [id])
//
// The handler calls back via:
//   window.halcyon.quillOrigin(origin)
//   window.halcyon.quillNotes(notes)
//   window.halcyon.quillNoteCreated(id)
//   window.halcyon.quillNoteSaved(id, modifiedEpoch)
//   window.halcyon.quillNoteDeleted(id)
//
// Storage layout (per-profile, per-origin):
//   <profile_dir>/Quill/<origin-sha256[:8]>/<note-uuid>.md
// Each note file is plain markdown. The filename is the UUID; the file
// mtime is the modified-time. There is no index file — the directory
// listing is the index.
//
// The handler does file IO on a background SequencedTaskRunner; the JS
// callbacks are posted back to the UI thread.

class HalcyonQuillMessageHandler : public content::WebUIMessageHandler {
 public:
  explicit HalcyonQuillMessageHandler(Profile* profile);
  ~HalcyonQuillMessageHandler() override;

  // content::WebUIMessageHandler:
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

 private:
  // chrome.send handlers.
  void HandleRequestOrigin(const base::Value::List& args);
  void HandleRequestNotes(const base::Value::List& args);
  void HandleNewNote(const base::Value::List& args);
  void HandleSaveNote(const base::Value::List& args);
  void HandleDeleteNote(const base::Value::List& args);

  // Background-file-IO helpers (run on file_task_runner_).
  void LoadNotesOnFileThread(const std::string& origin_dir_hash,
                             const std::string& callback_name);
  void WriteNoteOnFileThread(const std::string& origin_dir_hash,
                             const std::string& note_id,
                             const std::string& body);
  void DeleteNoteOnFileThread(const std::string& origin_dir_hash,
                              const std::string& note_id);

  // Emits a list of notes back to the page. Marshalled onto the UI thread.
  void EmitNotes(const std::string& origin,
                 const std::vector<struct NoteOnDisk>& notes);

  // Hashes an origin string (e.g. "https://example.com:443") to a stable
  // 8-character directory name. Uses SHA-256 truncated to the first 8 hex
  // chars — enough entropy to avoid collisions across a single user's
  // visited origins, while staying filename-safe.
  static std::string HashOrigin(const std::string& origin);

  raw_ptr<Profile> profile_;
  bool javascript_allowed_ = false;

  // The origin of the WebContents that owns this side panel. Empty if the
  // opener is a chrome:// page.
  std::string current_origin_;
  std::string current_origin_hash_;

  // File IO runner. Notes are small but we still avoid doing IO on the UI
  // thread. May be null in tests.
  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_QUILL_MESSAGE_HANDLER_H_
