// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_MARGIN_NOTES_HOST_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_MARGIN_NOTES_HOST_H_

#include <map>
#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "base/time/time.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "url/gurl.h"

namespace content {
class WebContents;
}

namespace halcyon::mojom {
class HalcyonMarginNotesHost;
}  // namespace halcyon::mojom

namespace halcyon {

// A single Margin Note on disk. Stored as JSON under
//   <profile>/MarginNotes/<url-hash>/<note-id>.json
struct NoteJson {
  std::string id;            // UUID, also the filename stem.
  std::string body;          // Plain markdown, the user's note text.
  std::string quoted_text;   // The page text the note refers to. May be empty.
  base::Time modified;       // Last modified (file mtime is the source of truth).
};

// HalcyonMarginNotesHost is the per-WebContents host for Margin Notes.
//
// Margin Notes are per-URL annotations that appear as a right-margin overlay
// on the page. They are stored on disk under
//   <profile_dir>/MarginNotes/<url-sha256[:16]>/<note-uuid>.json
// Each note is a small JSON file with {id, body, quotedText, modifiedEpoch}.
//
// Unlike Quill (which is a chrome:// side panel), Margin Notes is an overlay
// injected into the page by a content script. The content script talks to
// this host via the HalcyonMarginNotesHost Mojo interface (see
// halcyon/public/mojom/margin_notes.mojom).
//
// The host is created on demand when the WebContents commits an http(s)
// URL — see HalcyonMarginNotesTabHelper::DidFinishNavigation in the
// integration patch. The host lives for the lifetime of the WebContents.
//
// The notes are kept in memory while the WebContents is alive, and lazily
// loaded from disk on first access. Saves are debounced (300ms) and
// serialized to a background SequencedTaskRunner.
//
// Per-URL note surface:
//   - When the user navigates to a URL that has notes, a small "·N" mark
//     is shown in the omnibox (the HalcyonOmniboxView consults
//     HalcyonMarginNotesHost::HasNotesForUrl()).
//   - The overlay shows the notes for the current URL only.

class HalcyonMarginNotesHost : public content::WebContentsUserData<HalcyonMarginNotesHost>,
                                public content::WebContentsObserver {
 public:
  ~HalcyonMarginNotesHost() override;

  // Returns true if there are any notes for the given URL on disk. Used by
  // the omnibox to decide whether to show the "·N" mark.
  static bool HasNotesForUrl(content::WebContents* wc, const GURL& url);

  // WebContentsObserver:
  void DidFinishNavigation(content::NavigationHandle* handle) override;
  void WebContentsDestroyed() override;

  // Bind the Mojo receiver exposed to the content script's isolated world.
  void BindReceiver(mojo::PendingAssociatedReceiver<halcyon::mojom::HalcyonMarginNotesHost>
                       receiver);

  // Browser-side handlers — called by the Mojo impl. Each returns a list
  // (or modifies the in-memory cache and persists).
  std::vector<NoteJson> GetNotesForCurrentUrl();
  void CreateNote(const std::string& quoted_text);
  void SaveNote(const std::string& note_id, const std::string& body);
  void DeleteNote(const std::string& note_id);

 private:
  friend class content::WebContentsUserData<HalcyonMarginNotesHost>;
  WEB_CONTENTS_USERDATA_KEY_DECL();

  explicit HalcyonMarginNotesHost(content::WebContents* wc);

  // Hashes a URL to a stable 16-character hex directory name.
  static std::string HashUrl(const GURL& url);

  // Loads notes for the current URL from disk on a background runner.
  void LoadNotesForCurrentUrl();

  raw_ptr<content::WebContents> web_contents_;

  // Notes currently in memory for the loaded URL. Cleared on navigation.
  std::map<std::string /* id */, NoteJson> notes_cache_;
  GURL current_url_;
  std::string current_url_hash_;

  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;
  mojo::AssociatedReceiver<halcyon::mojom::HalcyonMarginNotesHost> receiver_{this};

  // Must be the last member.
  base::WeakPtrFactory<HalcyonMarginNotesHost> weak_factory_{this};
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_MARGIN_NOTES_HOST_H_
