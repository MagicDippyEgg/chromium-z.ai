// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_HALCYON_TAB_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_HALCYON_TAB_H_

#include <string>

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

namespace ui {
class NativeTheme;
}

class gfx::Canvas;

namespace halcyon {

// HalcyonTab — a single tab in the HalcyonTabStrip or HalcyonTidemarksView.
//
// Visual properties (brand_book.md §7):
//   - Active tab: background matches the page bg, so the tab visually IS
//     the page.
//   - Inactive tab: a softer, recessed surface color.
//   - Anchored tab: distinct surface, group-at-start.
//   - No rounded corners. Tabs are clipped flat against the bottom of the
//     strip.
//   - No favicon — just a small dot that grows from a circle into the
//     brand-mark line when the page is loading. (Halcyon's most distinctive
//     chrome detail.)
//   - The active tab has a 2px Tide-colored underline at the bottom.
//   - Hover state slightly raises the inactive tab's surface color.
//
// The "tab title" is rendered in the secondary text color when inactive
// and the primary text color when active. The font is Inter 11px (caption).
// (See branding/typography.md.)

class HalcyonTab : public views::View {
  METADATA_HEADER(HalcyonTab, views::View)

 public:
  HalcyonTab();
  ~HalcyonTab() override;

  // views::View:
  void OnPaint(gfx::Canvas* canvas) override;
  void OnThemeChanged() override;
  void OnMouseEntered(const ui::MouseEvent& event) override;
  void OnMouseExited(const ui::MouseEvent& event) override;
  bool OnMousePressed(const ui::MouseEvent& event) override;
  void StateChanged(int old_state, int new_state) override;

  // Setters.
  void set_active(bool active) { active_ = active; SchedulePaint(); }
  bool is_active() const { return active_; }
  void set_anchored(bool anchored) { anchored_ = anchored; SchedulePaint(); }
  bool is_anchored() const { return anchored_; }
  void set_loading(bool loading) { loading_ = loading; SchedulePaint(); }
  void set_title(const std::u16string& title) { title_ = title; SchedulePaint(); }
  void set_url(const GURL& url) { url_ = url; SchedulePaint(); }

  // Called by HalcyonTabStrip when the parent's theme changes.
  void OnParentThemeChanged(ui::NativeTheme* theme);

 private:
  void PaintBackground(gfx::Canvas* canvas);
  void PaintFavicon(gfx::Canvas* canvas);
  void PaintTitle(gfx::Canvas* canvas);
  void PaintActiveUnderline(gfx::Canvas* canvas);
  void PaintCloseButton(gfx::Canvas* canvas);
  void PaintAnchorMark(gfx::Canvas* canvas);

  std::u16string title_;
  GURL url_;

  bool active_ = false;
  bool anchored_ = false;
  bool loading_ = false;
  bool hovered_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_HALCYON_TAB_H_
