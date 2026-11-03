// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_NTP_UI_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_NTP_UI_H_

#include "content/public/browser/web_ui_controller.h"

// HalcyonNtpUI is the WebUI controller for chrome://newtab in Halcyon.
//
// In upstream Chromium, chrome://newtab is served by NewTabUI (see
// chrome/browser/ui/webui/new_tab/new_tab_ui.*). Halcyon replaces that
// controller with this one, which serves the Plume — the Halcyon NTP —
// from chrome://resources/halcyon_ntp/.
//
// The replacement is wired up in chrome/browser/ui/webui/webui_configurations.cc
// (HalcyonConfigurations::RegisterURLLoader) — see
// patches/webui_register_halcyon_ntp.patch for the integration point.
//
// The Plume is a calm, mostly-empty page:
//   - a greeting + date
//   - a daily quote about attention or stillness
//   - a single search field
//   - a "today's intention" line (locally stored, never synced)
//   - a softly-faded recently-visited list
//
// It does NOT show:
//   - a shortcuts grid / most-visited tiles
//   - a "Discover" feed
//   - a promotional banner
//   - a sign-in nudge
//
// See halcyon/branding/brand_book.md §13.1 for the design intent.

namespace content {
class WebUI;
}

namespace halcyon {

class HalcyonNtpUI : public content::WebUIController {
 public:
  explicit HalcyonNtpUI(content::WebUI* web_ui);
  ~HalcyonNtpUI() override;

  // content::WebUIController:
  void BindInterfaceReceiver() override;
  bool HandleSubframeNavigation(const std::string& url) override;

 private:
  void Initialize();

  bool initialized_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_NTP_UI_H_
