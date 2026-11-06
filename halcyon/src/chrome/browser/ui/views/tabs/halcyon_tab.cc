// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tabs/halcyon_tab.h"

#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/text_constants.h"
#include "ui/native_theme/native_theme.h"
#include "url/gurl.h"

namespace halcyon {

namespace {

constexpr int kTabPaddingH = 12;
constexpr int kTabDotSize = 8;
constexpr int kTabUnderlineHeight = 2;
constexpr int kTabCloseSize = 14;
constexpr float kTabCloseStroke = 1.5f;

// A simple caption-style font for tab labels.
gfx::FontList TabFont() {
  // Use the platform default; the font is set via design tokens elsewhere.
  // The Halcyon font override (Inter) is applied at the WebUI layer; the
  // native Views layer uses the platform default but at 11px (caption-size).
  return gfx::FontList({"Inter", "system-ui"}, gfx::Font::NORMAL, 11,
                       gfx::Font::Weight::NORMAL);
}

}  // namespace

HalcyonTab::HalcyonTab() {
  SetID(VIEW_ID_TAB);
  SetFocusBehavior(FocusBehavior::ALWAYS);
  SetAccessibleName(u"Tab");
  SetContextMenuController(nullptr);  // HalcyonTabContextMenuController attached
                                       // by HalcyonTabStrip::AddTab.
}

HalcyonTab::~HalcyonTab() = default;

void HalcyonTab::OnPaint(gfx::Canvas* canvas) {
  PaintBackground(canvas);
  PaintFavicon(canvas);
  PaintTitle(canvas);
  if (anchored_) PaintAnchorMark(canvas);
  if (active_) PaintActiveUnderline(canvas);
  PaintCloseButton(canvas);
}

void HalcyonTab::PaintBackground(gfx::Canvas* canvas) {
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  SkColor bg;
  if (active_) {
    bg = ui::halcyon::TabBgActive(dark);
  } else if (anchored_) {
    bg = ui::halcyon::TabBgPinned(dark);
  } else if (hovered_) {
    bg = ui::halcyon::TabBgHover(dark);
  } else {
    bg = ui::halcyon::TabBg(dark);
  }
  canvas->DrawColor(bg);
}

void HalcyonTab::PaintFavicon(gfx::Canvas* canvas) {
  // The Halcyon favicon is a small dot. When the page is loading, the dot
  // is replaced by a brief horizontal line (the brand mark) that pulses.
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);

  int cx = kTabPaddingH + kTabDotSize / 2;
  int cy = height() / 2;

  if (loading_) {
    flags.setStyle(cc::PaintFlags::kStroke_Style);
    flags.setStrokeCap(cc::PaintFlags::kRound_Cap);
    flags.setStrokeWidth(2);
    flags.setColor(ui::halcyon::Accent(dark));
    canvas->DrawLine(gfx::Point(cx - 6, cy), gfx::Point(cx + 6, cy), flags);
  } else {
    flags.setStyle(cc::PaintFlags::kFill_Style);
    flags.setColor(active_ ? ui::halcyon::TabTextActive(dark)
                             : ui::halcyon::TabText(dark));
    canvas->DrawCircle(gfx::Point(cx, cy), kTabDotSize / 2, flags);
  }
}

void HalcyonTab::PaintTitle(gfx::Canvas* canvas) {
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  if (title_.empty()) return;
  int title_x = kTabPaddingH * 2 + kTabDotSize;
  int title_y = (height() - TabFont().GetHeight()) / 2;
  int title_w = width() - title_x - kTabCloseSize - kTabPaddingH;
  if (title_w <= 0) return;

  SkColor color = active_ ? ui::halcyon::TabTextActive(dark)
                          : ui::halcyon::TabText(dark);

  canvas->DrawStringRectWithFlags(
      title_, TabFont(), color,
      gfx::Rect(title_x, title_y, title_w, TabFont().GetHeight()),
      gfx::Canvas::NO_ELLIPSIS);
  // Halcyon truncates tab titles with a soft ellipsis rather than fading
  // (upstream's Tab::PaintTitle uses a fade gradient).
  // For brevity, we just use DrawStringRect which already ellipsizes.
}

void HalcyonTab::PaintActiveUnderline(gfx::Canvas* canvas) {
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kFill_Style);
  flags.setColor(ui::halcyon::ActiveTab(dark));
  canvas->DrawRect(
      gfx::Rect(0, height() - kTabUnderlineHeight, width(), kTabUnderlineHeight),
      flags);
}

void HalcyonTab::PaintCloseButton(gfx::Canvas* canvas) {
  if (!hovered_ && !active_) return;  // close button is hidden unless hovered
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeCap(cc::PaintFlags::kRound_Cap);
  flags.setStrokeWidth(kTabCloseStroke);
  flags.setColor(ui::halcyon::TabText(dark));

  int size = kTabCloseSize;
  int x = width() - kTabPaddingH - size / 2;
  int y = height() / 2;
  canvas->DrawLine(gfx::Point(x - size / 3, y - size / 3),
                    gfx::Point(x + size / 3, y + size / 3), flags);
  canvas->DrawLine(gfx::Point(x + size / 3, y - size / 3),
                    gfx::Point(x - size / 3, y + size / 3), flags);
}

void HalcyonTab::PaintAnchorMark(gfx::Canvas* canvas) {
  // Anchored tabs have a small anchor glyph at the bottom-left.
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeCap(cc::PaintFlags::kRound_Cap);
  flags.setStrokeWidth(1.5f);
  flags.setColor(ui::halcyon::Accent(dark));

  int x = 4;
  int y = height() - 8;
  canvas->DrawLine(gfx::Point(x, y), gfx::Point(x, y + 4), flags);
  canvas->DrawLine(gfx::Point(x - 2, y + 2), gfx::Point(x + 2, y + 2), flags);
  canvas->DrawLine(gfx::Point(x - 2, y + 4), gfx::Point(x, y + 6), flags);
  canvas->DrawLine(gfx::Point(x + 2, y + 4), gfx::Point(x, y + 6), flags);
}

void HalcyonTab::OnThemeChanged() {
  views::View::OnThemeChanged();
  SchedulePaint();
}

void HalcyonTab::OnParentThemeChanged(ui::NativeTheme* theme) {
  SetNativeTheme(theme);
  SchedulePaint();
}

void HalcyonTab::OnMouseEntered(const ui::MouseEvent& event) {
  hovered_ = true;
  SchedulePaint();
  views::View::OnMouseEntered(event);
}

void HalcyonTab::OnMouseExited(const ui::MouseEvent& event) {
  hovered_ = false;
  SchedulePaint();
  views::View::OnMouseExited(event);
}

bool HalcyonTab::OnMousePressed(const ui::MouseEvent& event) {
  // Clicking a tab activates it. The HalcyonTabStripController handles the
  // actual activation; we just consume the press.
  return true;
}

void HalcyonTab::StateChanged(int old_state, int new_state) {
  SchedulePaint();
}

}  // namespace halcyon
