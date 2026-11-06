// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_HALCYON_TAB_STRIP_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_HALCYON_TAB_STRIP_H_

#include <memory>
#include <vector>

#include "base/timer/timer.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

class HalcyonTab;
class HalcyonTabStripController;

namespace gfx {
class Canvas;
}

namespace halcyon {

// HalcyonTabStrip is the Halcyon replacement for upstream's TabStrip
// (chrome/browser/ui/views/tabs/tab_strip.*).
//
// Differences from upstream TabStrip:
//
//   1. Tabs have NO rounded corners. They are clipped flat against the
//      bottom of the strip (the toolbar top).
//   2. There is no top gap and no side padding for the first tab.
//      Halcyon tabs go edge-to-edge across the strip.
//   3. The active tab's background matches the toolbar / page background,
//      so the active tab visually IS the page.
//   4. Inactive tabs have a softer, slightly recessed surface color.
//      They "recede" visually.
//   5. Anchored tabs (see brand_book.md §13.10) have a distinct surface
//      color and group together at the start of the strip.
//   6. The "new tab" button is on the right edge of the strip — a single
//      small "+" icon, not a full button.
//   7. There is no tab search button (upstream's TabSearchBubble).
//      The Tidemarks view replaces it.
//
// HalcyonTabStrip is created by HalcyonBrowserView::CreateTabStrip() and
// owned by HalcyonBrowserView.
//
// When Tidemarks (vertical tabs) is enabled, this view is hidden and a
// HalcyonTidemarksView is shown instead, on the left edge of the window.
// See HalcyonBrowserView::UpdateTabLayout() for the toggle.

class HalcyonTabStrip : public views::View {
  METADATA_HEADER(HalcyonTabStrip, views::View)

 public:
  explicit HalcyonTabStrip(HalcyonTabStripController* controller);
  ~HalcyonTabStrip() override;

  // views::View:
  void OnThemeChanged() override;
  void OnPaint(gfx::Canvas* canvas) override;
  gfx::Size CalculatePreferredSize() const override;
  void Layout() override;

  // Adds a new tab at the given index. The tab is owned by this view.
  HalcyonTab* AddTab(int index, std::unique_ptr<HalcyonTab> tab);

  // Removes and returns the tab at the given index. Caller now owns it.
  std::unique_ptr<HalcyonTab> RemoveTab(int index);

  // Returns the tab at the given index, or nullptr.
  HalcyonTab* GetTabAt(int index);
  int GetTabCount() const { return static_cast<int>(tabs_.size()); }

  // Marks the tab at the given index as active. Other tabs become inactive.
  void SetActiveTab(int index);

  // Moves a tab from |from| to |to|, animating the layout.
  void MoveTab(int from, int to);

 private:
  void OnThemeChangedInternal();
  void UpdateTabVisuals();

  raw_ptr<HalcyonTabStripController> controller_;

  // Children are stored in visual order. The first tab is at the left edge.
  std::vector<raw_ptr<HalcyonTab>> tabs_;

  int active_index_ = -1;

  // The "new tab" button at the right edge.
  raw_ptr<views::Button> new_tab_button_ = nullptr;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_HALCYON_TAB_STRIP_H_
