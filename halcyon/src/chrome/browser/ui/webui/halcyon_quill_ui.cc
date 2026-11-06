// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_quill_ui.h"

#include <string>

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/halcyon_quill_message_handler.h"
#include "chrome/browser/ui/webui/web_ui_util.h"
#include "content/public/browser/web_ui_data_source.h"

namespace halcyon {

namespace {

constexpr char kHost[] = "halcyon-quill";
constexpr char kResourcePath[] = "halcyon_quill";

constexpr const char* kQuillAssets[] = {
    "quill.html",
    "quill.css",
    "quill.js",
};

}  // namespace

HalcyonQuillUI::HalcyonQuillUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      Profile::FromWebUI(web_ui), kHost);

  source->SetDefaultResource(kResourcePath, IDR_HALCYON_QUILL_HTML);

  for (const char* asset : kQuillAssets) {
    source->AddResourcePath(asset, IDR_HALCYON_QUILL_HTML);  // placeholder
  }

  web_ui->AddMessageHandler(
      std::make_unique<HalcyonQuillMessageHandler>(Profile::FromWebUI(web_ui)));
}

HalcyonQuillUI::~HalcyonQuillUI() = default;

void HalcyonQuillUI::BindInterfaceReceiver() {
  // No Mojo interfaces. Quill communicates entirely via chrome.send.
}

std::string HalcyonQuillUI::GetOriginForActiveWebContents() const {
  // Read from the Browser's active WebContents. The C++ side owns this; the
  // page never reports its own origin (otherwise a compromised page could
  // lie about which origin's notes to show).
  //
  // Implemented in HalcyonQuillMessageHandler::OnJavascriptAllowed, where
  // the active WebContents is reachable via the WebUI's owning browser.
  return std::string();
}

void HalcyonQuillUI::Initialize() {
  if (initialized_) return;
  initialized_ = true;
}

}  // namespace halcyon
