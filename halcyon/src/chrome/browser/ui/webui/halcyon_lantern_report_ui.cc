// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_lantern_report_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/halcyon_lantern_report_message_handler.h"
#include "content/public/browser/web_ui_data_source.h"

namespace halcyon {

namespace {

constexpr char kHost[] = "halcyon-lantern-report";
constexpr char kResourcePath[] = "halcyon_lantern_report";

constexpr const char* kAssets[] = {
    "lantern_report.html",
    "lantern_report.css",
    "lantern_report.js",
};

}  // namespace

HalcyonLanternReportUI::HalcyonLanternReportUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      Profile::FromWebUI(web_ui), kHost);

  source->SetDefaultResource(kResourcePath, IDR_HALCYON_LANTERN_REPORT_HTML);
  for (const char* asset : kAssets) {
    source->AddResourcePath(asset, IDR_HALCYON_LANTERN_REPORT_HTML);  // placeholder
  }

  web_ui->AddMessageHandler(
      std::make_unique<HalcyonLanternReportMessageHandler>(Profile::FromWebUI(web_ui)));
}

HalcyonLanternReportUI::~HalcyonLanternReportUI() = default;

void HalcyonLanternReportUI::BindInterfaceReceiver() {}

}  // namespace halcyon
