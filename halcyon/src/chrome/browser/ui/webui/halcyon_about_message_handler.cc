// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_about_message_handler.h"

#include <string>

#include "base/system/sys_info.h"
#include "base/version.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/common/chrome_version.h"
#include "chrome/common/chrome_channels.h"
#include "content/public/browser/web_ui.h"
#include "url/gurl.h"

namespace halcyon {

namespace {

constexpr char kReleaseNotesUrl[] = "chrome://settings?topic=release-notes";
constexpr char kReportProblemUrl[] = "chrome://settings?topic=report";

std::string ArchString() {
#if defined(ARCH_CPU_X86_64)
  return "64-bit";
#elif defined(ARCH_CPU_ARM64)
  return "ARM64";
#elif defined(ARCH_CPU_X86)
  return "32-bit";
#elif defined(ARCH_CPU_ARMEL)
  return "ARM";
#else
  return "unknown";
#endif
}

std::string ChannelString() {
  switch (chrome::GetChannel()) {
    case version_info::Channel::STABLE:    return "stable";
    case version_info::Channel::BETA:      return "beta";
    case version_info::Channel::DEV:       return "dev";
    case version_info::Channel::CANARY:    return "canary";
    default:                               return "stable";
  }
}

}  // namespace

HalcyonAboutMessageHandler::HalcyonAboutMessageHandler(Profile* profile)
    : profile_(profile) {}

HalcyonAboutMessageHandler::~HalcyonAboutMessageHandler() = default;

void HalcyonAboutMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "halcyonAboutRequestInfo",
      base::BindRepeating(&HalcyonAboutMessageHandler::HandleRequestInfo,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonAboutReportProblem",
      base::BindRepeating(&HalcyonAboutMessageHandler::HandleReportProblem,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "halcyonAboutReleaseNotes",
      base::BindRepeating(&HalcyonAboutMessageHandler::HandleReleaseNotes,
                          base::Unretained(this)));
}

void HalcyonAboutMessageHandler::OnJavascriptAllowed() {
  javascript_allowed_ = true;
}

void HalcyonAboutMessageHandler::OnJavascriptDisallowed() {
  javascript_allowed_ = false;
}

void HalcyonAboutMessageHandler::HandleRequestInfo(const base::Value::List& args) {
  if (!javascript_allowed_) return;
  base::Value::Dict info;
  info.Set("version", version_info::GetVersionNumber());
  info.Set("build", version_info::GetLastChange());
  info.Set("channel", ChannelString());
  info.Set("arch", ArchString());
  web_ui()->CallJavascriptFunction("window.halcyon.aboutInfo",
                                    base::Value(std::move(info)));
}

void HalcyonAboutMessageHandler::HandleReportProblem(const base::Value::List& args) {
  // Open the report-problem settings page in a new foreground tab.
  NavigateParams params(chrome::FindBrowserWithProfile(profile_),
                        GURL(kReportProblemUrl),
                        ui::PAGE_TRANSITION_LINK);
  params.disposition = WindowOpenDisposition::NEW_FOREGROUND_TAB;
  Navigate(&params);
}

void HalcyonAboutMessageHandler::HandleReleaseNotes(const base::Value::List& args) {
  NavigateParams params(chrome::FindBrowserWithProfile(profile_),
                        GURL(kReleaseNotesUrl),
                        ui::PAGE_TRANSITION_LINK);
  params.disposition = WindowOpenDisposition::NEW_FOREGROUND_TAB;
  Navigate(&params);
}

}  // namespace halcyon
