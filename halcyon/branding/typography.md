# Halcyon — Typography

## Fonts

### Inter (UI body)
- **Vendor:** Rasmus Andersson, licensed under SIL OFL 1.1.
- **Used at:** 13px / 18px (body), 14px / 20px (subtitle), 11px / 14px (caption).
- **Weights shipped:** 400, 500, 600.
- **Bundled in:** `chrome/browser/resources/halcyon_shared/fonts/Inter-*.woff2`.
- **Why:** Humanist, modern, exceptional screen rendering at small sizes, neutral enough to disappear, distinctive enough to read as Halcyon.

### Inter Tight (display / headings)
- **Vendor:** Rasmus Andersson, OFL 1.1.
- **Used at:** 17px / 22px (title), 22px / 28px (headline), 34px / 40px (display).
- **Weights:** 500, 600.
- **Bundled in:** `chrome/browser/resources/halcyon_shared/fonts/InterTight-*.woff2`.
- **Why:** Slightly more compact than Inter. Reads as a name-as-product face for the wordmark and section headings without being heavy.

### Source Serif 4 (voice / serif moments)
- **Vendor:** Adobe, OFL 1.1.
- **Used at:** 22px / 32px italic (NTP quote, Cove greeting, About hero).
- **Weights:** 400 italic, 500 italic.
- **Bundled in:** `chrome/browser/resources/halcyon_shared/fonts/SourceSerif4-Italic-*.woff2`.
- **Why:** A serif with a warm, slightly literary character. Used sparingly, only in moments where the browser speaks in its own voice rather than as UI.

### JetBrains Mono (developer contexts only)
- **Vendor:** JetBrains, OFL 1.1.
- **Used at:** 12px / 16px.
- **Bundled in:** `chrome/browser/resources/halcyon_shared/fonts/JetBrainsMono-*.woff2`.
- **Why:** For the DevTools console, the omnibox in `view-source:` mode, and the Quill note-taking side panel when the user explicitly pastes code. **Not used in the regular chrome.**

## Why we bundle (and don't rely on system fonts)

Upstream Chrome uses the system UI font on each platform (San Francisco on macOS, Segoe on Windows, Roboto on ChromeOS, and a chosen sans on Linux). Halcyon **bundles Inter and Inter Tight on all platforms** so the visual identity is consistent. This costs ~240KB of woff2 across the three faces, which is acceptable.

System fonts are used as a *fallback* only, in order: `Inter, "Inter Tight", -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif`.

## The Halcyon typographic scale

```css
:root {
  --halcyon-font-ui:        Inter, -apple-system, "Segoe UI", Roboto, sans-serif;
  --halcyon-font-display:   "Inter Tight", Inter, -apple-system, sans-serif;
  --halcyon-font-serif:     "Source Serif 4", Georgia, serif;
  --halcyon-font-mono:      "JetBrains Mono", "SF Mono", Menlo, monospace;

  --halcyon-text-caption:   500 11px/14px var(--halcyon-font-ui);
  --halcyon-text-body:      400 13px/18px var(--halcyon-font-ui);
  --halcyon-text-body-lg:   400 15px/22px var(--halcyon-font-ui);
  --halcyon-text-subtitle:  500 14px/20px var(--halcyon-font-ui);
  --halcyon-text-title:     600 17px/22px var(--halcyon-font-display);
  --halcyon-text-headline:  600 22px/28px var(--halcyon-font-display);
  --halcyon-text-display:   500 34px/40px var(--halcyon-font-display);
  --halcyon-text-serif:     italic 400 22px/32px var(--halcyon-font-serif);
}
```

## Spacing

The Halcyon spacing scale is a 4px grid. The base unit is `--halcyon-space-1` = 4px.

```css
--halcyon-space-1:  4px;   /* hairline padding, tight gaps */
--halcyon-space-2:  8px;   /* default tight gap */
--halcyon-space-3:  12px;  /* default body gap */
--halcyon-space-4:  16px;  /* default section gap */
--halcyon-space-5:  20px;  /* group gap */
--halcyon-space-6:  24px;  /* panel padding */
--halcyon-space-7:  32px;  /* section break */
--halcyon-space-8:  48px;  /* page edge padding */
--halcyon-space-9:  64px;  /* NTP hero spacing */
--halcyon-space-10: 96px;  /* large empty-space moments */
```

## Corner radii

Halcyon uses three radii, deliberately. Most things are 6px — small enough to feel precise, large enough to feel calm.

```css
--halcyon-radius-sm: 4px;   /* inputs, list rows */
--halcyon-radius-md: 6px;   /* default — buttons, cards, menus */
--halcyon-radius-lg: 12px;  /* large cards, dialogs */
--halcyon-radius-pill: 9999px; /* only the Lantern indicator and toggle switches */
```

Tabs use **0 radius**. They are clipped to the toolbar edge, not rounded.

## Stroke widths

```css
--halcyon-stroke-hairline: 1px;   /* dividers, borders */
--halcyon-stroke-thin:    1.5px; /* icon strokes */
--halcyon-stroke-bold:    2px;   /* focus rings */
```

## Elevation

Halcyon does not use Material-style elevation tiers. There are exactly two elevation states:

- **Resting.** No shadow. Surfaces are separated by color.
- **Lifted.** A single, very soft shadow: `0 1px 3px rgba(0,0,0,0.06)`, or in dark mode `0 1px 3px rgba(0,0,0,0.4)`.

Lifted is used for:
- Menus and popovers (which need to clearly separate from the page).
- Modal dialogs.
- The Quill side panel.

It is *not* used for:
- The toolbar.
- Tab strip.
- Cards inside a page (they're separated by border, not shadow).

## Line-height and reading

WebUI body text uses 1.47 line-height (the "golden" reading ratio). The chrome (toolbar, tabs, menus) uses 1.15 — tighter, because UI labels are not paragraphs.
