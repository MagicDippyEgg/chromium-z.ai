// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_MENU_BUTTON_H_
#define CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_MENU_BUTTON_H_

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/controls/button/button.h"

class Browser;

namespace halcyon {

// HalcyonMenuButton is the Halcyon equivalent of upstream's
// ToolbarButton / WrenchMenuButton. It is the button at the top-right of the
// window that opens the main browser menu.
//
// Visually, Halcyon's menu button is a single horizontal line (the brand mark,
// minus the surrounding circle) inside a small bordered pill. Clicking it
// opens HalcyonMenuController, which shows a single column of plain text
// labels with section dividers and no icons (see brand_book.md §14).
//
// The button's icon is NOT the upstream "≡" three-line glyph. It is a single
// horizontal line, deliberately — three lines suggests "menu"; one line
// suggests "calm". The single line is also the Lantern indicator's icon,
// which unifies the visual language.

class HalcyonMenuButton : public views::Button {
  METADATA_HEADER(HalcyonMenuButton, views::Button)

 public:
  explicit HalcyonMenuButton(Browser* browser);
  ~HalcyonMenuButton() override;

  // views::Button:
  void OnThemeChanged() override;

 private:
  void OnClick();
  raw_ptr<Browser> browser_;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_MENU_BUTTON_H_
