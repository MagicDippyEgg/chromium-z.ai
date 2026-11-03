// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/location_bar/halcyon_omnibox_view.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/omnibox/omnibox_view.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/gfx/canvas.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/layout/fill_layout.h"

namespace halcyon {

namespace {

constexpr int kOmniboxHeight = 40;
constexpr int kOmniboxPaddingH = 16;
constexpr int kOmniboxTextInsetH = 12;

}  // namespace

HalcyonOmniboxView::HalcyonOmniboxView(Browser* browser,
                                          content::WebContents* web_contents)
    : browser_(browser), web_contents_(web_contents) {
  SetID(VIEW_ID_OMNIBOX);

  SetLayoutManager(std::make_unique<views::FillLayout>());

  // Wrap the upstream OmniboxView textfield. We don't rewrite the upstream
  // autocomplete system; we just visually re-shell it. The upstream
  // OmniboxView is created via the OmniboxController; we attach it as a child
  // of our view, and the surrounding view draws the pill background and
  // border.
  textfield_ = AddChildView(
      std::make_unique<views::Textfield>());
  textfield_->SetTextInputType(ui::TEXT_INPUT_TYPE_URL);
  textfield_->SetPlaceholderText(u"Search or type a URL");
  textfield_->SetAccessibleName(u"Search or type a URL");
  textfield_->set_controller(this);  // HalcyonOmniboxView implements
                                       // views::TextfieldController
  textfield_->SetBackgroundColor(SK_ColorTRANSPARENT);
  textfield_->SetTextColor(ui::kColorLabelForeground,
                            ui::halcyon::Text(false));
  textfield_->SetBorder(views::NullBorder());
  textfield_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
}

HalcyonOmniboxView::~HalcyonOmniboxView() = default;

void HalcyonOmniboxView::OnThemeChanged() {
  views::View::OnThemeChanged();
  const bool dark = GetNativeTheme()->ShouldUseDarkColors();
  // Background and border are drawn in OnPaint; here we just refresh the
  // textfield's text color.
  textfield_->SetTextColor(ui::kColorLabelForeground,
                            ui::halcyon::Text(dark));
  textfield_->SetPlaceholderTextColor(ui::halcyon::Text2(dark));
  SchedulePaint();
}

gfx::Size HalcyonOmniboxView::CalculatePreferredSize() const {
  // The omnibox takes the available width; height is fixed.
  return gfx::Size(0, kOmniboxHeight);
}

void HalcyonOmniboxView::Layout() {
  views::View::Layout();
  // The textfield fills the interior with a small horizontal inset so the
  // text doesn't sit on the pill's edge.
  textfield_->SetBounds(
      kOmniboxTextInsetH, 0,
      width() - kOmniboxTextInsetH * 2, height());
}

void HalcyonOmniboxView::OnPaint(gfx::Canvas* canvas) {
  views::View::OnPaint(canvas);

  const bool dark = GetNativeTheme()->ShouldUseDarkColors();

  // Background: a slight "flashlight" effect when focused.
  SkColor bg = focused_ ? ui::halcyon::OmniboxBgFocus(dark)
                          : ui::halcyon::OmniboxBg(dark);
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kFill_Style);
  flags.setColor(bg);
  gfx::Rect bounds = GetContentsBounds();
  canvas->DrawRoundRect(bounds, ui::halcyon::kRadiusMd, flags);

  // Border: a hairline when unfocused, a thicker Sage when focused.
  cc::PaintFlags border_flags;
  border_flags.setAntiAlias(true);
  border_flags.setStyle(cc::PaintFlags::kStroke_Style);
  border_flags.setStrokeWidth(focused_ ? 1.5f : 1.0f);
  border_flags.setColor(focused_ ? ui::halcyon::OmniboxBorderFocus(dark)
                                    : ui::halcyon::OmniboxBorder(dark));
  canvas->DrawRoundRect(bounds, ui::halcyon::kRadiusMd, border_flags);

  // Optional: focus ring.
  if (focused_) {
    cc::PaintFlags ring_flags;
    ring_flags.setAntiAlias(true);
    ring_flags.setStyle(cc::PaintFlags::kStroke_Style);
    ring_flags.setStrokeWidth(1.0f);
    ring_flags.setColor(SkColorSetA(ui::halcyon::Accent(dark), 80));
    canvas->DrawRoundRect(
        gfx::Rect(bounds.x() - 2, bounds.y() - 2, bounds.width() + 4,
                  bounds.height() + 4),
        ui::halcyon::kRadiusMd + 2, ring_flags);
  }
}

void HalcyonOmniboxView::OnFocus() {
  focused_ = true;
  textfield_->RequestFocus();
  SchedulePaint();
}

void HalcyonOmniboxView::OnBlur() {
  focused_ = false;
  SchedulePaint();
}

void HalcyonOmniboxView::Revert() {
  // Restore the omnibox text to the current page URL.
  if (web_contents_) {
    textfield_->SetText(base::UTF8ToUTF16(web_contents_->GetURL().spec()));
  }
}

}  // namespace halcyon
