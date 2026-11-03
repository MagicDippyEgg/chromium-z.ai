// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/frame/halcyon_browser_view.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/frame/halcyon_frame_view.h"
#include "chrome/browser/ui/views/tabs/halcyon_tab_strip.h"
#include "chrome/browser/ui/views/tidemarks/halcyon_tidemarks_view.h"
#include "chrome/browser/ui/views/toolbar/halcyon_toolbar_view.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/views/background.h"
#include "ui/views/layout/box_layout.h"

namespace halcyon {

namespace {

constexpr int kToolbarHeight = 44;
constexpr int kTabStripHeight = 36;
constexpr int kTidemarksWidth = 200;

}  // namespace

HalcyonBrowserView::HalcyonBrowserView(Browser* browser)
    : browser_(browser) {
  SetID(VIEW_ID_BROWSER_VIEW);

  // The layout is a vertical stack: tab strip on top (when Tidemarks is off),
  // toolbar below it, then the contents fill the rest.
  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical,
      gfx::Insets(), 0));
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);
  layout->set_cross_axis_alignment(views::BoxLayout::CrossAxisAlignment::kStretch);
}

HalcyonBrowserView::~HalcyonBrowserView() = default;

void HalcyonBrowserView::Init(content::WebContents* web_contents) {
  tab_strip_ = AddChildView(std::make_unique<HalcyonTabStrip>(nullptr));
  // (The controller is attached by the BrowserView's controller bridge.)

  toolbar_ = AddChildView(std::make_unique<HalcyonToolbarView>());
  toolbar_->Init(browser_, web_contents);

  // Tidemarks is created off-screen; shown only if the user has enabled it.
  tidemarks_ = AddChildView(std::make_unique<HalcyonTidemarksView>(nullptr));
  tidemarks_->SetVisible(false);

  // The contents container is the rest of the space; it's owned externally
  // (the WebContentsView), but we expose a slot here for it to be placed in.
  contents_container_ = AddChildView(std::make_unique<views::View>());
  contents_container_->SetProperty(views::kBoxLayoutFlexKey, 1.0f);

  SetTidemarksEnabled(false);
}

void HalcyonBrowserView::SetTidemarksEnabled(bool enabled) {
  tidemarks_enabled_ = enabled;
  if (tab_strip_) tab_strip_->SetVisible(!enabled);
  if (tidemarks_) {
    tidemarks_->SetVisible(enabled);
    if (enabled) {
      tidemarks_->SetRailWidth(kTidemarksWidth);
    }
  }
  Layout();
}

void HalcyonBrowserView::OnThemeChanged() {
  views::View::OnThemeChanged();
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  SetBackground(views::CreateSolidBackground(ui::halcyon::Bg(dark)));
  SchedulePaint();
}

gfx::Size HalcyonBrowserView::CalculatePreferredSize() const {
  // Width is whatever the window provides. Height is whatever the window
  // provides. The Layout() method distributes space among the children.
  return gfx::Size();
}

void HalcyonBrowserView::Layout() {
  views::View::Layout();

  // If Tidemarks is on, it takes the left edge; the rest of the chrome
  // (toolbar, contents) is offset by the rail's width.
  int left_inset = tidemarks_enabled_ ? kTidemarksWidth : 0;
  if (tidemarks_ && tidemarks_enabled_) {
    tidemarks_->SetBounds(0, 0, kTidemarksWidth, height());
  }

  // Tab strip and toolbar span the remaining width.
  int chrome_x = left_inset;
  int chrome_w = width() - left_inset;
  int y = 0;
  if (tab_strip_ && tab_strip_->GetVisible()) {
    tab_strip_->SetBounds(chrome_x, y, chrome_w, kTabStripHeight);
    y += kTabStripHeight;
  }
  if (toolbar_) {
    toolbar_->SetBounds(chrome_x, y, chrome_w, kToolbarHeight);
    y += kToolbarHeight;
  }
  if (contents_container_) {
    contents_container_->SetBounds(chrome_x, y, chrome_w, height() - y);
  }
}

}  // namespace halcyon
