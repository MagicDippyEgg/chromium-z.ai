// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/toolbar/halcyon_toolbar_view.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/location_bar/halcyon_omnibox_view.h"
#include "chrome/browser/ui/views/privacy_lantern/halcyon_lantern_indicator_view.h"
#include "chrome/browser/ui/views/bookmarks/halcyon_cairn_indicator_view.h"
#include "chrome/browser/ui/views/toolbar/halcyon_menu_button.h"
#include "chrome/browser/ui/views/toolbar/halcyon_navigation_button.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view_class_properties.h"

namespace halcyon {

namespace {

constexpr int kToolbarHeight = 44;  // slightly taller than upstream's 36, for air

}  // namespace

HalcyonToolbarView::HalcyonToolbarView() {
  SetID(VIEW_ID_TOOLBAR);

  // Box layout: left-aligned contents, with the omnibox getting any extra
  // horizontal space. Halcyon uses generous spacing between elements.
  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kHorizontal,
      gfx::Insets::VH(8, 12),  // vertical/horizontal inset
      8));                     // between-child spacing
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);
  layout->set_cross_axis_alignment(views::BoxLayout::CrossAxisAlignment::kCenter);
}

HalcyonToolbarView::~HalcyonToolbarView() = default;

void HalcyonToolbarView::Init(Browser* browser, content::WebContents* web_contents) {
  // Navigation buttons — back, forward, reload. Halcyon uses small,
  // stroke-only icons (chevron-left, chevron-right, arrow-refresh).
  back_ = AddChildView(std::make_unique<HalcyonNavigationButton>(
      HalcyonNavigationButton::kBack, browser));
  forward_ = AddChildView(std::make_unique<HalcyonNavigationButton>(
      HalcyonNavigationButton::kForward, browser));
  reload_ = AddChildView(std::make_unique<HalcyonNavigationButton>(
      HalcyonNavigationButton::kReload, browser));

  // A small flexible spacer that lets the omnibox take remaining space.
  auto* spacer = AddChildView(std::make_unique<views::View>());
  spacer->SetProperty(views::kBoxLayoutFlexKey, 1.0f);

  // The omnibox.
  omnibox_ = AddChildView(std::make_unique<HalcyonOmniboxView>(browser, web_contents));
  omnibox_->SetProperty(views::kBoxLayoutFlexKey, 4.0f);

  // Another spacer that lets the right-side buttons hug the right edge.
  auto* right_spacer = AddChildView(std::make_unique<views::View>());
  right_spacer->SetProperty(views::kBoxLayoutFlexKey, 1.0f);

  // The Lantern indicator.
  lantern_ = AddChildView(std::make_unique<HalcyonLanternIndicatorView>(browser, web_contents));

  // The Cairn indicator.
  cairn_ = AddChildView(std::make_unique<HalcyonCairnIndicatorView>(browser));

  // The menu button — a single horizontal line (the brand mark).
  menu_button_ = AddChildView(std::make_unique<HalcyonMenuButton>(browser));
}

void HalcyonToolbarView::OnThemeChanged() {
  views::BoxLayoutView::OnThemeChanged();

  // The toolbar background matches the page background (Vellum/Eclipse), so
  // the toolbar visually IS the page, separated from the tab strip above by
  // a thin border-soft hairline. (See brand_book.md §10.)
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  SetBackground(views::CreateSolidBackground(ui::halcyon::ToolbarBg(dark)));

  // Bottom border only (top border is the tab strip's bottom border).
  SetBorder(views::CreateSolidSidedBorder(
      0, 0, 1, 0, ui::halcyon::ToolbarBorder(dark)));
}

gfx::Size HalcyonToolbarView::CalculatePreferredSize() const {
  // Width is whatever the parent gives us; height is fixed.
  return gfx::Size(views::BoxLayoutView::CalculatePreferredSize().width(),
                   kToolbarHeight);
}

void HalcyonToolbarView::ChildPreferredSizeChanged(views::View* child) {
  Layout();
  SchedulePaint();
}

}  // namespace halcyon
