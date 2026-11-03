// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/toolbar/halcyon_navigation_button.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/views/widget/widget.h"

namespace halcyon {

namespace {

// 16px wide × 16px tall icons. The button itself is 32px square.
constexpr int kNavButtonSize = 32;
constexpr int kNavIconSize = 16;
constexpr float kNavIconStroke = 1.5f;

}  // namespace

HalcyonNavigationButton::HalcyonNavigationButton(Type type, Browser* browser)
    : type_(type), browser_(browser) {
  SetPreferredSize(gfx::Size(kNavButtonSize, kNavButtonSize));
  SetFocusBehavior(views::View::FocusBehavior::ALWAYS);

  using ClickCB = base::RepeatingCallback<void(const ui::Event& event)>;
  ClickCB click_cb;
  switch (type_) {
    case kBack:
      SetTooltipText(u"Back");
      SetAccessibleName(u"Back");
      click_cb = base::BindRepeating(
          [](Browser* b, const ui::Event&) { chrome::Back(b); }, browser_);
      break;
    case kForward:
      SetTooltipText(u"Forward");
      SetAccessibleName(u"Forward");
      click_cb = base::BindRepeating(
          [](Browser* b, const ui::Event&) { chrome::Forward(b); }, browser_);
      break;
    case kReload:
      SetTooltipText(u"Reload");
      SetAccessibleName(u"Reload");
      click_cb = base::BindRepeating(
          [](Browser* b, const ui::Event&) { chrome::Reload(b); }, browser_);
      break;
  }
  SetCallback(click_cb);
}

HalcyonNavigationButton::~HalcyonNavigationButton() = default;

void HalcyonNavigationButton::OnThemeChanged() {
  views::ImageButton::OnThemeChanged();
  SchedulePaint();
}

void HalcyonNavigationButton::OnPaint(gfx::Canvas* canvas) {
  views::ImageButton::OnPaint(canvas);

  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeCap(cc::PaintFlags::kRound_Cap);
  flags.setStrokeJoin(cc::PaintFlags::kRound_Join);
  flags.setStrokeWidth(kNavIconStroke);
  flags.setColor(GetState() == views::Button::STATE_HOVERED
                     ? ui::halcyon::Text(dark)
                     : ui::halcyon::Text2(dark));

  gfx::Rect bounds = GetContentsBounds();
  int cx = bounds.x() + bounds.width() / 2;
  int cy = bounds.y() + bounds.height() / 2;
  int r = kNavIconSize / 2;

  // Translate the icon's coordinate system to (cx, cy) — the center.
  // The icons are drawn on a 24×24 grid in icons.svg, so we translate
  // by -12, -12 (the center) and scale.
  canvas->Save();
  canvas->Translate(gfx::Vector2dF(cx - 12, cy - 12));

  switch (type_) {
    case kBack:
      canvas->DrawLine(gfx::PointF(15, 6), gfx::PointF(9, 12), flags);
      canvas->DrawLine(gfx::PointF(9, 12), gfx::PointF(15, 18), flags);
      break;
    case kForward:
      canvas->DrawLine(gfx::PointF(9, 6), gfx::PointF(15, 12), flags);
      canvas->DrawLine(gfx::PointF(15, 12), gfx::PointF(9, 18), flags);
      break;
    case kReload: {
      // A circular arrow: top arc + two arrowheads at each end.
      cc::PaintFlags arc = flags;
      SkPath path;
      path.addArc(SkRect::MakeLTRB(4, 4, 20, 20), -90, 180);
      canvas->DrawPath(path, arc);
      canvas->DrawLine(gfx::PointF(20, 12), gfx::PointF(20, 6), flags);
      canvas->DrawLine(gfx::PointF(20, 12), gfx::PointF(14, 12), flags);
      canvas->DrawLine(gfx::PointF(4, 12), gfx::PointF(4, 18), flags);
      canvas->DrawLine(gfx::PointF(4, 12), gfx::PointF(10, 12), flags);
      break;
    }
  }
  canvas->Restore();
}

}  // namespace halcyon
