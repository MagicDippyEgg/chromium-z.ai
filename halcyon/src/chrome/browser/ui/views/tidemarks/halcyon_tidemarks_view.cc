// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tidemarks/halcyon_tidemarks_view.h"

#include "base/bind.h"
#include "base/timer/timer.h"
#include "chrome/browser/ui/views/tabs/halcyon_tab.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/layout/box_layout.h"

namespace halcyon {

namespace {

constexpr int kTidemarksHeight = 36;       // matches HalcyonTabStrip height
constexpr int kTidemarksHoverDelayMs = 400;
constexpr int kTidemarksExpandDelta = 60;

}  // namespace

HalcyonTidemarksView::HalcyonTidemarksView(
    HalcyonTabStripController* controller)
    : controller_(controller) {
  SetID(VIEW_ID_TIDEMARKS_RAIL);

  // Vertical stack.
  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical,
      gfx::Insets::VH(8, 0),
      0));
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);
  layout->set_cross_axis_alignment(views::BoxLayout::CrossAxisAlignment::kStretch);

  SetRailWidth(200);  // default
}

HalcyonTidemarksView::~HalcyonTidemarksView() = default;

void HalcyonTidemarksView::SetRailWidth(int width_dips) {
  rail_width_ = width_dips;
  expanded_width_ = width_dips + kTidemarksExpandDelta;
  InvalidateLayout();
  PreferredSizeChanged();
}

void HalcyonTidemarksView::OnThemeChanged() {
  views::View::OnThemeChanged();
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  // The Tidemarks rail uses the same background as the toolbar.
  SetBackground(views::CreateSolidBackground(ui::halcyon::ToolbarBg(dark)));
  for (HalcyonTab* t : tabs_) {
    if (t) t->OnParentThemeChanged(GetNativeTheme());
  }
  SchedulePaint();
}

void HalcyonTidemarksView::OnMouseEntered(const ui::MouseEvent& event) {
  views::View::OnMouseEntered(event);
  ScheduleExpand();
}

void HalcyonTidemarksView::OnMouseExited(const ui::MouseEvent& event) {
  views::View::OnMouseExited(event);
  ScheduleCollapse();
}

void HalcyonTidemarksView::ScheduleExpand() {
  expand_timer_.Stop();
  expand_timer_.Start(
      FROM_HERE, base::Milliseconds(kTidemarksHoverDelayMs),
      base::BindOnce(&HalcyonTidemarksView::OnExpandTimer,
                     base::Unretained(this)));
}

void HalcyonTidemarksView::ScheduleCollapse() {
  expand_timer_.Stop();
  if (expanded_) {
    expanded_ = false;
    InvalidateLayout();
    PreferredSizeChanged();
  }
}

void HalcyonTidemarksView::OnExpandTimer() {
  expanded_ = true;
  InvalidateLayout();
  PreferredSizeChanged();
}

gfx::Size HalcyonTidemarksView::CalculatePreferredSize() const {
  // The view takes the full available height of the parent.
  // Width is the rail width (or expanded when expanded_).
  return gfx::Size(expanded_ ? expanded_width_ : rail_width_, 0);
}

void HalcyonTidemarksView::Layout() {
  views::View::Layout();

  // Stack the tabs vertically. Anchored tabs go first, then a divider, then
  // the regular tabs. Each tab is 36px tall by default; anchored tabs are
  // 32px (slightly more compact).
  std::vector<HalcyonTab*> anchored;
  std::vector<HalcyonTab*> regular;
  for (HalcyonTab* t : tabs_) {
    if (t->is_anchored()) anchored.push_back(t);
    else regular.push_back(t);
  }

  int y = 8;
  const int w = width();
  for (HalcyonTab* t : anchored) {
    t->SetBounds(0, y, w, 32);
    y += 32;
  }
  if (!anchored.empty() && !regular.empty()) {
    y += 4;  // small divider space
  }
  for (HalcyonTab* t : regular) {
    t->SetBounds(0, y, w, 36);
    y += 36;
  }
}

HalcyonTab* HalcyonTidemarksView::AddTab(int index,
                                          std::unique_ptr<HalcyonTab> tab) {
  HalcyonTab* raw = tab.get();
  tabs_.insert(tabs_.begin() + index, raw);
  AddChildView(std::move(tab));
  Layout();
  return raw;
}

std::unique_ptr<HalcyonTab> HalcyonTidemarksView::RemoveTab(int index) {
  if (index < 0 || index >= static_cast<int>(tabs_.size())) return nullptr;
  HalcyonTab* raw = tabs_[index];
  tabs_.erase(tabs_.begin() + index);
  auto owned = base::WrapUnique(raw);
  RemoveChildView(raw);
  return owned;
}

void HalcyonTidemarksView::SetActiveTab(int index) {
  active_index_ = index;
  for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
    if (tabs_[i]) tabs_[i]->set_active(i == index);
  }
  SchedulePaint();
}

}  // namespace halcyon
