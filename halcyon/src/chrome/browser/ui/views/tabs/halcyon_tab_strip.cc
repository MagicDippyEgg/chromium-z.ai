// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tabs/halcyon_tab_strip.h"

#include "chrome/browser/ui/views/tabs/halcyon_tab.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/layout/box_layout.h"

namespace halcyon {

namespace {

constexpr int kTabStripHeight = 36;       // slightly taller than upstream
constexpr int kTabMinWidth = 80;
constexpr int kTabMaxWidth = 240;
constexpr int kNewTabButtonSize = 28;

}  // namespace

HalcyonTabStrip::HalcyonTabStrip(HalcyonTabStripController* controller)
    : controller_(controller) {
  SetID(VIEW_ID_TAB_STRIP);

  // Box layout: tabs are arranged left-to-right at the start, with the
  // new-tab button hugging the right edge of the last tab.
  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kHorizontal,
      gfx::Insets(),  // no inset — tabs go edge-to-edge
      0));             // no gap — tabs are flush
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);
  layout->set_cross_axis_alignment(views::BoxLayout::CrossAxisAlignment::kStretch);
}

HalcyonTabStrip::~HalcyonTabStrip() = default;

void HalcyonTabStrip::OnThemeChanged() {
  views::View::OnThemeChanged();
  OnThemeChangedInternal();
}

void HalcyonTabStrip::OnThemeChangedInternal() {
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  // The tab strip background is the toolbar background; inactive tabs
  // are slightly recessed. (See brand_book.md §10.)
  SetBackground(views::CreateSolidBackground(ui::halcyon::ToolbarBg(dark)));
  UpdateTabVisuals();
  SchedulePaint();
}

void HalcyonTabStrip::UpdateTabVisuals() {
  for (HalcyonTab* tab : tabs_) {
    if (tab) tab->OnParentThemeChanged(GetNativeTheme());
  }
}

void HalcyonTabStrip::OnPaint(gfx::Canvas* canvas) {
  views::View::OnPaint(canvas);

  // The tab strip has a single 1px bottom border separating it from the
  // toolbar. (It does NOT have a top border — the top of the strip IS the
  // top of the window content.)
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeWidth(1);
  flags.setColor(ui::halcyon::ToolbarBorder(dark));
  int bottom = height() - 1;
  canvas->DrawLine(gfx::Point(0, bottom), gfx::Point(width(), bottom), flags);
}

gfx::Size HalcyonTabStrip::CalculatePreferredSize() const {
  // The strip prefers to be as wide as the parent gives it; height is fixed.
  return gfx::Size(0, kTabStripHeight);
}

void HalcyonTabStrip::Layout() {
  views::View::Layout();

  // Layout the tabs in two groups: anchored tabs at the start (left), then
  // a small divider, then the rest. Each tab gets kTabMaxWidth if there's
  // room, narrowing evenly if there isn't.

  // Count anchored tabs and regular tabs.
  std::vector<HalcyonTab*> anchored;
  std::vector<HalcyonTab*> regular;
  for (HalcyonTab* t : tabs_) {
    if (t->is_anchored()) anchored.push_back(t);
    else regular.push_back(t);
  }

  const int total_width = width();
  const int total_tabs = static_cast<int>(anchored.size() + regular.size());
  if (total_tabs == 0) return;

  // Compute tab width.
  const int total_target = total_tabs * kTabMaxWidth;
  int tab_width;
  if (total_target <= total_width - kNewTabButtonSize - 4) {
    tab_width = kTabMaxWidth;
  } else {
    tab_width = std::max(
        kTabMinWidth,
        (total_width - kNewTabButtonSize - 4) / total_tabs);
  }

  int x = 0;
  auto place = [&](HalcyonTab* tab) {
    tab->SetBounds(x, 0, tab_width, kTabStripHeight);
    x += tab_width;
  };
  for (HalcyonTab* t : anchored) place(t);
  // Small visual divider between anchored and regular tabs.
  if (!anchored.empty() && !regular.empty()) {
    x += 2;
  }
  for (HalcyonTab* t : regular) place(t);

  // Place the new-tab button at the right of the last tab.
  if (new_tab_button_) {
    new_tab_button_->SetBounds(
        x + 4, (kTabStripHeight - kNewTabButtonSize) / 2,
        kNewTabButtonSize, kNewTabButtonSize);
  }
}

HalcyonTab* HalcyonTabStrip::AddTab(int index, std::unique_ptr<HalcyonTab> tab) {
  HalcyonTab* raw = tab.get();
  tabs_.insert(tabs_.begin() + index, raw);
  AddChildView(std::move(tab));
  Layout();
  return raw;
}

std::unique_ptr<HalcyonTab> HalcyonTabStrip::RemoveTab(int index) {
  if (index < 0 || index >= static_cast<int>(tabs_.size())) return nullptr;
  HalcyonTab* raw = tabs_[index];
  tabs_.erase(tabs_.begin() + index);
  auto owned = base::WrapUnique(raw);
  // RemoveChildView triggers a Layout via LayoutManager.
  RemoveChildView(raw);
  return owned;
}

HalcyonTab* HalcyonTabStrip::GetTabAt(int index) {
  if (index < 0 || index >= static_cast<int>(tabs_.size())) return nullptr;
  return tabs_[index];
}

void HalcyonTabStrip::SetActiveTab(int index) {
  active_index_ = index;
  for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
    HalcyonTab* t = tabs_[i];
    if (t) t->set_active(i == index);
  }
  SchedulePaint();
}

void HalcyonTabStrip::MoveTab(int from, int to) {
  if (from == to) return;
  HalcyonTab* t = tabs_[from];
  tabs_.erase(tabs_.begin() + from);
  tabs_.insert(tabs_.begin() + to, t);
  Layout();
}

}  // namespace halcyon
