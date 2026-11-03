// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/privacy_lantern/halcyon_lantern_indicator_view.h"

#include "base/bind.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/privacy_lantern/halcyon_lantern_controller.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/views/widget/widget.h"

namespace halcyon {

namespace {

constexpr int kLanternSize = 32;
constexpr float kLanternRadius = 9.0f;
constexpr float kLanternLineHalfWidth = 5.0f;
constexpr float kLanternLineStroke = 1.5f;
constexpr int kLanternPulsePeriodMs = ui::halcyon::kLanternPulsePeriodMs;

}  // namespace

HalcyonLanternIndicatorView::HalcyonLanternIndicatorView(
    Browser* browser, content::WebContents* web_contents)
    : browser_(browser), web_contents_(web_contents) {
  SetPreferredSize(gfx::Size(kLanternSize, kLanternSize));
  SetTooltipText(u"Lantern");
  SetAccessibleName(u"Lantern");
  SetFocusBehavior(views::View::FocusBehavior::ALWAYS);

  // Subscribe to the Lantern controller's observation updates.
  HalcyonLanternController::GetOrCreateForWebContents(web_contents)
      ->AddObserver(this);

  pulse_timer_.Start(
      FROM_HERE, base::Milliseconds(kLanternPulsePeriodMs / 16),
      base::BindRepeating(&HalcyonLanternIndicatorView::OnPulseTimer,
                           base::Unretained(this)));
}

HalcyonLanternIndicatorView::~HalcyonLanternIndicatorView() {
  if (web_contents_) {
    HalcyonLanternController::GetOrCreateForWebContents(web_contents_)
        ->RemoveObserver(this);
  }
}

void HalcyonLanternIndicatorView::SetObservationCount(int count) {
  observation_count_ = count;
  SchedulePaint();
}

void HalcyonLanternIndicatorView::SetQuietMode(bool quiet) {
  quiet_mode_ = quiet;
  SchedulePaint();
}

void HalcyonLanternIndicatorView::OnPulseTimer() {
  // Advance the pulse phase only if we're actively glowing.
  if (observation_count_ > 0 && !quiet_mode_) {
    pulse_phase_ += 1.0 / 16.0;  // 16 ticks per period
    if (pulse_phase_ >= 1.0) pulse_phase_ -= 1.0;
    SchedulePaint();
  } else {
    pulse_phase_ = 0.0;
  }
}

void HalcyonLanternIndicatorView::OnPaint(gfx::Canvas* canvas) {
  views::View::OnPaint(canvas);

  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  const bool glowing = (observation_count_ > 0) || quiet_mode_;

  // Compute the color.
  SkColor base_color = glowing ? ui::halcyon::LanternGlow(dark)
                                 : ui::halcyon::LanternDim(dark);
  SkColor color = base_color;
  if (glowing && !quiet_mode_ && observation_count_ > 0) {
    // Apply a soft pulse: alpha oscillates between 0.7 and 1.0 of the base color.
    double alpha = 0.7 + 0.3 * (0.5 + 0.5 * std::sin(pulse_phase_ * 2 * M_PI));
    color = SkColorSetA(base_color, static_cast<U8CPU>(alpha * 255));
  }

  // Optional hover border.
  if (hovered_) {
    cc::PaintFlags border_flags;
    border_flags.setAntiAlias(true);
    border_flags.setStyle(cc::PaintFlags::kStroke_Style);
    border_flags.setStrokeWidth(1.0f);
    border_flags.setColor(ui::halcyon::Border(dark));
    canvas->DrawCircle(
        gfx::Point(width() / 2, height() / 2),
        kLanternSize / 2 - 1, border_flags);
  }

  // The brand mark — a circle with a horizontal line through it.
  gfx::Point center(width() / 2, height() / 2);

  cc::PaintFlags circle_flags;
  circle_flags.setAntiAlias(true);
  circle_flags.setStyle(cc::PaintFlags::kStroke_Style);
  circle_flags.setStrokeWidth(kLanternLineStroke);
  circle_flags.setColor(color);
  canvas->DrawCircle(center, kLanternRadius, circle_flags);

  cc::PaintFlags line_flags;
  line_flags.setAntiAlias(true);
  line_flags.setStyle(cc::PaintFlags::kStroke_Style);
  line_flags.setStrokeCap(cc::PaintFlags::kRound_Cap);
  line_flags.setStrokeWidth(kLanternLineStroke);
  line_flags.setColor(color);
  // The line does not touch the circle; small gap on both sides.
  canvas->DrawLine(
      gfx::Point(center.x() - kLanternLineHalfWidth, center.y()),
      gfx::Point(center.x() + kLanternLineHalfWidth, center.y()),
      line_flags);
}

void HalcyonLanternIndicatorView::OnThemeChanged() {
  views::View::OnThemeChanged();
  SchedulePaint();
}

void HalcyonLanternIndicatorView::OnMouseEntered(const ui::MouseEvent& event) {
  hovered_ = true;
  SchedulePaint();
}

void HalcyonLanternIndicatorView::OnMouseExited(const ui::MouseEvent& event) {
  hovered_ = false;
  SchedulePaint();
}

bool HalcyonLanternIndicatorView::OnMousePressed(const ui::MouseEvent& event) {
  // Open the Lantern popup.
  HalcyonLanternController::ShowPopupForBrowser(browser_, this, web_contents_);
  return true;
}

}  // namespace halcyon
