// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_FRAME_HALCYON_BROWSER_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_FRAME_HALCYON_BROWSER_VIEW_H_

#include <memory>

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

class HalcyonToolbarView;
class HalcyonTabStrip;
class HalcyonTidemarksView;
class HalcyonFrameView;

namespace content {
class WebContents;
}

namespace halcyon {

// HalcyonBrowserView — the Halcyon replacement for upstream's BrowserView.
//
// BrowserView (chrome/browser/ui/views/frame/browser_view.*) is the top-level
// views::View that owns the toolbar, tab strip, omnibox, bookmarks bar,
// downloads shelf, status bubble, and the contents area. HalcyonBrowserView
// replaces it with a much slimmer shell:
//
//   +-----------------------------------------------+
//   | Frame (slim, 1px sage top border)             |
//   +-----------------------------------------------+
//   | [Tab strip] (or, if Tidemarks on, hidden —     |
//   |  TidemarksView is shown on the left edge)      |
//   +-----------------------------------------------+
//   | [Toolbar — back, fwd, reload, omnibox,         |
//   |  lantern, cairn, menu]                         |
//   +-----------------------------------------------+
//   | <web contents>                                 |
//   +-----------------------------------------------+
//
// Differences from upstream BrowserView:
//   1. No bookmarks bar — bookmarks are a cairn popup, not a horizontal bar.
//   2. No downloads shelf — downloads are shown in the Lantern-style toolbar
//      indicator + the chrome://downloads page.
//   3. No "info bar" container — info bars are replaced by in-page
//      notifications or the Lantern, depending on severity.
//   4. No "FindBar" floating widget — Find uses a simple slim bar at the
//      top-right of the contents area (see HalcyonFindBarView).
//   5. No "StatusBubble" — Halcyon shows status in a tiny dim text at the
//      bottom-left of the contents area, only when hovering a link.

class HalcyonBrowserView : public views::View {
  METADATA_HEADER(HalcyonBrowserView, views::View)

 public:
  explicit HalcyonBrowserView(Browser* browser);
  ~HalcyonBrowserView() override;

  void Init(content::WebContents* web_contents);

  // views::View:
  void OnThemeChanged() override;
  gfx::Size CalculatePreferredSize() const override;
  void Layout() override;

  // Toggles between the horizontal TabStrip and the vertical TidemarksView.
  void SetTidemarksEnabled(bool enabled);

  // Returns the various sub-views for tests / external callers.
  HalcyonToolbarView* toolbar() { return toolbar_; }
  HalcyonTabStrip* tab_strip() { return tab_strip_; }
  HalcyonTidemarksView* tidemarks() { return tidemarks_; }

 private:
  raw_ptr<Browser> browser_;

  // Owned by the view hierarchy.
  raw_ptr<HalcyonTabStrip> tab_strip_ = nullptr;
  raw_ptr<HalcyonTidemarksView> tidemarks_ = nullptr;
  raw_ptr<HalcyonToolbarView> toolbar_ = nullptr;
  raw_ptr<views::View> contents_container_ = nullptr;

  // When Tidemarks is on, the regular tab strip is hidden and the tidemarks
  // view takes the left edge.
  bool tidemarks_enabled_ = false;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_FRAME_HALCYON_BROWSER_VIEW_H_
