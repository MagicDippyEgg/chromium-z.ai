// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_HALCYON_OMNIBOX_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_HALCYON_OMNIBOX_VIEW_H_

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

class Browser;

namespace content {
class WebContents;
}

namespace halcyon {

// HalcyonOmniboxView — the Halcyon omnibox.
//
// Visual properties (brand_book.md §7):
//   - The omnibox is a single horizontal pill, 40px tall, with 6px corner
//     radius (Halcyon's kRadiusMd), in the toolbar's center.
//   - Unfocused background: Mist (the surface color), 1px hairline border
//     in Thready.
//   - Focused background: a slightly lighter Vellum (the "flashlight turned
//     on" effect), 1.5px Sage border. A subtle focus ring appears.
//   - The omnibox does NOT have:
//       - a "share" button on the right
//       - a "save to cairn" star icon (bookmarks are added via the menu
//         or right-click)
//       - a "page info" lock icon on the left (security info is shown in
//         the Lantern popup, not the omnibox)
//       - a "verification icon" for HTTPS
//   - The omnibox DOES have:
//       - a margin note indicator on the right (when the current URL has
//         margin notes — see brand_book.md §13.5)
//       - a small "the page is loading" pulse mark on the left (a single
//         dot that grows into a line, same as the tab favicon)
//   - The omnibox's placeholder is "Search or type a URL" — no marketing copy.
//
// Functionally, HalcyonOmniboxView wraps the upstream OmniboxView (the
// textfield) and OmniboxPopupView (the autocomplete dropdown). It owns the
// OmniboxController, which talks to the AutocompleteController. See
// patches/replace_omnibox_view.patch for the integration point.

class HalcyonOmniboxView : public views::View {
  METADATA_HEADER(HalcyonOmniboxView, views::View)

 public:
  HalcyonOmniboxView(Browser* browser, content::WebContents* web_contents);
  ~HalcyonOmniboxView() override;

  // views::View:
  void OnThemeChanged() override;
  gfx::Size CalculatePreferredSize() const override;
  void Layout() override;
  void OnPaint(gfx::Canvas* canvas) override;
  void OnFocus() override;
  void OnBlur() override;

  // Reverts the omnibox text to the current URL (when the user clicks away
  // without committing).
  void Revert();

 private:
  raw_ptr<Browser> browser_;
  raw_ptr<content::WebContents> web_contents_;

  // The actual text field. Owned by this view.
  raw_ptr<views::Textfield> textfield_ = nullptr;

  // True when the textfield has focus. Used to draw the focus border.
  bool focused_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_HALCYON_OMNIBOX_VIEW_H_
