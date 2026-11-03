// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_LANTERN_REPORT_MESSAGE_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_LANTERN_REPORT_MESSAGE_HANDLER_H_

#include "content/public/browser/web_ui_message_handler.h"

class Profile;

namespace halcyon {

// HalcyonLanternReportMessageHandler is the chrome.send bridge for
// lantern_report.js.
//
//   chrome.send('halcyonLanternReportRequest', [rangeStartEpoch, rangeEndEpoch])
//   C++ calls back: window.halcyon.lanternReport(report)
//
// The report is a base::Value::Dict with:
//   rangeStart: epoch_ms
//   rangeEnd:   epoch_ms
//   summary:    one-sentence plain-language summary
//   breakdown:  [{label, count, weight}, ...]
//   sites:      [{origin, total, items: [string, ...]}, ...]
//
// The aggregation is done by querying the Lantern SQLite logs for the
// given time range. The query runs on a background runner; the result is
// posted back to the UI thread and emitted via CallJavascriptFunction.

class HalcyonLanternReportMessageHandler : public content::WebUIMessageHandler {
 public:
  explicit HalcyonLanternReportMessageHandler(Profile* profile);
  ~HalcyonLanternReportMessageHandler() override;

  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

 private:
  void HandleRequest(const base::Value::List& args);

  // Builds the report on the database runner, then posts the result back
  // to the UI thread. (Declared virtual so tests can override and inject
  // synthetic data.)
  virtual void BuildReport(int64_t range_start, int64_t range_end);

  raw_ptr<Profile> profile_;
  bool javascript_allowed_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_LANTERN_REPORT_MESSAGE_HANDLER_H_
