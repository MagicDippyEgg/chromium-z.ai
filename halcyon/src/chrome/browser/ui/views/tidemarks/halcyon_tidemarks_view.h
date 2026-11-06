// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TIDEMARKS_HALCYON_TIDEMARKS_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_TIDEMARKS_HALCYON_TIDEMARKS_VIEW_H_

#include <memory>
#include <vector>

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

class HalcyonTab;
class HalcyonTabStripController;

namespace halcyon {

// HalcyonTidemarksView — the optional vertical tab rail on the left edge of
// the window. (See brand_book.md §13.2.)
//
// When Tidemarks is enabled (via Settings or ⌘⇧L), the regular horizontal
// HalcyonTabStrip is hidden and this view takes its place, spanning the full
// height of the window's left edge.
//
// Visual properties:
//   - The rail is 200px wide by default (configurable in Settings).
//   - Tabs are stacked vertically. Each tab is a "card": favicon dot on the
//     left, truncated title on the right. Click to switch; drag to reorder.
//   - Inactive tabs visually recede (lower opacity, smaller padding).
//     This is Halcyon's signature "fade-back" effect.
//   - Anchored tabs group at the top with a divider.
//   - The "new tab" button is at the bottom of the rail.
//
// Hovering the rail for >400ms expands it by 60px (showing full titles).
// Leaving the rail collapses it back. This is the only "automatic" animation
// in Halcyon's chrome; everything else responds to direct user input.

class HalcyonTidemarksView : public views::View {
  METADATA_HEADER(HalcyonTidemarksView, views::View)

 public:
  explicit HalcyonTidemarksView(HalcyonTabStripController* controller);
  ~HalcyonTidemarksView() override;

  // views::View:
  void OnThemeChanged() override;
  void OnMouseEntered(const ui::MouseEvent& event) override;
  void OnMouseExited(const ui::MouseEvent& event) override;
  gfx::Size CalculatePreferredSize() const override;
  void Layout() override;

  // Adds/removes tabs (same ownership model as HalcyonTabStrip).
  HalcyonTab* AddTab(int index, std::unique_ptr<HalcyonTab> tab);
  std::unique_ptr<HalcyonTab> RemoveTab(int index);
  void SetActiveTab(int index);

  // Sets the rail's preferred width (narrow / default / wide per Settings).
  void SetRailWidth(int width_dips);

 private:
  void ScheduleExpand();
  void ScheduleCollapse();
  void OnExpandTimer();

  raw_ptr<HalcyonTabStripController> controller_;
  std::vector<raw_ptr<HalcyonTab>> tabs_;
  int active_index_ = -1;
  int rail_width_ = 200;
  int expanded_width_ = 260;

  // Timer for hover-to-expand.
  base::OneShotTimer expand_timer_;
  bool expanded_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_TIDEMARKS_HALCYON_TIDEMARKS_VIEW_H_
