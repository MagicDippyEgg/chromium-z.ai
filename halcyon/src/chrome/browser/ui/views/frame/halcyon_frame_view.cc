// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/frame/halcyon_frame_view.h"

#include "chrome/browser/ui/browser.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/skia_util.h"
#include "ui/views/background.h"
#include "ui/views/widget/widget.h"

namespace halcyon {

namespace {

constexpr int kFrameBorderThickness = 1;  // single sage hairline around the window

}  // namespace

HalcyonFrameView::HalcyonFrameView(Browser* browser) : browser_(browser) {
  SetID(VIEW_ID_FRAME);
  SetBackgroundColor(SK_ColorTRANSPARENT);
}

HalcyonFrameView::~HalcyonFrameView() = default;

gfx::Rect HalcyonFrameView::GetBoundsForClientView() const {
  // The client view (which contains HalcyonBrowserView) fills the entire
  // frame minus the border.
  return gfx::Rect(
      kFrameBorderThickness, kFrameBorderThickness,
      width() - 2 * kFrameBorderThickness,
      height() - 2 * kFrameBorderThickness);
}

gfx::Rect HalcyonFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  return gfx::Rect(
      client_bounds.x() - kFrameBorderThickness,
      client_bounds.y() - kFrameBorderThickness,
      client_bounds.width() + 2 * kFrameBorderThickness,
      client_bounds.height() + 2 * kFrameBorderThickness);
}

int HalcyonFrameView::NonClientHitTest(const gfx::Point& point) {
  // Allow the user to drag the window from anywhere along the top of the
  // frame that's not on a button or omnibox.
  if (point.y() < kFrameBorderThickness + 4) return HTCAPTION;
  // Defer to children for the rest.
  return views::NonClientFrameView::NonClientHitTest(point);
}

void HalcyonFrameView::GetWindowMask(const gfx::Size& size,
                                       SkPath* window_mask) {
  // Halcyon windows are plain rectangles on Windows/Linux and use the system
  // chrome on macOS. No custom shape.
  window_mask->addRect(gfx::RectToSkRect(gfx::Rect(size)));
}

void HalcyonFrameView::ResetWindowControls() {}

void HalcyonFrameView::UpdateWindowRoundedCorners() {}

void HalcyonFrameView::UpdateWindowControlsOverlay(const gfx::Size& size) {}

void HalcyonFrameView::OnPaint(gfx::Canvas* canvas) {
  views::NonClientFrameView::OnPaint(canvas);

  // Draw a 1px Sage border around the entire window.
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeWidth(kFrameBorderThickness);
  flags.setColor(ui::halcyon::Border(dark));
  canvas->DrawRect(gfx::Rect(0, 0, width() - 1, height() - 1), flags);

  // Halcyon windows do NOT paint a title bar. The system title bar is
  // visible only on platforms where it's required (Windows, Linux); on
  // macOS, the system chrome handles it.
}

void HalcyonFrameView::OnThemeChanged() {
  views::NonClientFrameView::OnThemeChanged();
  SchedulePaint();
}

gfx::Size HalcyonFrameView::CalculatePreferredSize() const {
  // Size is determined by the client view; this view just adds the border.
  return gfx::Size();
}

void HalcyonFrameView::Layout() {
  views::NonClientFrameView::Layout();
  if (client_view()) {
    client_view()->SetBoundsRect(GetBoundsForClientView());
  }
}

}  // namespace halcyon
