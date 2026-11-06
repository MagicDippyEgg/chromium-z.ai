// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_FRAME_HALCYON_FRAME_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_FRAME_HALCYON_FRAME_VIEW_H_

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/window/non_client_view.h"

namespace halcyon {

// HalcyonFrameView — the Halcyon replacement for upstream's
// BrowserNonClientFrameView (and its platform subclasses: GlassBrowserFrameView
// on Windows,OpaqueBrowserFrameView on Linux, NativeBrowserFrameView on
// macOS).
//
// The Halcyon frame is deliberately minimal:
//   - On macOS, Halcyon uses the system window chrome but with the Halcyon
//     palette. (We respect macOS's expected window control positions.)
//   - On Windows and Linux, Halcyon uses a custom-drawn frame with a single
//     1px sage border around the entire window. (No 8px client-edge; no
//     fake "Windows 10/11" shadow.)
//   - There is NO title bar text by default. The window's title is shown
//     only in the OS's taskbar/dock/menubar, not in the chrome.
//   - When the user hovers near the top of the window, a small dim title
//     fades in for 1 second and fades back out.
//
// HalcyonFrameView is the parent of HalcyonBrowserView in the views hierarchy.
// It is created by HalcyonBrowserFrame (which subclasses upstream's
// BrowserFrame).

class HalcyonFrameView : public views::NonClientFrameView {
  METADATA_HEADER(HalcyonFrameView, views::NonClientFrameView)

 public:
  explicit HalcyonFrameView(Browser* browser);
  ~HalcyonFrameView() override;

  // views::NonClientFrameView:
  gfx::Rect GetBoundsForClientView() const override;
  gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const override;
  int NonClientHitTest(const gfx::Point& point) override;
  void GetWindowMask(const gfx::Size& size, SkPath* window_mask) override;
  void ResetWindowControls() override;
  void UpdateWindowRoundedCorners() override;
  void UpdateWindowControlsOverlay(const gfx::Size& size) override;

  // views::View:
  void OnPaint(gfx::Canvas* canvas) override;
  void OnThemeChanged() override;
  gfx::Size CalculatePreferredSize() const override;
  void Layout() override;

 private:
  raw_ptr<Browser> browser_;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_FRAME_HALCYON_FRAME_VIEW_H_
