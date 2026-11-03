// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_ntp_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/halcyon_ntp_message_handler.h"
#include "chrome/browser/ui/webui/web_ui_util.h"
#include "chrome/common/chrome_constants.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/common/url_constants.h"

namespace halcyon {

namespace {

constexpr char kHost[] = "newtab";
constexpr char kResourcePath[] = "halcyon_ntp";

// The list of static assets that the Plume page is allowed to load. Each entry
// is the file name as it appears in chrome/browser/resources/halcyon_ntp/.
// The HalcyonNtpMessageHandler uses the same prefix to verify chrome.send
// calls; anything not on this list is rejected by the WebUI URL loader.
constexpr const char* kPlumeAssets[] = {
    "plume.html",
    "plume.css",
    "plume.js",
    "plume.d.ts",
    "quotes.json",
};

}  // namespace

HalcyonNtpUI::HalcyonNtpUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      Profile::FromWebUI(web_ui), kHost);

  // The page's HTML lives at chrome://newtab/plume.html; we expose the page
  // at chrome://newtab/ by setting the default resource.
  source->SetDefaultResource(kResourcePath, IDR_HALCYON_NTP_PLUME_HTML);

  for (const char* asset : kPlumeAssets) {
    // Register each asset under its file name. (Resources are mapped in
    // chrome/browser/resources/halcyon_ntp/BUILD.gn and packed via grit.)
    source->AddResourcePath(asset, IDR_HALCYON_NTP_PLUME_HTML);  // placeholder
  }

  // The shared Halcyon assets (design tokens, icon element, theme controller,
  // icon sprite) live under chrome://resources/halcyon_shared/. Those are
  // registered once for all Halcyon WebUI pages by
  // HalcyonSharedDataSource::Register() in chrome/browser/ui/webui/.
  // We do NOT register them here.

  // The MessageHandler processes chrome.send calls from plume.js. We attach
  // it to this WebUI; it lives for the lifetime of the WebUI.
  web_ui->AddMessageHandler(
      std::make_unique<HalcyonNtpMessageHandler>(Profile::FromWebUI(web_ui)));
}

HalcyonNtpUI::~HalcyonNtpUI() = default;

void HalcyonNtpUI::BindInterfaceReceiver() {
  // No Mojo interfaces for now. The Plume communicates via chrome.send
  // (see HalcyonNtpMessageHandler). When we move to Mojo, the receiver
  // will be wired here.
}

bool HalcyonNtpUI::HandleSubframeNavigation(const std::string& url) {
  // The Plume itself has no iframes; any subframe navigation is suspicious.
  // Refuse it. This matches upstream NewTabUI's behavior.
  return false;
}

void HalcyonNtpUI::Initialize() {
  if (initialized_) return;
  initialized_ = true;
  // Push theme + any other initial state to the page via JavaScript once
  // the renderer is ready. (Done from HalcyonNtpMessageHandler's
  // AllowJavascript / OnJavascriptAllowed callback.)
}

}  // namespace halcyon
