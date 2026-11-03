// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/themes/halcyon_theme.h"

#include "base/no_destructor.h"
#include "chrome/browser/themes/theme_service.h"
#include "ui/base/halcyon_color_palette.h"
#include "ui/native_theme/native_theme.h"

namespace halcyon {

// HalcyonTheme — see halcyon_theme.h for the design intent.
//
// Implementation note: this class subclasses upstream's ThemeService. For
// the design project, only GetColor() is implemented with the real
// Halcyon palette. The other ThemeService methods (UsingExtensionTheme,
// GetThemeID, etc.) are stubbed out — they would be implemented for a full
// build. See chrome/browser/themes/theme_service.h for the upstream
// interface.

HalcyonTheme::HalcyonTheme() = default;
HalcyonTheme::~HalcyonTheme() = default;

bool HalcyonTheme::ShouldUseDarkColors() const {
  if (forced_dark_) return true;
  if (forced_light_) return false;
  return ui::NativeTheme::GetInstanceForNativeUi()->ShouldUseDarkColors();
}

SkColor HalcyonTheme::GetColor(int color_id, bool use_dark) const {
  using namespace ui::halcyon;
  switch (color_id) {
    // Window / page background
    case TP_COLOR_WINDOW_BACKGROUND:
    case TP_COLOR_FRAME:
    case TP_COLOR_FRAME_INACTIVE:
      return use_dark ? dark::kBg : kBg;

    // Toolbar
    case TP_COLOR_TOOLBAR:
    case TP_COLOR_TOOLBAR_TOP_SEPARATOR:
      return ToolbarBg(use_dark);

    // Tab strip
    case TP_COLOR_TAB_BACKGROUND_INACTIVE:
      return TabBg(use_dark);
    case TP_COLOR_TAB_BACKGROUND_ACTIVE:
      return TabBgActive(use_dark);
    case TP_COLOR_TAB_FOREGROUND_INACTIVE:
      return TabText(use_dark);
    case TP_COLOR_TAB_FOREGROUND_ACTIVE:
      return TabTextActive(use_dark);
    case TP_COLOR_TAB_HOVER_BACKGROUND:
      return TabBgHover(use_dark);

    // Omnibox
    case TP_COLOR_OMNIBOX_BACKGROUND:
      return OmniboxBg(use_dark);
    case TP_COLOR_OMNIBOX_BACKGROUND_HOVER:
      return OmniboxBg(use_dark);
    case TP_COLOR_OMNIBOX_BACKGROUND_FOCUSED:
      return OmniboxBgFocus(use_dark);
    case TP_COLOR_OMNIBOX_TEXT:
      return Text(use_dark);
    case TP_COLOR_OMNIBOX_BORDER:
      return OmniboxBorder(use_dark);
    case TP_COLOR_OMNIBOX_BORDER_FOCUSED:
      return OmniboxBorderFocus(use_dark);

    // Text colors (used in various chrome contexts)
    case TP_COLOR_BOOKMARK_TEXT:
    case TP_COLOR_LOCATION_BAR_TEXT:
    case TP_COLOR_LOCATION_BAR_TEXT_DIMMED:
      return Text2(use_dark);
    case TP_COLOR_LOCATION_BAR_TEXT_DEFAULT:
    case TP_COLOR_TOOLBAR_TEXT:
      return Text(use_dark);

    // Buttons / accents
    case TP_COLOR_TOOLBAR_BUTTON_ICON:
      return Text2(use_dark);
    case TP_COLOR_TOOLBAR_BUTTON_ICON_HOVERED:
    case TP_COLOR_TOOLBAR_BUTTON_ICON_PRESSED:
      return Text(use_dark);
    case TP_COLOR_TOOLBAR_BUTTON_BACKGROUND:
      return Surface(use_dark);
    case TP_COLOR_TOOLBAR_BUTTON_BACKGROUND_HOVERED:
      return Surface2(use_dark);
    case TP_COLOR_TOOLBAR_BUTTON_BACKGROUND_PRESSED:
      return Surface3(use_dark);

    // Menu
    case TP_COLOR_MENU_BACKGROUND:
      return MenuBg(use_dark);
    case TP_COLOR_MENU_ITEM_BACKGROUND_HOVERED:
      return MenuItemHover(use_dark);
    case TP_COLOR_MENU_ITEM_BACKGROUND_SELECTED:
      return MenuItemActive(use_dark);
    case TP_COLOR_MENU_TEXT:
      return Text(use_dark);
    case TP_COLOR_MENU_SEPARATOR:
      return BorderSoft(use_dark);

    // Bookmarks bar (not used by Halcyon; Halcyon has no bookmarks bar)
    case TP_COLOR_BOOKMARK_BAR:
    case TP_COLOR_BOOKMARK_BAR_BACKGROUND:
      return ToolbarBg(use_dark);

    // The Lantern indicator (Halcyon-specific color_id values defined in
    // halcyon_color_palette.h)
    case TP_COLOR_HALCYON_LANTERN_DIM:
      return LanternDim(use_dark);
    case TP_COLOR_HALCYON_LANTERN_GLOW:
      return LanternGlow(use_dark);

    default:
      // Fall back to a neutral surface for unknown color_ids.
      return Surface(use_dark);
  }
}

// static
HalcyonTheme& HalcyonTheme::Get() {
  static base::NoDestructor<HalcyonTheme> instance;
  return *instance;
}

}  // namespace halcyon
