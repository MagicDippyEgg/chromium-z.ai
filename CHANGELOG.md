# Changelog — Halcyon Browser

All notable changes to the Halcyon browser source tree are documented
here. The format is loosely based on [Keep a Changelog](https://keepachangelog.com/).

Halcyon versions are independent of the Chromium version it's built
against. The Halcyon tree applies cleanly to the Chromium version
noted in IMPLEMENTATION.md.

---

## [1.0.0] — 2026-11-03

The initial public source tree of the Halcyon browser.

### Identity

  - Invented the browser name "**Halcyon**" with the brand promise
    "*Calm the web.*" (`halcyon/branding/brand_book.md`)
  - Two themes: **Vellum** (light) and **Eclipse** (dark), both
    designed for calm. (`halcyon/branding/color_system.md`)
  - Typography: Inter (UI), Inter Tight (display), Source Serif 4
    (serif quotes), JetBrains Mono (code). All OFL 1.1.
    (`halcyon/branding/typography.md`)
  - Iconography: custom icons (Cairn, Lantern, Quill, Tidemarks, etc.)
    drawn from scratch. (`halcyon/branding/iconography.md`,
    `halcyon/src/chrome/browser/resources/halcyon_shared/icons/icons.svg`)
  - Voice and copy guide that rewrites upstream chrome UI strings:
    sentence case, no exclamation marks, no marketing copy, no onboarding
    tours. (`halcyon/branding/voice.md`)

### Design system

  - `halcyon/design/design_tokens.css` — the single source of truth
    for the WebUI side. CSS custom properties for every color, font,
    spacing, radius, stroke, motion, elevation, and shadow value.
    Light, dark, high-contrast, and forced-colors variants all included.
  - `halcyon/src/ui/base/halcyon_color_palette.h` — the C++ mirror
    of the design tokens, as `SkColor` constants. Light (Vellum) and
    dark (Eclipse) variants.
  - `halcyon/design/verify_tokens.py` — design-system consistency
    checker. Parses both files and exits non-zero on any mismatch.

### Native Views (C++)

The Halcyon replacement for the upstream BrowserView + frame +
toolbar + tab strip + omnibox:

  - `halcyon/src/chrome/browser/ui/views/frame/halcyon_browser_view.{h,cc}`
    — top-level shell. Slim, vertical BoxLayout of tab strip → toolbar
    → contents.
  - `halcyon/src/chrome/browser/ui/views/frame/halcyon_frame_view.{h,cc}`
    — custom window frame with a 1px sage "horizon" line on top.
  - `halcyon/src/chrome/browser/ui/views/tabs/halcyon_tab.{h,cc}`
    — single tab.
  - `halcyon/src/chrome/browser/ui/views/tabs/halcyon_tab_strip.{h,cc}`
    — horizontal tab strip. Active tab visually IS the page (background
    matches the page color).
  - `halcyon/src/chrome/browser/ui/views/tidemarks/halcyon_tidemarks_view.{h,cc}`
    — vertical tab rail (the original "Tidemarks" feature). ⌘⇧L toggles.
  - `halcyon/src/chrome/browser/ui/views/toolbar/halcyon_toolbar_view.{h,cc}`
    — toolbar container: nav buttons, omnibox, Lantern, Cairn, menu.
  - `halcyon/src/chrome/browser/ui/views/toolbar/halcyon_navigation_button.{h,cc}`
    — back / forward / reload.
  - `halcyon/src/chrome/browser/ui/views/toolbar/halcyon_menu_button.{h,cc}`
    — three short horizontal lines (NOT three dots).
  - `halcyon/src/chrome/browser/ui/views/location_bar/halcyon_omnibox_view.{h,cc}`
    — omnibox shell, including the small Margin Notes mark for URLs that
    have annotations.
  - `halcyon/src/chrome/browser/ui/views/privacy_lantern/halcyon_lantern_indicator_view.{h,cc}`
    — the Lantern, a small indicator with a slow breathing animation when
    the site is doing something worth knowing about.

### WebUI (chrome://)

  - `chrome://newtab` — **The Plume**. Calm NTP: greeting, daily quote,
    single search field, "today's intention" line, faded recently-visited
    list. No shortcuts grid. No Discover feed. No banner.
    (`halcyon/src/chrome/browser/resources/halcyon_ntp/`,
    `halcyon/src/chrome/browser/ui/webui/halcyon_ntp_*`)
  - `chrome://settings` — **Halcyon Settings**. Single column, 14
    sections, sentence-case row titles. No promotion. No "What's New"
    tour. (`halcyon/src/chrome/browser/resources/halcyon_settings/`)
  - `chrome://history` — **Tides**. History as a river: per-day
    expandable list with site weight bars. "Wash away" specific sites.
    (`halcyon/src/chrome/browser/resources/halcyon_tides/`)
  - `chrome://bookmarks` — **Cairn**. Bookmarks as named stacks, not
    a flat list. (`halcyon/src/chrome/browser/resources/halcyon_cairn/`)
  - `chrome://covetab` — **The Cove** (private browsing). Calm, dark
    start page with a single sentence explaining what's private and
    what isn't. No "incognito" branding. No spy disclaimer.
    (`halcyon/src/chrome/browser/resources/halcyon_cove/`)
  - `chrome://halcyon` — **About Halcyon**. Single page of plain text.
    No logos, no splash, no "Made with ♥".
    (`halcyon/src/chrome/browser/resources/halcyon_about/`,
    `halcyon/src/chrome/browser/ui/webui/halcyon_about_*`)
  - `chrome://halcyon-quill` — **Quill**. Per-origin note-taking side
    panel. Storage at `<profile>/Quill/<origin-sha256[:8]>/<note-uuid>.md`.
    Autosave on input (debounced 400ms). Plain-language timestamps.
    ⌘⇧N opens. (`halcyon/src/chrome/browser/resources/halcyon_quill/`,
    `halcyon/src/chrome/browser/ui/webui/halcyon_quill_*`)
  - `chrome://halcyon-lantern-report` — **Lantern Report**. Weekly,
    on-demand, local-only summary of what the Lantern observed or
    blocked this week. Tide-table breakdown plus per-site expandable
    list. No data ever leaves the device.
    (`halcyon/src/chrome/browser/resources/halcyon_lantern_report/`,
    `halcyon/src/chrome/browser/ui/webui/halcyon_lantern_report_*`)
  - `chrome://halcyon-margin-notes` — **Margin Notes overlay UI** (used
    by the content script; not in chrome://chrome-urls).
    (`halcyon/src/chrome/browser/resources/halcyon_margin_notes/`,
    `halcyon/src/chrome/browser/ui/webui/halcyon_margin_notes_host.*`)

### Original features

  1. **The Plume** (calm NTP) — no shortcuts grid, no Discover feed.
  2. **Tidemarks** — optional vertical tab rail (⌘⇧L).
  3. **Quiet Mode** — global toggle (⌘⇧M) that mutes notifications,
     popups, and autoplay. Default on for new users.
  4. **The Lantern** — privacy/tracking indicator with a slow
     breathing animation when something is happening. Click to see a
     plain-language list.
  5. **Margin Notes** — per-URL annotations on every http(s) page,
     injected via Shadow DOM. Right-click → "Add a margin note". A
     small "·N" mark in the omnibox when the URL has notes.
  6. **Tides** (history, reimagined) — history as a tide table, not a
     list. Per-day expandable. Wash sites away.
  7. **Cairn** (bookmarks, reimagined) — bookmarks as named stacks,
     not a flat list.
  8. **The Cove** (private browsing, reimagined) — calm, dark start
     page with one sentence about what's private.
  9. **Anchored tabs** — tabs that stay open across sessions, group at
     the start of the strip. UI is implemented; cross-session
     persistence is a deferred patch.
  10. **Quill** — per-origin note-taking side panel (⌘⇧N).
  11. **Halcyon Search** — multiple engines pre-installed with a clean
      default picker. Default: DuckDuckGo.
  12. **Lantern Reports** — weekly, on-demand, local-only summary.

### Patches (integration glue)

Each patch modifies a small number of upstream files to wire the
Halcyon code into the build:

  - `patches/gyp_enable_halcyon.patch` — adds the `enable_halcyon`
    build flag.
  - `patches/theme_use_halcyon.patch` — makes the build use
    `halcyon::HalcyonTheme` instead of upstream `ThemeService`.
  - `patches/chrome_command_ids_halcyon.patch` — adds the
    IDC_HALCYON_* command IDs.
  - `patches/browser_view_replace_toolbar.patch` — replaces upstream
    ToolbarView with HalcyonToolbarView.
  - `patches/browser_view_replace_tab_strip.patch` — replaces upstream
    TabStrip with HalcyonTabStrip.
  - `patches/webui_register_halcyon_ntp.patch` — registers the Plume
    at chrome://newtab.
  - `patches/webui_register_halcyon_pages.patch` — registers the four
    additional Halcyon chrome:// pages (About, Quill, Lantern Report,
    Margin Notes overlay).
  - `patches/context_menu_add_margin_note.patch` — adds the "Add a
    margin note" item to the page context menu.

### Tooling

  - `halcyon/design/verify_tokens.py` — design-system consistency
    checker (CSS vs C++).
  - `halcyon/build/fetch_fonts.py` — downloads the OFL 1.1 fonts
    (Inter, Inter Tight, Source Serif 4, JetBrains Mono) from Google
    Fonts into `halcyon/src/chrome/browser/resources/halcyon_shared/fonts/`.
  - `halcyon/build/gen_build_gns.py` — generates the per-resource-dir
    BUILD.gn files from a manifest.
  - `halcyon/preview/halcyon_preview.html` — standalone HTML
    rendering of the Halcyon chrome. Open in any browser to see the
    design language without building.

### Documentation

  - `README.md` — the one-page story.
  - `ARCHITECTURE.md` — the four-layer model and file-by-file map.
  - `IMPLEMENTATION.md` — the build procedure a Chromium engineer
    would follow.
  - `halcyon/branding/brand_book.md` — the brand book (identity,
    palette, typography, iconography, voice, motion, the menu map,
    and the list of original features).
  - `halcyon/branding/{color_system,typography,iconography,voice}.md`
    — the four canonical design-system references.
  - `halcyon/build/BUILD.md` — the build flags reference.

### Deferred (intentionally, documented)

  - Anchored Tabs cross-session persistence (UI is there; persistence
    needs a TabStripModel patch).
  - The remaining ~10 icons of the catalog (the most-used ~30 are in
    `icons.svg`; the rest are documented in `iconography.md`).
  - The physical font files (`halcyon_shared/fonts/`). Run
    `halcyon/build/fetch_fonts.py` to download them from Google Fonts.
