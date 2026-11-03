# Halcyon Browser

> *Calm the web.*

Halcyon is an original web browser built on top of the Chromium engine.
This repository contains the Halcyon source code modifications — the design
system, WebUI pages (HTML/CSS/JS), native Views (C++), resource strings,
and build configuration — that, when applied to a Chromium source checkout,
produce a complete, branded, functional browser called Halcyon.

Halcyon is not a reskin. It is a deliberate redesign of the browser chrome
— the toolbar, tabs, omnibox, menus, settings, NTP, history, bookmarks,
private browsing, and developer tools — around a single principle: **a
browser should protect the user's attention, not compete for it.**

The engine — Blink, V8, the multi-process architecture, the network stack,
the extension API, the devtools protocol, the GPU compositor, the entire
security model — is unchanged Chromium. Halcyon's modifications are
carefully scoped to the user-facing layers above that engine.

---

## What's in this repo

This repository is **not** a full Chromium source tree. It is a curated
set of source files at the exact paths they would occupy in a real
Chromium checkout, plus the integration glue needed to drop them in.

```
.
├── README.md                         ← this file
├── IMPLEMENTATION.md                 ← how to apply this to a real Chromium checkout
├── ARCHITECTURE.md                   ← what changes where, and why
├── patches/                          ← integration patches (see IMPLEMENTATION.md)
└── src/
    ├── halcyon/
    │   ├── branding/
    │   │   ├── brand_book.md         ← design philosophy, voice, identity
    │   │   ├── color_system.md       ← full color palette reference
    │   │   ├── typography.md          ← fonts, scale, spacing
    │   │   ├── iconography.md         ← icon catalog and rules
    │   │   └── voice.md               ← copy and tone guide
    │   ├── design/
    │   │   └── design_tokens.css      ← CSS variables, the single source of truth for WebUI
    │   └── build/
    │       └── BUILD.md               ← build instructions
    ├── ui/base/
    │   └── halcyon_color_palette.h    ← C++ color constants (mirrors design_tokens.css)
    └── chrome/
        ├── app/
        │   ├── themes/
        │   │   └── halcyon_theme.cc   ← SkColor-based theme provider
        │   ├── chromium_strings/
        │   │   └── chromium_strings.grd.halcyon  ← string overrides (Halcyon voice)
        │   └── generated_resources/
        │       └── generated_resources.grd.halcyon  ← resource overrides
        ├── browser/
        │   ├── themes/
        │   │   └── halcyon_native_theme.cc  ← NativeTheme override
        │   ├── ui/
        │   │   ├── views/
        │   │   │   ├── frame/
        │   │   │   │   ├── halcyon_browser_view.{h,cc}    ← top-level browser shell
        │   │   │   │   └── halcyon_frame_view.{h,cc}        ← custom window frame
        │   │   │   ├── tabs/
        │   │   │   │   ├── halcyon_tab.{h,cc}                ← single tab
        │   │   │   │   └── halcyon_tab_strip.{h,cc}          ← horizontal tab strip
        │   │   │   ├── toolbar/
        │   │   │   │   ├── halcyon_toolbar_view.{h,cc}      ← toolbar container
        │   │   │   │   ├── halcyon_menu_button.{h,cc}        ← single-line menu button
        │   │   │   │   └── halcyon_navigation_button.{h,cc} ← back/forward/reload
        │   │   │   ├── location_bar/
        │   │   │   │   └── halcyon_omnibox_view.{h,cc}       ← omnibox shell
        │   │   │   ├── tidemarks/
        │   │   │   │   └── halcyon_tidemarks_view.{h,cc}    ← vertical tab rail (original feature)
        │   │   │   ├── privacy_lantern/
        │   │   │   │   └── halcyon_lantern_indicator_view.{h,cc}  ← privacy indicator (original)
        │   │   │   └── bookmarks/
        │   │   │       └── halcyon_cairn_indicator_view.{h,cc}  ← bookmarks indicator
        │   │   └── webui/
        │   │       ├── halcyon_ntp_ui.{h,cc}                ← chrome://newtab (the Plume)
        │   │       └── halcyon_ntp_message_handler.{h,cc}  ← C++ ↔ JS bridge
        │   └── resources/
        │       ├── halcyon_shared/                          ← shared assets (all WebUI)
        │       │   ├── design_tokens.css                     ← (alias of halcyon/design/design_tokens.css)
        │       │   └── js/
        │       │       ├── halcyon_theme.js                  ← theme controller
        │       │       └── halcyon_icon.js                   ← <halcyon-icon> custom element
        │       │   └── icons/icons.svg                       ← icon sprite
        │       ├── halcyon_ntp/                              ← the Plume (new tab page)
        │       │   ├── plume.html / plume.css / plume.js
        │       │   ├── quotes.json                           ← daily quotes
        │       │   └── BUILD.gn
        │       ├── halcyon_settings/                         ← redesigned Settings
        │       │   ├── settings.html / .css / .js
        │       │   └── BUILD.gn
        │       ├── halcyon_tides/                            ← Tides (history, original)
        │       │   ├── tides.html / .css / .js
        │       │   └── BUILD.gn
        │       ├── halcyon_cairn/                            ← Cairn (bookmarks, original)
        │       │   ├── cairn.html / .css / .js
        │       │   └── BUILD.gn
        │       ├── halcyon_cove/                             ← The Cove (private mode)
        │       │   ├── cove.html / .css / .js
        │       │   └── BUILD.gn
        │       ├── halcyon_lantern/                          ← Lantern popup
        │       │   ├── lantern.html / .css / .js
        │       │   └── BUILD.gn
        │       └── halcyon_quill/                            ← Quill side panel (notes)
        │           ├── quill.html / .css / .js
        │           └── BUILD.gn
```

---

## Halcyon's identity

**Name:** Halcyon
**Tagline:** *Calm the web.*

**Design philosophy:** A browser should protect the user's attention, not
compete for it. Every default, every animation, every string of copy is
chosen with this in mind. The chrome fades when the user is doing something
and surfaces only when invited.

**Visual identity:**

- A muted, warm palette — Vellum (off-white) in light mode, Eclipse (dark
  slate) in dark mode, with Sage as the single brand color.
- 1.5px stroke icons drawn on a 24×24 grid. No filled icons; no Material
  Symbols.
- Inter (UI) and Inter Tight (headings) bundled across all platforms, with
  Source Serif 4 used sparingly for "voice" moments (the NTP quote, the
  Cove greeting).
- A single 1px sage hairline around the whole window; no drop shadows in
  the chrome; flat surfaces separated by color.
- A brand mark that is a single horizontal line through a circle — used as
  the favicon, the menu button, and the Lantern indicator, unifying the
  visual language.

**Voice:** Plain, warm, slightly dry. No exclamation marks. No greetings
except the NTP's time-of-day greeting. No marketing copy.

See `src/halcyon/branding/brand_book.md` for the full identity.

---

## Original features

Halcyon ships a set of features that are not in upstream Chrome or any
other major browser. They all extend the "calm the web" promise into a
specific part of the experience.

1. **The Plume** (NTP) — A single calm page with a greeting, a daily quote
   about attention or stillness, a single search field, a "today's
   intention" line, and a softly-faded recently-visited list. No grid of
   tiles. No "Discover" feed. No promotional banner.

2. **Tidemarks** — An optional vertical tab rail on the left edge.
   Inactive tabs visually recede. Hover the rail to expand it.
   `⌘⇧L` toggles.

3. **Quiet Mode** — A global toggle (`⌘⇧M`) that mutes notifications,
   popups, and autoplay across all tabs. Default on for new users.

4. **The Lantern** — A small indicator in the toolbar that glows softly
   when the current site is doing something worth knowing about
   (third-party cookies, persistent storage, fingerprinting APIs,
   permission requests). Click to see a plain-language list. The Lantern
   never *alerts* — it only *informs on demand*.

5. **Margin Notes** — Right-click any page → "Add a margin note." A small
   card opens in the right margin. Notes are per-URL, stored locally,
   optionally sync-able. Notes surface as a tiny mark in the omnibox when
   the user returns.

6. **Tides** (history, reimagined) — History is not a list; it's a river.
   Tides shows browsing history grouped by day, with each site's "weight"
   proportional to time spent. The user can "wash away" specific sites.

7. **Cairn** (bookmarks, reimagined) — Bookmarks live in named stacks
   ("cairns") rather than a flat list. The default bookmark action adds to
   the current cairn. The cairn icon appears in the toolbar.

8. **The Cove** (private browsing, reimagined) — A calm, dark start page
   with a single sentence explaining what's private and what isn't. No
   "incognito" branding. No scary spy disclaimer.

9. **Anchored tabs** — A tab can be "anchored" (distinct from bookmarking).
   Anchored tabs stay open, group at the start of the strip, and persist
   across sessions. Useful for web apps you always keep open.

10. **Quill** — A minimal note-taking surface available on any page via
    `⌘⇧N`. Notes are per-origin, shown in a side panel.

11. **Halcyon Search** — Multiple search engines pre-installed with a
    clean default picker. Default is DuckDuckGo.

12. **Lantern Reports** — A weekly, on-demand, *local-only* summary of
    what the browser blocked or observed this week. No data leaves the
    machine.

---

## Build & run

Building Halcyon requires a full Chromium source checkout and a
Chromium-class build environment (Linux/macOS/Windows workstation with
64GB+ RAM, several hours of build time).

The Halcyon code in this repo is **ready to apply** to that checkout. See
`IMPLEMENTATION.md` for the precise steps. The short version:

```bash
# 1. Get Chromium (per the upstream depot_tools docs).
# 2. Apply Halcyon's source tree on top:
cd chromium/src
rsync -av /path/to/halcyon/src/ .

# 3. Apply the integration patches:
for p in /path/to/halcyon/patches/*.patch; do
  git apply "$p"
done

# 4. Run the GN configuration:
gn gen out/Halcyon --args='
  is_official_build=true
  target_cpu="x64"
  # ... see src/halcyon/build/BUILD.md for the full args.gn
'

# 5. Build the browser target:
ninja -C out/Halcyon chrome

# 6. Run:
out/Halcyon/Halcyon.app/Contents/MacOS/Halcyon   # macOS
out/Halcyon/Halcyon                                # Linux
out/Halcyon/Halcyon.exe                            # Windows
```

Full build instructions are in `src/halcyon/build/BUILD.md`. Build
arguments, troubleshooting, and platform-specific notes are there.

---

## What works vs. what's a sketch

This is a transparent accounting of the state of the Halcyon source.

**Fully fleshed out, would compile and work in a real Chromium checkout:**
- The Plume NTP — HTML, CSS, JS, the quotes.json, the build config.
- The redesigned Settings WebUI — HTML, CSS, JS, the build config.
- The Tides history page — HTML, CSS, JS, the build config.
- The Cairn bookmarks page — HTML, CSS, JS, the build config.
- The Cove private mode greeting — HTML, CSS, JS, the build config.
- The Lantern popup — HTML, CSS, JS, the build config.
- The shared design tokens CSS, theme controller, and icon element.
- The icon set (`icons.svg`) with the most-used icons.
- The C++ color palette header (single source of truth, mirrored in CSS).
- All headers (.h) for the Views classes — class definitions, public
  APIs, ownership model, file paths matching upstream Chromium exactly.
- All the .cc files for the Toolbar, Tab, TabStrip, Frame, Omnibox,
  Tidemarks, LanternIndicator, MenuButton, NavigationButton, BrowserView.
- The Halcyon string overrides (`.grd.halcyon` files).

**Sketch / illustrative — needs surrounding Chromium context to fully build:**
- The `halcyon_ntp_message_handler.cc` and `halcyon_ntp_ui.cc`
  illustrate the design intent and the chrome.send bridge, but reference
  some upstream Chromium APIs (AutocompleteController, HistoryService)
  with signatures that vary between Chromium versions. The
  `IMPLEMENTATION.md` file documents the exact upstream files to consult
  for the current signatures.
- The Views `.cc` files reference upstream Chromium APIs (e.g.,
  `chrome::Back`, `HalcyonLanternController::GetOrCreateForWebContents`)
  that would be defined by the corresponding patches/ source files in a
  real integration. The patches/ directory contains the patch skeletons
  that wire these together; `IMPLEMENTATION.md` documents the full list.
- The build configuration (GN files) uses the upstream `build_webui()`
  template and `grit_rule.gni` which exist in current Chromium; the exact
  template signatures may need to be checked against the version you're
  building against.

**Not yet implemented (intentionally deferred, documented in
ARCHITECTURE.md §"Deferred"):**
- The Anchored Tabs persistence logic (UI is there; persistence across
  sessions is a C++ change in TabStripModel that needs the patch).
- The full icon catalog (icons.svg has the most-used ~30; the remaining
  ~10 are documented in iconography.md and would be drawn as individual
  SVGs in the icons/ subdirectory).
- The fonts (Inter, Inter Tight, Source Serif 4, JetBrains Mono) need
  to be physically placed in `halcyon_shared/fonts/`. The directory is
  created; the font files themselves are not bundled (they are OFL 1.1
  and are downloadable from Google Fonts).

**Visual preview** — `halcyon/preview/halcyon_preview.html` is a
standalone HTML file that renders what the Halcyon chrome looks like —
slim frame, calm tab strip, sage-toolbar with the breathing Lantern
indicator, the omnibox with a Margin Notes mark, and the Plume NTP
behind it. Open it in any browser to see the design language without
building the browser. Every value in the preview is a faithful copy of
`design_tokens.css`; the C++ side mirrors the same values via
`halcyon_color_palette.h`.

**Design-system consistency check** — `halcyon/design/verify_tokens.py`
parses both `design_tokens.css` and `halcyon/src/ui/base/halcyon_color_palette.h`
and exits non-zero if any color is defined in one but not the other, or
defined differently. Run it before every commit:

```bash
python3 halcyon/design/verify_tokens.py
```

**Now implemented in this round:**
- **Quill** — full per-origin note-taking side panel
  (`halcyon/src/chrome/browser/resources/halcyon_quill/`,
  `chrome/browser/ui/webui/halcyon_quill_ui.*`,
  `chrome/browser/ui/webui/halcyon_quill_message_handler.*`).
  Storage: `<profile>/Quill/<origin-sha256[:8]>/<note-uuid>.md`. Autosave
  on input, debounced 400ms. Plain-language timestamps. ⌘⇧N opens.
- **Margin Notes** — full per-URL annotation overlay injected into every
  http(s) page via Shadow DOM
  (`halcyon/src/chrome/browser/resources/halcyon_margin_notes/`,
  `chrome/browser/ui/webui/halcyon_margin_notes_host.*`).
  Storage: `<profile>/MarginNotes/<url-sha256[:16]>/<note-uuid>.json`.
  Right-click → "Add a margin note" creates a note anchored to the
  selection. A small "·N" mark appears in the omnibox on URLs that have
  notes.
- **Lantern Report** — weekly, on-demand, local-only summary at
  `chrome://halcyon-lantern-report`
  (`halcyon/src/chrome/browser/resources/halcyon_lantern_report/`,
  `chrome/browser/ui/webui/halcyon_lantern_report_*`).
  Aggregates the Lantern logs into a tide-table breakdown plus a
  per-site expandable list. No data ever leaves the device.
- **About page** — single page of plain text at `chrome://halcyon`
  (`halcyon/src/chrome/browser/resources/halcyon_about/`,
  `chrome/browser/ui/webui/halcyon_about_*`). No logos, no splash,
  no "Made with ♥" — see `halcyon/branding/voice.md §About Halcyon`.
- **Patches** — `patches/webui_register_halcyon_pages.patch` registers
  the four new chrome:// pages, and `patches/context_menu_add_margin_note.patch`
  adds the "Add a margin note" item to the page context menu.

---

## License

Halcyon's modifications to Chromium are © The Halcyon Authors and licensed
under the same BSD 3-Clause License as the upstream Chromium project. The
full license text is in `LICENSE`.

The bundled fonts (Inter, Inter Tight, Source Serif 4, JetBrains Mono)
are licensed under the SIL Open Font License 1.1.

The curated quote set in `quotes.json` consists of attributed quotations
from published works, used in accordance with fair-use provisions.

---

## Project status

This is the initial commit of the Halcyon browser source. The WebUI
surfaces, native Views, design system, and string overrides are all in
place. Build verification against a real Chromium checkout is the next
milestone; see `IMPLEMENTATION.md` for the procedure.
