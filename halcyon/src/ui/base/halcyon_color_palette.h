// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_HALCYON_COLOR_PALETTE_H_
#define UI_BASE_HALCYON_COLOR_PALETTE_H_

#include "third_party/skia/include/core/SkColor.h"

// Halcyon color constants — the C++ side of the design tokens defined in
// src/halcyon/design/design_tokens.css.
//
// When you change a value here, also update:
//   - design_tokens.css
//   - src/halcyon/branding/color_system.md
// And run halcyon/design/verify_tokens.py to confirm they match.
//
// All values are 8-bit RGB; the alpha channel is encoded in the SkColor.

namespace ui {

namespace halcyon {

// ===== Light theme — "Vellum" =====
constexpr SkColor kBg            = SkColorSetRGB(0xFA, 0xF8, 0xF3);
constexpr SkColor kSurface       = SkColorSetRGB(0xF0, 0xEE, 0xE8);
constexpr SkColor kSurface2       = SkColorSetRGB(0xE8, 0xE5, 0xDC);
constexpr SkColor kSurface3       = SkColorSetRGB(0xDE, 0xDB, 0xCF);
constexpr SkColor kBorder         = SkColorSetRGB(0xD8, 0xD4, 0xC8);
constexpr SkColor kBorderSoft     = SkColorSetRGB(0xE4, 0xE0, 0xD2);

constexpr SkColor kText           = SkColorSetRGB(0x2B, 0x2F, 0x38);
constexpr SkColor kText2          = SkColorSetRGB(0x6B, 0x6F, 0x78);
constexpr SkColor kText3          = SkColorSetRGB(0xA8, 0xA8, 0xA0);

constexpr SkColor kAccent         = SkColorSetRGB(0x7A, 0x9B, 0x76);
constexpr SkColor kAccentPress     = SkColorSetRGB(0x5F, 0x7D, 0x5C);
constexpr SkColor kAccentSoft     = SkColorSetRGB(0xEA, 0xEE, 0xE6);
constexpr SkColor kAccentText      = SkColorSetRGB(0x2F, 0x4A, 0x2C);

constexpr SkColor kLink           = SkColorSetRGB(0x5B, 0x7C, 0x99);
constexpr SkColor kLinkPress      = SkColorSetRGB(0x45, 0x61, 0x7B);
constexpr SkColor kActiveTab       = kLink;

constexpr SkColor kCaution         = SkColorSetRGB(0xC4, 0x85, 0x5C);
constexpr SkColor kCautionSoft     = SkColorSetRGB(0xF2, 0xE3, 0xD2);
constexpr SkColor kDanger          = SkColorSetRGB(0xB0, 0x59, 0x45);
constexpr SkColor kDangerSoft      = SkColorSetRGB(0xEF, 0xD6, 0xCE);
constexpr SkColor kSuccess         = SkColorSetRGB(0x6B, 0x8A, 0x68);
constexpr SkColor kSuccessSoft     = SkColorSetRGB(0xE4, 0xEC, 0xDE);

constexpr SkColor kOmniboxBg           = kSurface;
constexpr SkColor kOmniboxBgFocus      = SkColorSetRGB(0xFF, 0xFF, 0xFF);
constexpr SkColor kOmniboxBorder       = kBorder;
constexpr SkColor kOmniboxBorderFocus  = kAccent;

constexpr SkColor kTabBg             = SkColorSetRGB(0xE4, 0xE0, 0xD2);
constexpr SkColor kTabBgHover        = SkColorSetRGB(0xEC, 0xE9, 0xDD);
constexpr SkColor kTabBgActive       = kBg;  // matches the page so the active tab visually IS the page
constexpr SkColor kTabBgPinned        = SkColorSetRGB(0xDE, 0xDB, 0xCF);
constexpr SkColor kTabText           = kText2;
constexpr SkColor kTabTextActive     = kText;

constexpr SkColor kToolbarBg          = kBg;
constexpr SkColor kToolbarBorder      = SkColorSetRGB(0xE4, 0xE0, 0xD2);

constexpr SkColor kMenuBg              = kBg;
constexpr SkColor kMenuItemHover       = SkColorSetRGB(0xE8, 0xE5, 0xDC);
constexpr SkColor kMenuItemActive      = kSurface3;

constexpr SkColor kLanternDim          = SkColorSetRGB(0xC9, 0xC5, 0xB7);
constexpr SkColor kLanternGlow         = kAccent;

constexpr SkColor kCoveBg              = SkColorSetRGB(0x1A, 0x1D, 0x24);
constexpr SkColor kCoveText            = SkColorSetRGB(0xE4, 0xE2, 0xDC);

// ===== Dark theme — "Eclipse" =====
namespace dark {
constexpr SkColor kBg              = SkColorSetRGB(0x1A, 0x1D, 0x24);
constexpr SkColor kSurface          = SkColorSetRGB(0x22, 0x26, 0x2F);
constexpr SkColor kSurface2         = SkColorSetRGB(0x2B, 0x2F, 0x39);
constexpr SkColor kSurface3         = SkColorSetRGB(0x34, 0x39, 0x45);
constexpr SkColor kBorder           = SkColorSetRGB(0x3A, 0x3F, 0x4B);
constexpr SkColor kBorderSoft       = SkColorSetRGB(0x2F, 0x34, 0x3F);

constexpr SkColor kText             = SkColorSetRGB(0xE4, 0xE2, 0xDC);
constexpr SkColor kText2            = SkColorSetRGB(0x9A, 0x9C, 0xA0);
constexpr SkColor kText3            = SkColorSetRGB(0x6B, 0x6D, 0x70);

constexpr SkColor kAccent           = SkColorSetRGB(0x8F, 0xB3, 0x89);
constexpr SkColor kAccentPress      = SkColorSetRGB(0x7B, 0xA0, 0x75);
constexpr SkColor kAccentSoft       = SkColorSetRGB(0x2A, 0x36, 0x27);
constexpr SkColor kAccentText       = SkColorSetRGB(0xB7, 0xD5, 0xB2);

constexpr SkColor kLink             = SkColorSetRGB(0x7F, 0xA4, 0xC7);
constexpr SkColor kLinkPress        = SkColorSetRGB(0x6B, 0x8D, 0xAE);
constexpr SkColor kActiveTab        = kLink;

constexpr SkColor kCaution           = SkColorSetRGB(0xD9, 0xA2, 0x76);
constexpr SkColor kCautionSoft       = SkColorSetRGB(0x3A, 0x2F, 0x23);
constexpr SkColor kDanger            = SkColorSetRGB(0xD1, 0x78, 0x63);
constexpr SkColor kDangerSoft        = SkColorSetRGB(0x3A, 0x26, 0x20);
constexpr SkColor kSuccess           = SkColorSetRGB(0x8F, 0xB3, 0x89);
constexpr SkColor kSuccessSoft       = SkColorSetRGB(0x2A, 0x36, 0x27);

constexpr SkColor kOmniboxBg           = kSurface;
constexpr SkColor kOmniboxBgFocus      = kBg;
constexpr SkColor kOmniboxBorder       = kBorder;
constexpr SkColor kOmniboxBorderFocus  = kAccent;

constexpr SkColor kTabBg              = kBg;
constexpr SkColor kTabBgHover         = kSurface;
constexpr SkColor kTabBgActive       = kSurface2;
constexpr SkColor kTabBgPinned        = kSurface3;
constexpr SkColor kTabText            = kText2;
constexpr SkColor kTabTextActive      = kText;

constexpr SkColor kToolbarBg          = kBg;
constexpr SkColor kToolbarBorder      = SkColorSetRGB(0x2F, 0x34, 0x3F);

constexpr SkColor kMenuBg              = kSurface;
constexpr SkColor kMenuItemHover       = kSurface2;
constexpr SkColor kMenuItemActive      = kSurface3;

constexpr SkColor kLanternDim          = SkColorSetRGB(0x4A, 0x4F, 0x5B);
constexpr SkColor kLanternGlow         = kAccent;

constexpr SkColor kCoveBg              = SkColorSetRGB(0x14, 0x16, 0x1B);
constexpr SkColor kCoveText            = kText;
}  // namespace dark

// ===== Helpers =====

// Returns the right color for the given light/dark variant.
inline SkColor Accent(bool is_dark)         { return is_dark ? dark::kAccent : kAccent; }
inline SkColor AccentPress(bool is_dark)    { return is_dark ? dark::kAccentPress : kAccentPress; }
inline SkColor AccentSoft(bool is_dark)     { return is_dark ? dark::kAccentSoft : kAccentSoft; }
inline SkColor Text(bool is_dark)           { return is_dark ? dark::kText : kText; }
inline SkColor Text2(bool is_dark)          { return is_dark ? dark::kText2 : kText2; }
inline SkColor Text3(bool is_dark)          { return is_dark ? dark::kText3 : kText3; }
inline SkColor Bg(bool is_dark)             { return is_dark ? dark::kBg : kBg; }
inline SkColor Surface(bool is_dark)        { return is_dark ? dark::kSurface : kSurface; }
inline SkColor Surface2(bool is_dark)       { return is_dark ? dark::kSurface2 : kSurface2; }
inline SkColor Surface3(bool is_dark)       { return is_dark ? dark::kSurface3 : kSurface3; }
inline SkColor Border(bool is_dark)         { return is_dark ? dark::kBorder : kBorder; }
inline SkColor BorderSoft(bool is_dark)     { return is_dark ? dark::kBorderSoft : kBorderSoft; }
inline SkColor Link(bool is_dark)           { return is_dark ? dark::kLink : kLink; }
inline SkColor Danger(bool is_dark)         { return is_dark ? dark::kDanger : kDanger; }
inline SkColor Caution(bool is_dark)        { return is_dark ? dark::kCaution : kCaution; }
inline SkColor OmniboxBg(bool is_dark)       { return is_dark ? dark::kOmniboxBg : kOmniboxBg; }
inline SkColor OmniboxBgFocus(bool is_dark)  { return is_dark ? dark::kOmniboxBgFocus : kOmniboxBgFocus; }
inline SkColor OmniboxBorder(bool is_dark)   { return is_dark ? dark::kOmniboxBorder : kOmniboxBorder; }
inline SkColor OmniboxBorderFocus(bool is_dark) { return is_dark ? dark::kOmniboxBorderFocus : kOmniboxBorderFocus; }
inline SkColor TabBg(bool is_dark)            { return is_dark ? dark::kTabBg : kTabBg; }
inline SkColor TabBgHover(bool is_dark)       { return is_dark ? dark::kTabBgHover : kTabBgHover; }
inline SkColor TabBgActive(bool is_dark)      { return is_dark ? dark::kTabBgActive : kTabBgActive; }
inline SkColor TabBgPinned(bool is_dark)      { return is_dark ? dark::kTabBgPinned : kTabBgPinned; }
inline SkColor TabText(bool is_dark)          { return is_dark ? dark::kTabText : kTabText; }
inline SkColor TabTextActive(bool is_dark)    { return is_dark ? dark::kTabTextActive : kTabTextActive; }
inline SkColor ToolbarBg(bool is_dark)        { return is_dark ? dark::kToolbarBg : kToolbarBg; }
inline SkColor ToolbarBorder(bool is_dark)    { return is_dark ? dark::kToolbarBorder : kToolbarBorder; }
inline SkColor MenuBg(bool is_dark)           { return is_dark ? dark::kMenuBg : kMenuBg; }
inline SkColor MenuItemHover(bool is_dark)    { return is_dark ? dark::kMenuItemHover : kMenuItemHover; }
inline SkColor MenuItemActive(bool is_dark)   { return is_dark ? dark::kMenuItemActive : kMenuItemActive; }
inline SkColor LanternDim(bool is_dark)       { return is_dark ? dark::kLanternDim : kLanternDim; }
inline SkColor LanternGlow(bool is_dark)      { return is_dark ? dark::kLanternGlow : kLanternGlow; }
inline SkColor CoveBg(bool is_dark)           { return is_dark ? dark::kCoveBg : kCoveBg; }
inline SkColor CoveText(bool is_dark)         { return is_dark ? dark::kCoveText : kCoveText; }

// Halcyon's only radii.
constexpr float kRadiusSm = 4.0f;
constexpr float kRadiusMd = 6.0f;
constexpr float kRadiusLg = 12.0f;

// Halcyon's UI body font size (in dips). Slightly larger than upstream
// Chrome's 12px default.
constexpr int kUiFontSize = 13;

// The Lantern indicator's pulse period (in ms). Only animates when the
// Lantern has something to report.
constexpr int kLanternPulsePeriodMs = 1600;

}  // namespace halcyon
}  // namespace ui

#endif  // UI_BASE_HALCYON_COLOR_PALETTE_H_
