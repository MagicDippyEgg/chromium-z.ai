# Halcyon — Color System

This is the implementation-facing color reference. The brand-level intent is in `brand_book.md §7`. The exact values here are the ones used by every Halcyon surface, native or WebUI. **Do not invent new colors.** If you need a new color, add it here first.

## Token names

Token names follow the pattern `<role>-<state>` in light mode, with the dark-mode variant having the same name suffixed `--dark`. The WebUI CSS exposes every token as a CSS custom property under `--halcyon-<name>`. The C++ `HalcyonTheme` exposes every token as a `SkColor` constant under `kHalcyon<Role>`.

## Light — "Vellum"

```css
:root {
  /* Page & surfaces */
  --halcyon-bg:           #FAF8F3;  /* Vellum — page background */
  --halcyon-surface:      #F0EEE8;  /* Mist — toolbar, tab bg, card */
  --halcyon-surface-2:    #E8E5DC;  /* Linen — hover/active surfaces */
  --halcyon-surface-3:    #DEDBCF;  /* Stone — pressed */
  --halcyon-border:       #D8D4C8;  /* Thready — hairlines */
  --halcyon-border-soft:  #E4E0D2;  /* Faint — subtle dividers */

  /* Text */
  --halcyon-text:         #2B2F38;  /* Haze — body */
  --halcyon-text-2:       #6B6F78;  /* Drift — secondary */
  --halcyon-text-3:       #A8A8A0;  /* Pale — disabled */

  /* Brand & accent */
  --halcyon-accent:       #7A9B76;  /* Sage */
  --halcyon-accent-press: #5F7D5C;  /* Pine */
  --halcyon-accent-soft:  #EAEEE6;  /* Sage wash — backgrounds with sage */
  --halcyon-accent-text:  #2F4A2C;  /* Sage text (for on-accent-soft surfaces) */

  /* Link & active */
  --halcyon-link:         #5B7C99;  /* Tide */
  --halcyon-link-press:   #45617B;
  --halcyon-active-tab:   #5B7C99;  /* Tide — the tab underline */

  /* States (used sparingly) */
  --halcyon-caution:      #C4855C;  /* Ember */
  --halcyon-caution-soft: #F2E3D2;
  --halcyon-danger:       #B05945;  /* Coral */
  --halcyon-danger-soft:  #EFD6CE;
  --halcyon-success:      #6B8A68;  /* Sage, slightly darker */
  --halcyon-success-soft: #E4ECDE;

  /* Omnibox */
  --halcyon-omnibox-bg:        #F0EEE8;  /* Mist */
  --halcyon-omnibox-bg-focus:  #FFFFFF;
  --halcyon-omnibox-border:   #D8D4C8;
  --halcyon-omnibox-border-focus: #7A9B76;

  /* Tab strip */
  --halcyon-tab-bg:           #E4E0D2;  /* inactive tab — recedes */
  --halcyon-tab-bg-hover:     #ECE9DD;
  --halcyon-tab-bg-active:    #FAF8F3;  /* matches page bg */
  --halcyon-tab-bg-pinned:   #DEDBCF;  /* anchored tabs */
  --halcyon-tab-text:        #6B6F78;
  --halcyon-tab-text-active: #2B2F38;

  /* Toolbar */
  --halcyon-toolbar-bg:      #FAF8F3;  /* same as page — floats above */
  --halcyon-toolbar-border:  #E4E0D2;

  /* Menu & popovers */
  --halcyon-menu-bg:         #FAF8F3;
  --halcyon-menu-item-hover:#E8E5DC;
  --halcyon-menu-item-active:#DEDBCF;

  /* Lantern */
  --halcyon-lantern-dim:     #C9C5B7;  /* nothing to report */
  --halcyon-lantern-glow:    #7A9B76;  /* something to say */

  /* Quill / Cove / margin notes surfaces */
  --halcyon-cove-bg:         #1A1D24;
  --halcyon-cove-text:       #E4E2DC;
}
```

## Dark — "Eclipse"

```css
:root[data-halcyon-theme="dark"] {
  /* Page & surfaces */
  --halcyon-bg:           #1A1D24;  /* Eclipse */
  --halcyon-surface:      #22262F;  /* Twilight */
  --halcyon-surface-2:    #2B2F39;  /* Dusk */
  --halcyon-surface-3:    #343945;  /* Pressed */
  --halcyon-border:       #3A3F4B;  /* Cindery */
  --halcyon-border-soft:  #2F343F;

  /* Text */
  --halcyon-text:         #E4E2DC;  /* Frost */
  --halcyon-text-2:       #9A9CA0;  /* Mistlight */
  --halcyon-text-3:       #6B6D70;  /* Ashen */

  /* Brand & accent — brightened for dark */
  --halcyon-accent:       #8FB389;
  --halcyon-accent-press: #7BA075;
  --halcyon-accent-soft:  #2A3627;
  --halcyon-accent-text:  #B7D5B2;

  /* Link & active */
  --halcyon-link:         #7FA4C7;
  --halcyon-link-press:   #6B8DAE;
  --halcyon-active-tab:   #7FA4C7;

  /* States */
  --halcyon-caution:      #D9A276;
  --halcyon-caution-soft: #3A2F23;
  --halcyon-danger:       #D17863;
  --halcyon-danger-soft:  #3A2620;
  --halcyon-success:      #8FB389;
  --halcyon-success-soft: #2A3627;

  /* Omnibox */
  --halcyon-omnibox-bg:        #22262F;
  --halcyon-omnibox-bg-focus:  #1A1D24;
  --halcyon-omnibox-border:   #3A3F4B;
  --halcyon-omnibox-border-focus: #8FB389;

  /* Tab strip */
  --halcyon-tab-bg:           #1A1D24;  /* inactive — recedes */
  --halcyon-tab-bg-hover:     #22262F;
  --halcyon-tab-bg-active:    #2B2F39;
  --halcyon-tab-bg-pinned:    #343945;
  --halcyon-tab-text:         #9A9CA0;
  --halcyon-tab-text-active:  #E4E2DC;

  /* Toolbar */
  --halcyon-toolbar-bg:      #1A1D24;
  --halcyon-toolbar-border:  #2F343F;

  /* Menu */
  --halcyon-menu-bg:         #22262F;
  --halcyon-menu-item-hover: #2B2F39;
  --halcyon-menu-item-active:#343945;

  /* Lantern */
  --halcyon-lantern-dim:     #4A4F5B;
  --halcyon-lantern-glow:   #8FB389;

  /* Cove */
  --halcyon-cove-bg:         #14161B;  /* slightly darker than Eclipse */
  --halcyon-cove-text:       #E4E2DC;
}
```

## High-contrast mode

For users who set their OS to high-contrast, Halcyon uses:

- Page background: pure black (`#000000`) in dark HC, pure white in light HC.
- Text: pure white / pure black.
- Accent: Sage at full saturation, but with a 1px black (or white) outline on every interactive element.
- No surface color distinction — only borders.

This is achieved by replacing the tokens with HC values via `prefers-contrast: more` and `forced-colors: active`.

## Accessibility

All Halcyon text and UI pairings meet **WCAG AA** (4.5:1 for body text, 3:1 for large text and UI components). Most pairings approach AAA.

| Foreground | Background        | Ratio (light) | Ratio (dark) |
|------------|-------------------|---------------|--------------|
| Haze       | Vellum            | 11.6:1        | —            |
| Frost      | Eclipse           | —             | 12.4:1       |
| Drift      | Vellum            | 4.7:1         | —            |
| Mistlight  | Eclipse           | —             | 7.2:1        |
| Sage       | Vellum            | 3.1:1 (UI)    | —            |
| Sage (dark)| Eclipse           | —             | 5.8:1        |
| Tide       | Vellum            | 4.8:1         | —            |
| Tide (dark)| Eclipse           | —             | 6.4:1        |
| Coral      | Vellum            | 4.6:1         | —            |
| Coral (dk) | Eclipse           | —             | 5.4:1        |

Sage on Vellum is 3.1:1 — below AA for body text. **Sage is therefore never used for body text.** It is used for UI accents, focus rings, the brand mark, and primary-action buttons (where the text on the button is Haze or Frost, not Sage on Vellum). This is verified by the accessibility check in `halcyon/design/verify_contrast.py`.

## Where each token is used — quick map

| Token              | Used by |
|--------------------|---------|
| `bg`               | WebUI page background, native window background. |
| `surface`          | Toolbar, omnibox (unfocused), tab (hover), menu background, dialog. |
| `surface-2`        | Active/hover states on cards and menu items. |
| `surface-3`        | Pressed states. |
| `border`           | Hairlines around dialogs, menus, the omnibox. |
| `border-soft`      | Dividers inside panels. |
| `text`             | Body text everywhere. |
| `text-2`           | Captions, tab labels (inactive), metadata. |
| `text-3`           | Disabled text. |
| `accent`           | Focus rings, primary buttons' backgrounds, the Lantern glow, the brand mark, active settings section. |
| `accent-press`     | Pressed state on primary buttons. |
| `accent-soft`      | Selected-row backgrounds (Tides, Cairn, Settings list). |
| `link`             | WebUI links, the active tab underline. |
| `caution`          | "Site is doing something you should know about but it's not dangerous." |
| `danger`           | "Delete this forever", "This file is malicious." |
| `success`          | "Download finished", "Saved." |
| `omnibox-bg`       | Omnibox background, unopened. |
| `omnibox-bg-focus` | Omnibox background, focused — slightly lighter, like a flashlight turning on. |
| `tab-bg`           | Inactive tab — recedes. |
| `tab-bg-active`    | Active tab — matches the page background, so the active tab visually *is* the page. |
| `tab-bg-pinned`    | Anchored tabs — visually distinct without being decorative. |
| `lantern-dim`      | Default Lantern state. |
| `lantern-glow`     | Lantern when there's something to say. |
| `cove-bg`          | The Cove (private mode) — slightly darker than Eclipse. |

## Forbidden color combinations

- **Sage on Vellum as text.** Use Sage as an accent only.
- **Tide as a button background.** Tide is for links and tab indicators only.
- **Coral and Ember in the same view.** Pick one warning color per view.
- **Pure white (`#FFFFFF`) as a background in light mode.** Use Vellum. (Pure white is permitted *only* as the focused-omnibox background, where it functions as a flashlight.)
- **Pure black (`#000000`) as a background in dark mode.** Use Eclipse.
- **Any color from upstream Chrome's palette.** (Upstream `--google-blue-500`, `--google-red-600`, etc., are removed from the Halcyon token set entirely. Where upstream code references them, Halcyon overrides them — see `chrome/browser/themes/halcyon_theme_overrides.cc`.)
