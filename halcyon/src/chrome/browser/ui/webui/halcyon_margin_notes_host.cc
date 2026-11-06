// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_margin_notes_host.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/base64.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/hash/sha256.h"
#include "base/json/json_file_value_serializer.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/uuid.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_contents.h"
#include "third_party/absl/strings/str_cat.h"

namespace halcyon {

WEB_CONTENTS_USERDATA_KEY_IMPL(HalcyonMarginNotesHost);

namespace {

constexpr char kMarginNotesDir[] = "MarginNotes";
constexpr size_t kUrlHashLen = 16;  // 8 bytes → 16 hex chars.

base::FilePath MarginRootForProfile(Profile* profile) {
  return profile->GetPath().AppendASCII(kMarginNotesDir);
}

std::string HashUrl(const GURL& url) {
  if (!url.is_valid()) return std::string();
  // Strip the ref — notes don't depend on #anchor.
  GURL::Replacements strip;
  strip.ClearRef();
  GURL canonical = url.GetOrigin().ReplaceComponents(strip);
  // Hash the origin (scheme + host + port) AND the path. Same page, same
  // notes — even if the query changes. (Notes on a /search?q=a URL are the
  // same notes as /search?q=b, by intent.)
  std::string key = canonical.GetOrigin().spec() + url.path();
  std::string hash = base::SHA256HashString(key);
  std::string out;
  for (size_t i = 0; i < hash.size() && out.size() < kUrlHashLen; ++i) {
    absl::StrAppend(&out, base::HexEncode(&hash[i], 1));
  }
  return out;
}

base::FilePath::StringType JsonFileName(const std::string& id) {
  return base::FilePath::StringType(FILE_PATH_LITERAL(".")) +
         base::FilePath::StringType(FILE_PATH_LITERAL("/")) +
         base::UTF8ToUTF16(id) + FILE_PATH_LITERAL(".json");
}

base::Value::Dict NoteToValue(const NoteJson& n) {
  base::Value::Dict d;
  d.Set("id", n.id);
  d.Set("body", n.body);
  d.Set("quotedText", n.quoted_text);
  d.Set("modifiedEpoch",
        static_cast<double>(n.modified.InMillisecondsFSinceUnixEpoch()));
  return d;
}

std::optional<NoteJson> NoteFromValue(const base::Value::Dict& d) {
  NoteJson n;
  const std::string* id = d.FindString("id");
  const std::string* body = d.FindString("body");
  const std::string* quoted = d.FindString("quotedText");
  if (!id) return std::nullopt;
  n.id = *id;
  n.body = body ? *body : std::string();
  n.quoted_text = quoted ? *quoted : std::string();
  if (auto e = d.FindDouble("modifiedEpoch")) {
    n.modified = base::Time::FromMillisecondsSinceUnixEpoch(static_cast<int64_t>(*e));
  }
  return n;
}

void WriteNoteToDisk(const base::FilePath& dir, const NoteJson& n) {
  base::CreateDirectory(dir);
  base::Value::Dict d = NoteToValue(n);
  std::string json;
  if (!base::JSONWriter::Write(d, &json)) return;
  base::WriteFile(dir.AppendASCII(n.id).AddExtension(FILE_PATH_LITERAL(".json")),
                  json);
}

void DeleteNoteFromDisk(const base::FilePath& dir, const std::string& id) {
  base::DeleteFile(
      dir.AppendASCII(id).AddExtension(FILE_PATH_LITERAL(".json")));
}

std::vector<NoteJson> LoadNotesFromDisk(const base::FilePath& dir) {
  std::vector<NoteJson> out;
  if (!base::DirectoryExists(dir)) return out;
  base::FileEnumerator it(dir, /*recursive=*/false,
                          base::FileEnumerator::FILES,
                          FILE_PATH_LITERAL("*.json"));
  for (base::FilePath name = it.Next(); !name.empty(); name = it.Next()) {
    std::string contents;
    if (!base::ReadFileToString(name, &contents)) continue;
    auto parsed = base::JSONReader::Read(contents);
    if (!parsed || !parsed->is_dict()) continue;
    auto n = NoteFromValue(parsed->GetDict());
    if (n) {
      base::File::Info info;
      if (base::GetFileInfo(name, &info)) n->modified = info.last_modified;
      out.push_back(*n);
    }
  }
  return out;
}

}  // namespace

HalcyonMarginNotesHost::HalcyonMarginNotesHost(content::WebContents* wc)
    : content::WebContentsObserver(wc),
      web_contents_(wc),
      file_task_runner_(base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskPriority::BEST_EFFORT,
           base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN})) {}

HalcyonMarginNotesHost::~HalcyonMarginNotesHost() = default;

// static
bool HalcyonMarginNotesHost::HasNotesForUrl(content::WebContents* wc,
                                             const GURL& url) {
  if (!wc) return false;
  auto* host = HalcyonMarginNotesHost::FromWebContents(wc);
  if (host && url == host->current_url_) {
    return !host->notes_cache_.empty();
  }
  // Fall back to a quick directory check on the file thread. (The omnibox
  // code calls this synchronously; for the common case where the user is
  // on the URL right now, the in-memory cache answers fast.)
  if (!wc) return false;
  Profile* profile = Profile::FromBrowserContext(wc->GetBrowserContext());
  if (!profile) return false;
  std::string hash = HashUrl(url);
  if (hash.empty()) return false;
  base::FilePath dir = MarginRootForProfile(profile).AppendASCII(hash);
  if (!base::DirectoryExists(dir)) return false;
  base::FileEnumerator it(dir, /*recursive=*/false,
                          base::FileEnumerator::FILES,
                          FILE_PATH_LITERAL("*.json"));
  return !it.Next().empty();
}

void HalcyonMarginNotesHost::DidFinishNavigation(content::NavigationHandle* h) {
  if (!h->HasCommitted() || h->IsSameDocument()) return;
  if (!h->IsInMainFrame()) return;
  current_url_ = h->GetURL();
  current_url_hash_ = HashUrl(current_url_);
  notes_cache_.clear();
  LoadNotesForCurrentUrl();
}

void HalcyonMarginNotesHost::WebContentsDestroyed() {
  receiver_.reset();
}

void HalcyonMarginNotesHost::AddNoteFromSelection(
    const std::string& quoted_text) {
  // Thin wrapper around CreateNote, called by the page context menu
  // (when the user picks "Add a margin note" with text selected).
  // The menu-item wiring itself is deferred — see
  // patches/context_menu_add_margin_note.patch — but this method is
  // the entry point the future context-menu code will call.
  CreateNote(quoted_text);
}

void HalcyonMarginNotesHost::BindReceiver(
    mojo::PendingAssociatedReceiver<halcyon::mojom::HalcyonMarginNotesHost>
        r) {
  receiver_.Bind(std::move(r));
}

std::vector<NoteJson> HalcyonMarginNotesHost::GetNotesForCurrentUrl() {
  std::vector<NoteJson> out;
  out.reserve(notes_cache_.size());
  for (auto& [_, n] : notes_cache_) out.push_back(n);
  return out;
}

void HalcyonMarginNotesHost::CreateNote(const std::string& quoted_text) {
  if (current_url_hash_.empty()) return;
  NoteJson n;
  n.id = base::Uuid::GenerateRandomV4().AsLowerString();
  n.body = std::string();
  n.quoted_text = quoted_text;
  n.modified = base::Time::Now();

  notes_cache_[n.id] = n;
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&WriteNoteToDisk,
                     MarginRootForProfile(Profile::FromBrowserContext(
                         web_contents_->GetBrowserContext()))
                         .AppendASCII(current_url_hash_),
                     n));
}

void HalcyonMarginNotesHost::SaveNote(const std::string& id, const std::string& body) {
  if (current_url_hash_.empty()) return;
  auto it = notes_cache_.find(id);
  if (it == notes_cache_.end()) return;
  it->second.body = body;
  it->second.modified = base::Time::Now();

  NoteJson snapshot = it->second;
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&WriteNoteToDisk,
                     MarginRootForProfile(Profile::FromBrowserContext(
                         web_contents_->GetBrowserContext()))
                         .AppendASCII(current_url_hash_),
                     snapshot));
}

void HalcyonMarginNotesHost::DeleteNote(const std::string& id) {
  if (current_url_hash_.empty()) return;
  notes_cache_.erase(id);
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&DeleteNoteFromDisk,
                     MarginRootForProfile(Profile::FromBrowserContext(
                         web_contents_->GetBrowserContext()))
                         .AppendASCII(current_url_hash_),
                     id));
}

void HalcyonMarginNotesHost::LoadNotesForCurrentUrl() {
  if (current_url_hash_.empty()) return;
  Profile* profile = Profile::FromBrowserContext(web_contents_->GetBrowserContext());
  if (!profile) return;
  base::FilePath dir = MarginRootForProfile(profile).AppendASCII(current_url_hash_);

  file_task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&LoadNotesFromDisk, dir),
      base::BindOnce(
          [](base::WeakPtr<HalcyonMarginNotesHost> self,
             std::vector<NoteJson> notes) {
            if (!self) return;
            self->notes_cache_.clear();
            for (auto& n : notes) self->notes_cache_[n.id] = std::move(n);
          },
          weak_factory_.GetWeakPtr()));
}

}  // namespace halcyon
