# Halcyon — Iconography

## Principles

1. **Stroke, not fill.** Halcyon icons are line drawings. The only filled shapes are the brand mark, the Lantern indicator (when glowing), and toggle-switch thumbs.
2. **1.5px stroke.** Slightly heavier than Material Symbols' typical 1px, slightly lighter than Lucide's 2px. Reads as deliberate without being heavy.
3. **Rounded line caps.** `stroke-linecap="round"`, `stroke-linejoin="round"`. Always.
4. **24×24 grid.** All icons drawn on a 24×24 canvas, with the active artwork in a 20×20 inner area (2px padding).
5. **Two sizes only: 16px and 20px.** Rendered at these sizes via `width`/`height`. No 18px, no 24px in the chrome.
6. **Secondary text color by default, accent when active.** Icons inherit `currentColor`; the surrounding element sets the color.
7. **No emoji as icons.** Anywhere.
8. **No Material Symbols.** Halcyon ships its own SVG icon set. Upstream uses Material Symbols; we override the icon font with our own set in `chrome/browser/resources/halcyon_shared/icons/`.

## Icon catalog

Each icon is drawn as an SVG. The catalog below lists every icon Halcyon ships. File names follow `kebab-case.svg`.

### Navigation
- `chevron-left.svg` — back
- `chevron-right.svg` — forward
- `chevron-down.svg` — dropdown indicator
- `chevron-up.svg` — collapse
- `arrow-refresh.svg` — reload
- `arrow-out.svg` — open in new tab
- `home.svg` — home (rare; used in toolbar customization)

### Tabs & windows
- `plus.svg` — new tab
- `x.svg` — close tab
- `pin.svg` — anchor tab (deliberately *not* a thumbtack — see below)
- `tabs.svg` — tab overview (rare)
- `window.svg` — new window
- `window-cove.svg` — new Cove window (a window with a small wavy line at the bottom)

### Page actions
- `star.svg` — add to cairn (outline)
- `star-filled.svg` — already in a cairn
- `download.svg` — downloads
- `share.svg` — share
- `print.svg` — print
- `find.svg` — find in page
- `zoom-in.svg`, `zoom-out.svg` — zoom

### The Lantern
- `lantern.svg` — the privacy indicator. A simple horizontal line inside a circle, identical to the brand mark, but when it has something to report, the line becomes slightly thicker and pulses. (We use the brand mark itself as the Lantern icon — it is the visual unification of "calm" and "informed.")

### The Tidemarks
- `tidemarks.svg` — three small horizontal lines of decreasing length, stacked vertically. Used as the rail toggle.

### Quiet Mode
- `quiet.svg` — a horizontal line. (Same as the brand mark's interior line, without the circle.) When Quiet Mode is off, the icon shows three short vertical bars beside the line (suggesting sound waves); when on, only the line.

### The Plume (NTP)
- `plume-greeting.svg` — used decoratively, a single curved line. (Not an action icon.)
- `plume-search.svg` — a single circle with a line extending down-right (a feather quill, drawn minimally).

### Quill (notes)
- `quill.svg` — a pen-nib, drawn as two strokes meeting at a point. Used as the Quill side-panel toggle.

### Margin Notes
- `margin-note.svg` — a small bracket `[` (used to indicate a margin note exists on a page).

### The Cove
- `cove.svg` — a curved line between two horizontal strokes. (A small bay, drawn minimally.)

### Cairn
- `cairn.svg` — three stacked horizontal lines, slightly different lengths. (A stack of stones, drawn minimally.)

### Tides (history)
- `tides.svg` — three wavy horizontal lines. Used as the history-page favicon and Tides nav icon.

### Settings
- `settings.svg` — a single horizontal slider line with a small circle on it. (Not a gear — gears suggest machinery; Halcyon settings are sliders, which suggest adjustment.)
- `settings-appearance.svg` — a circle with a small horizontal line through it (the brand mark).
- `settings-search.svg` — the plume-search quill.
- `settings-privacy.svg` — the Lantern.
- `settings-system.svg` — three short stacked horizontal lines.

### Misc
- `check.svg` — single check mark.
- `minus.svg` — single horizontal line (used in the menu button itself).
- `dot.svg` — single small filled circle (used for "page is loading" and "page has finished loading a Lantern-reportable event").

## Why no thumbtack for "anchor tab"?

The "anchor" concept is a Halcyon original — a tab that stays open across sessions. Using a thumbtack icon would suggest "pin," which already exists in upstream Chrome with a different meaning (icon-only tab). The Halcyon `pin.svg` is instead a small anchor's fluke drawn as three strokes — a vertical line, a horizontal line crossing near the top, and a curved fluke at the bottom — which reads visually as "this is anchored."

## File format

Each icon is a self-contained SVG file:

```xml
<svg xmlns="http://www.w3.org/2000/svg"
     width="24"
     height="24"
     viewBox="0 0 24 24"
     fill="none"
     stroke="currentColor"
     stroke-width="1.5"
     stroke-linecap="round"
     stroke-linejoin="round"
     role="img"
     aria-hidden="true">
  <!-- icon paths here -->
</svg>
```

The `currentColor` stroke lets the icon inherit color from its container.

## How icons are referenced

In WebUI: `<img src="chrome://resources/halcyon_shared/icons/lantern.svg">` or, more commonly, inlined via a `<halcyon-icon name="lantern">` custom element defined in `halcyon_shared/icons/`. The custom element inlines the SVG, which lets CSS animate the strokes.

In native Views: icons are loaded from the same SVG files via `gfx::ImageSkia` and `HalcyonIconLoader` (see `chrome/browser/ui/views/halcyon_icon_loader.cc`). Halcyon does not ship PNG fallbacks for the chrome — SVG is rendered natively.

## What we do not ship

- No brand logos for "Made by Halcyon" partnerships.
- No country flags (we use the locale's two-letter code instead).
- No device icons (phone, tablet, laptop) — when "Send to device" would have used one, the menu item is text only.
- No "AI" sparkle icons. (Halcyon's optional AI features, if enabled, use the Quill icon. Sparkle iconography is deliberately avoided.)
