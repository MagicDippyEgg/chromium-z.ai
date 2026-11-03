// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_PRIVACY_LANTERN_HALCYON_LANTERN_INDICATOR_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_PRIVACY_LANTERN_HALCYON_LANTERN_INDICATOR_VIEW_H_

#include "base/timer/timer.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

class Browser;

namespace content {
class WebContents;
}

namespace halcyon {

// HalcyonLanternIndicatorView — the Lantern indicator in the toolbar.
//
// The Lantern is a small circular indicator at the right edge of the toolbar
// that glows softly when the current site is doing something the user might
// want to know about (third-party cookies, persistent storage, fingerprinting
// APIs, permission requests). Clicking it opens the Lantern popup (a small
// WebUI bubble) that lists the observations in plain language.
//
// Visual properties:
//   - When the Lantern has nothing to report: a dim circle with a horizontal
//     line through it. (The brand mark, in a muted color.)
//   - When the Lantern has something to report: the same mark in the Sage
//     accent color, with a soft 1.6s pulse animation. The pulse is the only
//     decorative animation in Halcyon; it is permitted because the Lantern's
//     entire purpose is to be quietly informative.
//   - When Quiet Mode is on: the Lantern is fully Sage (a calmer "all good").
//   - On hover: a small border appears.
//   - On focus: a focus ring.
//
// The Lantern is created by HalcyonToolbarView::Init() and is owned by the
// toolbar.

class HalcyonLanternIndicatorView : public views::View {
  METADATA_HEADER(HalcyonLanternIndicatorView, views::View)

 public:
  HalcyonLanternIndicatorView(Browser* browser, content::WebContents* web_contents);
  ~HalcyonLanternIndicatorView() override;

  // views::View:
  void OnPaint(gfx::Canvas* canvas) override;
  void OnThemeChanged() override;
  void OnMouseEntered(const ui::MouseEvent& event) override;
  void OnMouseExited(const ui::MouseEvent& event) override;
  bool OnMousePressed(const ui::MouseEvent& event) override;

  // Called by the HalcyonLanternController when the observations for the
  // current page change. The indicator's visual state updates.
  void SetObservationCount(int count);

  // Called when Quiet Mode is toggled. When Quiet Mode is on, the Lantern
  // is fully Sage regardless of observations.
  void SetQuietMode(bool quiet);

 private:
  void OnPulseTimer();

  raw_ptr<Browser> browser_;
  raw_ptr<content::WebContents> web_contents_;

  // Number of observations the Lantern has for the current page.
  int observation_count_ = 0;
  bool quiet_mode_ = false;
  bool hovered_ = false;

  // Pulse animation (only when observation_count_ > 0 and not quiet_mode_).
  base::RepeatingTimer pulse_timer_;
  double pulse_phase_ = 0.0;
};

}  // namespace halcyon

#endif  // CHROME_BROWSER_UI_VIEWS_PRIVACY_LANTERN_HALCYON_LANTERN_INDICATOR_VIEW_H_
