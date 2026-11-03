// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_TOOLBAR_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_TOOLBAR_VIEW_H_

#include <memory>

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/view.h"

class HalcyonOmniboxView;
class HalcyonLanternIndicatorView;
class HalcyonCairnIndicatorView;
class HalcyonMenuButton;

namespace content {
class WebContents;
}

namespace halcyon {

// HalcyonToolbarView is the Halcyon replacement for upstream's ToolbarView.
//
// In upstream Chromium, ToolbarView (chrome/browser/ui/views/toolbar/toolbar_view.*)
// is the container that owns the back/forward, reload, omnibox, browser
// actions, the 3-dot menu, and (in newer versions) the profile avatar.
//
// HalcyonToolbarView replaces it with a much simpler, calmer toolbar:
//
//   [<- back] [-> fwd] [reload]   [   omnibox   ]   [lantern] [cairn] [menu]
//
// Differences from upstream ToolbarView:
//   1. No profile avatar button (Halcyon doesn't prompt for sign-in).
//   2. No "send feedback" or "what's new" buttons.
//   3. No extension toolbar actions area by default — extensions are shown
//      only via the menu, not in the chrome.
//   4. No "side panel" / Gemini / Reading Mode button.
//   5. The Lantern indicator (privacy) is a first-class toolbar citizen.
//   6. The Cairn indicator (bookmarks) is also a first-class citizen.
//   7. The menu button is a single horizontal line (the brand mark),
//      not three horizontal lines (≡).
//
// HalcyonToolbarView is created by HalcyonBrowserView::CreateToolbar() and
// owned by HalcyonBrowserView. See patches/browser_view_replace_toolbar.patch
// for the integration point in upstream BrowserView.

class HalcyonToolbarView : public views::BoxLayoutView {
  METADATA_HEADER(HalcyonToolbarView, views::BoxLayoutView)

 public:
  HalcyonToolbarView();
  ~HalcyonToolbarView() override;

  // Initializes the toolbar with the given browser and web contents.
  void Init(Browser* browser, content::WebContents* web_contents);

  // views::View:
  void OnThemeChanged() override;
  gfx::Size CalculatePreferredSize() const override;
  void ChildPreferredSizeChanged(views::View* child) override;

  // Accessors.
  HalcyonOmniboxView* omnibox() { return omnibox_; }
  HalcyonLanternIndicatorView* lantern() { return lantern_; }
  HalcyonMenuButton* menu_button() { return menu_button_; }

 private:
  // Owned by the view hierarchy (via views::View::AddChildView).
  raw_ptr<HalcyonOmniboxView> omnibox_ = nullptr;
  raw_ptr<HalcyonLanternIndicatorView> lantern_ = nullptr;
  raw_ptr<HalcyonCairnIndicatorView> cairn_ = nullptr;
  raw_ptr<HalcyonMenuButton> menu_button_ = nullptr;

  // Back, forward, reload. Halcyon uses simple image buttons.
  raw_ptr<views::Button> back_ = nullptr;
  raw_ptr<views::Button> forward_ = nullptr;
  raw_ptr<views::Button> reload_ = nullptr;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_TOOLBAR_HALCYON_TOOLBAR_VIEW_H_
