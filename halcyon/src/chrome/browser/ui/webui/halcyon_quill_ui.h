// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HALCYON_QUILL_UI_H_
#define CHROME_BROWSER_UI_WEBUI_HALCYON_QUILL_UI_H_

#include <string>

#include "content/public/browser/web_ui_controller.h"
#include "url/origin.h"

namespace content {
class WebUI;
}

namespace halcyon {

// HalcyonQuillUI is the WebUI controller for chrome://halcyon-quill.
//
// Quill is the Halcyon note-taking side panel. It opens on any page via
// ⌘⇧N (Ctrl+Shift+N on Windows/Linux). Each origin has its own stack of
// notes; notes are stored locally in the user's profile directory under
// `Quill/<origin-hash>/<note-uuid>.md`. Sync is opt-in and off by default.
//
// The controller itself is thin. It:
//   - registers the static assets (quill.html, quill.css, quill.js) under
//     chrome://resources/halcyon_quill/
//   - attaches HalcyonQuillMessageHandler, which owns the actual note
//     storage and bridges chrome.send calls
//
// The page is intentionally per-origin: when the user opens Quill on
// site A and then on site B, they see different notes. The origin is
// reported by the controller (it reads the active WebContents's last
// committed URL), not by the page — so a compromised page can't spoof
// the origin label.
//
// See halcyon/branding/brand_book.md §13.10 for the design intent.

class HalcyonQuillUI : public content::WebUIController {
 public:
  explicit HalcyonQuillUI(content::WebUI* web_ui);
  ~HalcyonQuillUI() override;

  // content::WebUIController:
  void BindInterfaceReceiver() override;

  // Reports the origin of the WebContents that opened this side panel.
  // Empty if the opener is a chrome:// page (Quill is per-origin, not
  // per-chrome-page).
  std::string GetOriginForActiveWebContents() const;

 private:
  void Initialize();
  bool initialized_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_WEBUI_HALCYON_QUILL_UI_H_
