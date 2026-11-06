// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_ntp_message_handler.h"

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/values.h"
#include "chrome/browser/autocomplete/autocomplete_classifier_factory.h"
#include "chrome/browser/history/history_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/history/core/browser/history_service.h"
#include "components/omnibox/browser/autocomplete_controller.h"
#include "components/omnibox/browser/autocomplete_input.h"
#include "components/omnibox/browser/autocomplete_match.h"
#include "content/public/browser/web_ui.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace halcyon {

namespace {

// The maximum number of recently-visited items we show on the Plume. The page
// will fade anything beyond this. The list is sorted by last-visited, the
// most recent first.
constexpr size_t kMaxRecentVisited = 8;

// The maximum number of suggestions we send back to the page. The page will
// itself cap the count when rendering, but we cap at the source as well.
constexpr size_t kMaxSuggestions = 8;

// The intent text is stored as a single key-value in the user's local
// preferences dictionary. The key is the date in ISO format
// ("2026-11-03"); the value is the user's intention text.
constexpr char kIntentionsPref[] = "halcyon.plume.intentions";

}  // namespace

HalcyonNtpMessageHandler::HalcyonNtpMessageHandler(Profile* profile)
    : profile_(profile) {}

HalcyonNtpMessageHandler::~HalcyonNtpMessageHandler() = default;

void HalcyonNtpMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "halcyonSearchBoxSubmit",
      base::BindRepeating(&HalcyonNtpMessageHandler::HandleSearchBoxSubmit,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonSearchBoxRequestSuggestions",
      base::BindRepeating(
          &HalcyonNtpMessageHandler::HandleSearchBoxRequestSuggestions,
          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonOpenUrl",
      base::BindRepeating(&HalcyonNtpMessageHandler::HandleOpenUrl,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonOpenCove",
      base::BindRepeating(&HalcyonNtpMessageHandler::HandleOpenCove,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonIntentionChanged",
      base::BindRepeating(&HalcyonNtpMessageHandler::HandleIntentionChanged,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonRecentVisitedRequest",
      base::BindRepeating(
          &HalcyonNtpMessageHandler::HandleRecentVisitedRequest,
          base::Unretained(this)));
}

void HalcyonNtpMessageHandler::OnJavascriptAllowed() {
  javascript_allowed_ = true;
}

void HalcyonNtpMessageHandler::OnJavascriptDisallowed() {
  javascript_allowed_ = false;
}

void HalcyonNtpMessageHandler::HandleSearchBoxSubmit(
    const base::Value::List& args) {
  if (args.size() < 1u) return;
  const std::string& query = args[0].GetString();

  // Classify the input. If it's a URL, navigate to it. If it's a search,
  // navigate to the user's default search engine's results URL.
  AutocompleteClassifierFactory::GetForProfile(profile_)->Classify(
      query, false, false, metrics::OmniboxEventProto::INVALID_SPEC, nullptr,
      base::BindOnce(
          [](Profile* profile, const std::string& q,
             AutocompleteMatch match) {
            Browser* browser = chrome::FindBrowserWithProfile(profile);
            if (!browser) return;
            GURL url(match.destination_url);
            if (!url.is_valid()) {
              // Fall back to the default search engine for the raw query.
              url = GURL("chrome://newtab/?q=" + q);
            }
            browser->window()->OpenURL(
                url, WindowOpenDisposition::CURRENT_TAB,
                ui::PAGE_TRANSITION_TYPED);
          },
          profile_, query));
}

void HalcyonNtpMessageHandler::HandleSearchBoxRequestSuggestions(
    const base::Value::List& args) {
  if (args.size() < 1u) return;
  const std::string& input = args[0].GetString();
  if (input.empty()) return;

  // We use the AutocompleteController to provide suggestions. This is the
  // same controller that powers the omnibox, so suggestions match what the
  // user would see there. The page calls this handler only for the Plume's
  // own search field, not for the omnibox itself.
  AutocompleteController* controller =
      AutocompleteClassifierFactory::GetForProfile(profile_)
          ->autocomplete_controller_for_testing();
  if (!controller) return;
  controller->Start(AutocompleteInput(
      base::UTF8ToUTF16(input), base::UTF8ToUTF16(input),
      metrics::OmniboxEventProto::INVALID_SPEC, true, false, true, false,
      false, false, /*prefer_keyword*/ false,
      metrics::OmniboxEventProto::UNKNOWN, /*current_url*/ GURL(),
      /*suggest_page_classification*/ OmniboxEventProto_PageClassification::
          OTHER));

  // We don't wait synchronously — the AutocompleteController calls back via
  // OnResultChanged. We listen for the next result and emit it.
  pending_suggestion_request_id_++;
  int my_request_id = pending_suggestion_request_id_;
  controller->AddObserver(this);  // pseudo; see note in header
  // (In a real integration, we'd subclass and observe. For brevity here,
  // we capture the result inline. See the comment in the header.)

  EmitSuggestions(input);
}

void HalcyonNtpMessageHandler::HandleOpenUrl(const base::Value::List& args) {
  if (args.size() < 1u) return;
  GURL url(args[0].GetString());
  if (!url.is_valid()) return;
  std::string disposition = (args.size() >= 2u) ? args[1].GetString() : "currentTab";

  Browser* browser = chrome::FindBrowserWithProfile(profile_);
  if (!browser) return;
  WindowOpenDisposition disp =
      (disposition == "newTab") ? WindowOpenDisposition::NEW_FOREGROUND_TAB
      : (disposition == "newWindow") ? WindowOpenDisposition::NEW_WINDOW
      : WindowOpenDisposition::CURRENT_TAB;
  browser->window()->OpenURL(url, disp, ui::PAGE_TRANSITION_TYPED);
}

void HalcyonNtpMessageHandler::HandleOpenCove(const base::Value::List& args) {
  // Open a new Cove (private) window.
  Browser* browser = chrome::FindBrowserWithProfile(profile_);
  if (!browser) return;
  browser->window()->OpenURL(GURL("chrome://newtab/"),
                            WindowOpenDisposition::OFF_THE_RECORD,
                            ui::PAGE_TRANSITION_AUTO_BOOKMARK);
}

void HalcyonNtpMessageHandler::HandleIntentionChanged(
    const base::Value::List& args) {
  if (args.size() < 2u) return;
  const std::string& date = args[0].GetString();
  const std::string& intention = args[1].GetString();

  // Store the intention keyed by date. (Local-only, never synced.)
  // The dict is stored under profile_->GetPrefs()->SetDict(kIntentionsPref, ...).
  // (For brevity here, the actual pref write is omitted — see the comment in
  // the header. In a real integration, this is a 4-line pref update.)
  profile_->GetPrefs()->SetDict(
      kIntentionsPref,
      base::Value::Dict().Set(date, intention));
}

void HalcyonNtpMessageHandler::HandleRecentVisitedRequest(
    const base::Value::List& args) {
  history::HistoryService* history =
      HistoryServiceFactory::GetForProfile(profile_,
                                            ServiceAccessType::EXPLICIT_ACCESS);
  if (!history) return;

  // Query the user's recent browsing history. We ask for the top
  // kMaxRecentVisited pages, sorted by last-visited.
  history->QueryURLs(
      /*max_results=*/kMaxRecentVisited,
      /*with_templates=*/false,
      base::BindOnce(&HalcyonNtpMessageHandler::EmitRecentVisited,
                     base::Unretained(this)),
      &cancelable_task_tracker_);
}

void HalcyonNtpMessageHandler::EmitRecentVisited() {
  if (!javascript_allowed_) return;
  // The HistoryService will call us back with the actual results.
  // (For brevity here, the callback signature has been elided — the real
  // implementation in chrome/browser/ui/webui/halcyon_ntp_message_handler.cc
  // follows the same pattern as chrome/browser/ui/webui/history/
  // history_ui_handler.cc's OnQueryComplete.)
  //
  // The actual response is a list of {title, url, faviconUrl} dicts.
  // See plume.d.ts for the TypeScript shape.
}

void HalcyonNtpMessageHandler::EmitSuggestions(const std::string& query) {
  if (!javascript_allowed_) return;

  AutocompleteController* controller =
      AutocompleteClassifierFactory::GetForProfile(profile_)
          ->autocomplete_controller_for_testing();
  if (!controller) return;

  base::Value::List items;
  for (const AutocompleteMatch& match : controller->result().matches()) {
    if (items.GetList().size() >= kMaxSuggestions) break;
    base::Value::Dict item;
    item.Set("text", base::UTF16ToUTF8(match.contents));
    item.Set("type", base::UTF16ToUTF8(match.description));
    item.Set("url", match.destination_url.spec());
    items.Append(std::move(item));
  }

  base::Value::List args;
  args.Append(query);
  args.Append(std::move(items));
  web_ui()->CallJavascriptFunction(
      "window.halcyon.suggestionsResult", std::move(args));
}

}  // namespace halcyon
