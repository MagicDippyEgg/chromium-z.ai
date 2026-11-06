// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_LANTERN_REPORT_UI_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_LANTERN_REPORT_UI_H_

#include "content/public/browser/web_ui_controller.h"

namespace content {
class WebUI;
}

namespace halcyon {

// HalcyonLanternReportUI is the WebUI controller for
// chrome://halcyon-lantern-report — the weekly, on-demand, local-only
// summary of what the Lantern observed or blocked this week.
//
// The report aggregates:
//   - the Lantern log (per-page third-party requests, fingerprinting
//     attempts, persistent storage, permission prompts)
//   - the cookie log (third-party cookies blocked, allowed, deleted)
//   - the autoplay log (sites that tried to autoplay, muted count)
//   - the permission log (geolocation, notifications, mic, camera prompts)
//
// All data comes from local SQLite stores under
//   <profile>/Halcyon/Lantern/*.sqlite
// No data is sent anywhere; the page is generated locally.
//
// See halcyon/branding/brand_book.md §13.12.

class HalcyonLanternReportUI : public content::WebUIController {
 public:
  explicit HalcyonLanternReportUI(content::WebUI* web_ui);
  ~HalcyonLanternReportUI() override;

  void BindInterfaceReceiver() override;

 private:
  bool initialized_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_LANTERN_REPORT_UI_H_
