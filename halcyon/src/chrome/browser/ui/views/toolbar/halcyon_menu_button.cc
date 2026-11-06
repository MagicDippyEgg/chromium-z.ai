// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/toolbar/halcyon_menu_button.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/halcyon_menu_controller.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/widget/widget.h"

namespace halcyon {

namespace {

constexpr int kMenuButtonSize = 32;
constexpr int kMenuButtonLineStroke = 2;  // slightly thicker than the icon stroke
constexpr int kMenuButtonLineHalfWidth = 10;  // the line is 20px wide

}  // namespace

HalcyonMenuButton::HalcyonMenuButton(Browser* browser)
    : views::Button(base::BindRepeating(&HalcyonMenuButton::OnClick,
                                         base::Unretained(this))),
      browser_(browser) {
  SetPreferredSize(gfx::Size(kMenuButtonSize, kMenuButtonSize));
  SetTooltipText(u"Menu");
  SetAccessibleName(u"Menu");
  SetFocusBehavior(views::View::FocusBehavior::ALWAYS);
  SetCornerRadius(ui::halcyon::kRadiusMd);
}

HalcyonMenuButton::~HalcyonMenuButton() = default;

void HalcyonMenuButton::OnThemeChanged() {
  views::Button::OnThemeChanged();
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  SetBackground(views::CreateThemedSolidBackground(
      ui::halcyon::Surface(dark), ui::halcyon::Surface2(dark)));
  SetBorder(views::CreateSolidSidedBorder(
      1, 1, 1, 1, ui::halcyon::Border(dark)));
  SchedulePaint();
}

void HalcyonMenuButton::OnClick() {
  // Open the Halcyon menu anchored to this button.
  HalcyonMenuController::ShowForBrowser(browser_, this);
}

void HalcyonMenuButton::OnPaint(gfx::Canvas* canvas) {
  views::Button::OnPaint(canvas);

  // Draw a single horizontal line centered in the button.
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeCap(cc::PaintFlags::kRound_Cap);
  flags.setStrokeWidth(kMenuButtonLineStroke);
  flags.setColor(GetState() == views::Button::STATE_HOVERED
                     ? ui::halcyon::Text(dark)
                     : ui::halcyon::Text2(dark));

  gfx::Rect bounds = GetContentsBounds();
  int cx = bounds.x() + bounds.width() / 2;
  int cy = bounds.y() + bounds.height() / 2;
  canvas->DrawLine(
      gfx::Point(cx - kMenuButtonLineHalfWidth, cy),
      gfx::Point(cx + kMenuButtonLineHalfWidth, cy),
      flags);
}

}  // namespace halcyon
