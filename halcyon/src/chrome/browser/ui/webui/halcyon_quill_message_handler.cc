// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_quill_message_handler.h"

#include <memory>
#include <string>
#include <vector>

#include "base/base64.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/hash/sha256.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "base/uuid.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_ui.h"
#include "third_party/absl/strings/str_cat.h"

namespace halcyon {

namespace {

// On-disk note representation used between the file thread and the UI thread.
struct NoteOnDisk {
  std::string id;            // UUID, also the filename stem.
  std::string body;          // Plain markdown.
  base::Time modified;       // File mtime.
};

constexpr char kQuillDirName[] = "Quill";

base::FilePath QuillRootForProfile(Profile* profile) {
  return profile->GetPath().AppendASCII(kQuillDirName);
}

}  // namespace

HalcyonQuillMessageHandler::HalcyonQuillMessageHandler(Profile* profile)
    : profile_(profile),
      file_task_runner_(base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskPriority::BEST_EFFORT,
           base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN})) {}

HalcyonQuillMessageHandler::~HalcyonQuillMessageHandler() = default;

void HalcyonQuillMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "halcyonQuillRequestOrigin",
      base::BindRepeating(&HalcyonQuillMessageHandler::HandleRequestOrigin,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonQuillRequestNotes",
      base::BindRepeating(&HalcyonQuillMessageHandler::HandleRequestNotes,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonQuillNewNote",
      base::BindRepeating(&HalcyonQuillMessageHandler::HandleNewNote,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonQuillSaveNote",
      base::BindRepeating(&HalcyonQuillMessageHandler::HandleSaveNote,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonQuillDeleteNote",
      base::BindRepeating(&HalcyonQuillMessageHandler::HandleDeleteNote,
                          base::Unretained(this)));
}

void HalcyonQuillMessageHandler::OnJavascriptAllowed() {
  javascript_allowed_ = true;
  // The origin is read from the owning WebContents's last-committed URL,
  // not from the page itself — otherwise a compromised page could spoof the
  // origin label and read another origin's notes.
  //
  // In the integration patch the BrowserView sets the side-panel's
  // opener-origin property before the WebUI is created; we read it here.
  // For now, the placeholder "" means "no origin" — the page shows the
  // empty state. (The integration test in HalcyonQuillBrowserTest exercises
  // the non-empty case.)
  current_origin_ = std::string();
  current_origin_hash_ = HashOrigin(current_origin_);
}

void HalcyonQuillMessageHandler::OnJavascriptDisallowed() {
  javascript_allowed_ = false;
}

// static
std::string HalcyonQuillMessageHandler::HashOrigin(const std::string& origin) {
  if (origin.empty()) return std::string();
  std::string hash = base::SHA256HashString(origin);
  // First 4 bytes → 8 hex chars.
  std::string out;
  for (size_t i = 0; i < 4 && i < hash.size(); ++i) {
    absl::StrAppend(&out, base::HexEncode(&hash[i], 1));
  }
  return out;
}

void HalcyonQuillMessageHandler::HandleRequestOrigin(
    const base::Value::List& args) {
  if (!javascript_allowed_) return;
  web_ui()->CallJavascriptFunction("window.halcyon.quillOrigin",
                                    base::Value(current_origin_.empty()
                                                    ? std::string("—")
                                                    : current_origin_));
}

void HalcyonQuillMessageHandler::HandleRequestNotes(
    const base::Value::List& args) {
  if (!javascript_allowed_ || current_origin_hash_.empty()) {
    // No origin → no notes. Emit an empty list.
    if (javascript_allowed_) {
      web_ui()->CallJavascriptFunction("window.halcyon.quillNotes",
                                        base::Value(base::Value::List()));
    }
    return;
  }
  // Run the directory enumeration on the file thread.
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&HalcyonQuillMessageHandler::LoadNotesOnFileThread,
                     base::Unretained(this), current_origin_hash_,
                     std::string("window.halcyon.quillNotes")));
}

void HalcyonQuillMessageHandler::HandleNewNote(const base::Value::List& args) {
  if (!javascript_allowed_ || current_origin_hash_.empty()) return;
  const std::string id = base::Uuid::GenerateRandomV4().AsLowerString();
  // Create an empty file so the next reload sees it.
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&HalcyonQuillMessageHandler::WriteNoteOnFileThread,
                     base::Unretained(this), current_origin_hash_, id,
                     std::string()));
  web_ui()->CallJavascriptFunction("window.halcyon.quillNoteCreated",
                                    base::Value(id));
}

void HalcyonQuillMessageHandler::HandleSaveNote(const base::Value::List& args) {
  if (!javascript_allowed_ || current_origin_hash_.empty()) return;
  if (args.size() < 2) return;
  const std::string* id = args[0].GetIfString();
  const std::string* body = args[1].GetIfString();
  if (!id || !body) return;
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&HalcyonQuillMessageHandler::WriteNoteOnFileThread,
                     base::Unretained(this), current_origin_hash_, *id, *body));
}

void HalcyonQuillMessageHandler::HandleDeleteNote(
    const base::Value::List& args) {
  if (!javascript_allowed_ || current_origin_hash_.empty()) return;
  if (args.empty()) return;
  const std::string* id = args[0].GetIfString();
  if (!id) return;
  file_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&HalcyonQuillMessageHandler::DeleteNoteOnFileThread,
                     base::Unretained(this), current_origin_hash_, *id));
  web_ui()->CallJavascriptFunction("window.halcyon.quillNoteDeleted",
                                    base::Value(*id));
}

// --- File-thread helpers --------------------------------------------------

void HalcyonQuillMessageHandler::LoadNotesOnFileThread(
    const std::string& origin_dir_hash,
    const std::string& callback_name) {
  const base::FilePath root = QuillRootForProfile(profile_);
  const base::FilePath dir = root.AppendASCII(origin_dir_hash);

  std::vector<NoteOnDisk> notes;
  if (base::DirectoryExists(dir)) {
    base::FileEnumerator it(dir, /*recursive=*/false,
                            base::FileEnumerator::FILES, FILE_PATH_LITERAL("*.md"));
    for (base::FilePath name = it.Next(); !name.empty(); name = it.Next()) {
      NoteOnDisk n;
      n.id = name.BaseName().RemoveExtension().AsUTF8Unsafe();
      std::string body;
      if (!base::ReadFileToString(name, &body)) continue;
      n.body = std::move(body);
      base::File::Info info;
      if (base::GetFileInfo(name, &info)) n.modified = info.last_modified;
      notes.push_back(std::move(n));
    }
  }

  // Build a base::Value::List of {id, body, modifiedEpoch} and post back to
  // the UI thread.
  base::Value::List list;
  for (const auto& n : notes) {
    base::Value::Dict d;
    d.Set("id", n.id);
    d.Set("body", n.body);
    d.Set("modifiedEpoch",
          static_cast<double>(n.modified.InMillisecondsFSinceUnixEpoch()));
    list.Append(base::Value(std::move(d)));
  }

  // Post the callback to the UI thread.
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](content::WebUI* web_ui, std::string cb, base::Value::List notes) {
            web_ui->CallJavascriptFunction(cb, base::Value(std::move(notes)));
          },
          web_ui(), callback_name, std::move(list)));
}

void HalcyonQuillMessageHandler::WriteNoteOnFileThread(
    const std::string& origin_dir_hash,
    const std::string& note_id,
    const std::string& body) {
  const base::FilePath root = QuillRootForProfile(profile_);
  const base::FilePath dir = root.AppendASCII(origin_dir_hash);
  base::CreateDirectory(dir);
  const base::FilePath file =
      dir.AppendASCII(note_id).AddExtension(FILE_PATH_LITERAL(".md"));
  base::WriteFile(file, body);
  // File mtime is now the modified time. Don't emit back; the page can
  // request a fresh list if it wants, or read the mtime on next save.
}

void HalcyonQuillMessageHandler::DeleteNoteOnFileThread(
    const std::string& origin_dir_hash,
    const std::string& note_id) {
  const base::FilePath root = QuillRootForProfile(profile_);
  const base::FilePath file = root.AppendASCII(origin_dir_hash)
                                  .AppendASCII(note_id)
                                  .AddExtension(FILE_PATH_LITERAL(".md"));
  base::DeleteFile(file);
}

}  // namespace halcyon
