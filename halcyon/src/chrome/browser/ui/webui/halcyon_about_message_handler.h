// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_ABOUT_MESSAGE_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_ABOUT_MESSAGE_HANDLER_H_

#include "content/public/browser/web_ui_message_handler.h"

class Profile;

namespace halcyon {

// HalcyonAboutMessageHandler — chrome.send bridge for about.js.
//
//   chrome.send('halcyonAboutRequestInfo', [])
//     C++ replies: window.halcyon.aboutInfo({version, build, channel, arch})
//   chrome.send('halcyonAboutReportProblem', [])
//     C++ opens chrome://settings?topic=report in a new tab.
//   chrome.send('halcyonAboutReleaseNotes', [])
//     C++ opens chrome://settings?topic=release-notes in a new tab.

class HalcyonAboutMessageHandler : public content::WebUIMessageHandler {
 public:
  explicit HalcyonAboutMessageHandler(Profile* profile);
  ~HalcyonAboutMessageHandler() override;

  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

 private:
  void HandleRequestInfo(const base::Value::List& args);
  void HandleReportProblem(const base::Value::List& args);
  void HandleReleaseNotes(const base::Value::List& args);

  raw_ptr<Profile> profile_;
  bool javascript_allowed_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_ABOUT_MESSAGE_HANDLER_H_
