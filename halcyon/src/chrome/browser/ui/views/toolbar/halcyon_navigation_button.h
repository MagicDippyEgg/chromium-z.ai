// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_NAVIGATION_BUTTON_H_
#define CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_NAVIGATION_BUTTON_H_

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/controls/button/image_button.h"

class Browser;

namespace halcyon {

// HalcyonNavigationButton — back, forward, reload.
//
// These are simple image buttons that render the Halcyon icon set's
// chevron-left, chevron-right, and arrow-refresh glyphs. They are slightly
// larger than upstream's navigation buttons (32px vs 28px) for a calmer feel.

class HalcyonNavigationButton : public views::ImageButton {
  METADATA_HEADER(HalcyonNavigationButton, views::ImageButton)

 public:
  enum Type { kBack, kForward, kReload };

  HalcyonNavigationButton(Type type, Browser* browser);
  ~HalcyonNavigationButton() override;

  void OnThemeChanged() override;
  void OnPaint(gfx::Canvas* canvas) override;

 private:
  Type type_;
  raw_ptr<Browser> browser_;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_NAVIGATION_BUTTON_H_
