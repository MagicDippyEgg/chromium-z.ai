// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_about_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/halcyon_about_message_handler.h"
#include "chrome/common/chrome_version.h"
#include "content/public/browser/web_ui_data_source.h"

namespace halcyon {

namespace {

constexpr char kHost[] = "halcyon";
constexpr char kResourcePath[] = "halcyon_about";

constexpr const char* kAssets[] = {
    "about.html",
    "about.css",
    "about.js",
};

}  // namespace

HalcyonAboutUI::HalcyonAboutUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      Profile::FromWebUI(web_ui), kHost);

  source->SetDefaultResource(kResourcePath, IDR_HALCYON_ABOUT_HTML);
  for (const char* asset : kAssets) {
    source->AddResourcePath(asset, IDR_HALCYON_ABOUT_HTML);  // placeholder
  }

  web_ui->AddMessageHandler(
      std::make_unique<HalcyonAboutMessageHandler>(Profile::FromWebUI(web_ui)));
}

HalcyonAboutUI::~HalcyonAboutUI() = default;

void HalcyonAboutUI::BindInterfaceReceiver() {}

}  // namespace halcyon
