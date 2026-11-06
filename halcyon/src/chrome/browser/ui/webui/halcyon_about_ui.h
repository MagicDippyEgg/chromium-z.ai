// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_ABOUT_UI_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_ABOUT_UI_H_

#include "content/public/browser/web_ui_controller.h"

namespace content {
class WebUI;
}

namespace halcyon {

// HalcyonAboutUI is the WebUI controller for chrome://halcyon — the About
// page. See halcyon/branding/voice.md §About Halcyon for the design intent.
//
// The page is intentionally a single page of plain text. No logos of
// upstream vendors, no animated splash, no "Made with ♥".

class HalcyonAboutUI : public content::WebUIController {
 public:
  explicit HalcyonAboutUI(content::WebUI* web_ui);
  ~HalcyonAboutUI() override;

  void BindInterfaceReceiver() override;

 private:
  bool initialized_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_ABOUT_UI_H_
