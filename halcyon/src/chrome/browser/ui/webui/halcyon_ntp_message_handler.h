// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_NTP_MESSAGE_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_NTP_MESSAGE_HANDLER_H_

#include <string>
#include <vector>

#include "content/public/browser/web_ui_message_handler.h"

class Profile;

namespace history {
class HistoryService;
}

namespace halcyon {

// HalcyonNtpMessageHandler is the chrome.send bridge for plume.js.
//
// plume.js calls:
//   chrome.send('halcyonSearchBoxSubmit', [query])
//   chrome.send('halcyonSearchBoxRequestSuggestions', [input])
//   chrome.send('halcyonOpenUrl', [url, disposition])
//   chrome.send('halcyonOpenCove', [])
//   chrome.send('halcyonIntentionChanged', [date, intention])
//   chrome.send('halcyonRecentVisitedRequest', [])
//
// The handler calls back via:
//   window.halcyon.recentVisitedResult(items)
//   window.halcyon.suggestionsResult(query, items)
//
// All chrome.send names start with the `halcyon` prefix and are explicitly
// registered in RegisterMessages(). Unknown names are dropped (matching
// upstream WebUI default behavior).
//
// The Plume deliberately does not have access to:
//   - the user's account / sync state
//   - extensions
//   - any external fetches
//   - any telemetry or stats
//
// If you need to add a message, add it here AND in plume.d.ts so the
// TypeScript bridge stays in sync. Run
//   halcyon/design/check_webui_bridge.py
// to verify.

class HalcyonNtpMessageHandler : public content::WebUIMessageHandler {
 public:
  explicit HalcyonNtpMessageHandler(Profile* profile);
  ~HalcyonNtpMessageHandler() override;

  // content::WebUIMessageHandler:
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

 private:
  // chrome.send handlers.
  void HandleSearchBoxSubmit(const base::Value::List& args);
  void HandleSearchBoxRequestSuggestions(const base::Value::List& args);
  void HandleOpenUrl(const base::Value::List& args);
  void HandleOpenCove(const base::Value::List& args);
  void HandleIntentionChanged(const base::Value::List& args);
  void HandleRecentVisitedRequest(const base::Value::List& args);

  // Pushes the recently-visited list back to the page. Called from
  // HandleRecentVisitedRequest after the HistoryService responds.
  void EmitRecentVisited();

  // Pushes autocomplete suggestions back to the page. Called from
  // HandleSearchBoxRequestSuggestions after AutocompleteController responds.
  void EmitSuggestions(const std::string& query);

  raw_ptr<Profile> profile_;
  bool javascript_allowed_ = false;

  // Pending autocomplete session, if any. Only one at a time.
  int pending_suggestion_request_id_ = 0;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_NTP_MESSAGE_HANDLER_H_
