# Halcyon — Architecture

This document is a map of the Halcyon codebase for engineers who need to
understand the design before changing it. It complements `README.md`
(overview) and `IMPLEMENTATION.md` (build/apply procedure).

---

## Design intent (the 90-second version)

Halcyon is a Chromium reskin in the strict sense — the engine is unchanged
— but a deliberate redesign of every user-facing surface. The Halcyon
codebase is organized around three principles:

1. **The design system is the source of truth.** Every color, font, spacing
   value, and motion parameter is defined once, in one place, and
   everything else references it. CSS variables on the WebUI side, SkColor
   constants on the native side, both generated from the same brand book.
2. **The chrome is a thin shell.** The browser chrome (toolbar, tabs,
   frame, omnibox, menus) is replaced; the engine, network, security, and
   web-platform layers are upstream. Halcyon's code lives in a small number
   of well-defined files; everything else is delegated.
3. **Calm by default, loud only on purpose.** Halcyon's UI never
   interrupts. The Lantern is the only animation; popups only open on
   click; nothing promotes itself. The defaults (Quiet Mode on,
   notifications off, autoplay muted) are chosen for the user's attention,
   not the platform's metrics.

---

## The four layers

```
┌────────────────────────────────────────────────────────────────────┐
│ WebUI (chrome://) — HTML/CSS/JS                                     │
│   Plume, Settings, Tides, Cairn, Cove, Lantern, Quill               │
├────────────────────────────────────────────────────────────────────┤
│ Views (native C++) — chrome/browser/ui/views/                       │
│   BrowserView, FrameView, ToolbarView, TabStrip, Tidemarks,         │
│   OmniboxView, LanternIndicator, MenuButton, NavigationButton        │
├────────────────────────────────────────────────────────────────────┤
│ Design system — design_tokens.css + halcyon_color_palette.h          │
│   Single source of truth for colors, fonts, motion, spacing          │
├────────────────────────────────────────────────────────────────────┤
│ Brand book — halcyon/branding/*.md                                   │
│   The intent: voice, palette, typography, iconography, motion        │
└────────────────────────────────────────────────────────────────────┘
```

The bottom layer (brand book) is prose — what the browser *is*. The next
layer (design system) is the encoded version of the brand book. The next
two (Views and WebUI) consume the design system.

This means: if you want to know why a color is the value it is, look one
layer up. If you want to change a color, change it in the design system;
everything else updates.

---

## File-by-file map (the parts that matter most)

### The design system

| File | Role |
|------|------|
| `src/halcyon/branding/brand_book.md` | The whole identity: name, philosophy, visual identity, voice, motion, the menu structure, and the 12 original features. Read this first. |
| `src/halcyon/branding/color_system.md` | The full color palette, light and dark, with accessibility ratios and forbidden combinations. |
| `src/halcyon/branding/typography.md` | Fonts, scale, spacing, radii, strokes, elevation. |
| `src/halcyon/branding/iconography.md` | The icon set catalog and rules. |
| `src/halcyon/branding/voice.md` | Copy rules, word choices, sentence patterns, and the list of upstream strings that Halcyon deletes. |
| `src/halcyon/design/design_tokens.css` | The single CSS file every Halcyon WebUI page imports. CSS custom properties for every design token. |
| `src/ui/base/halcyon_color_palette.h` | The C++ equivalent of `design_tokens.css`. `SkColor` constants and `inline` helpers that pick the light/dark variant. |

### WebUI (chrome://)

Each Halcyon WebUI page has the same structure: an HTML file, a CSS file,
a JS file (often a single file rather than split into a TS module tree),
and a `BUILD.gn`. The JS talks to the C++ side via `chrome.send` /
`window.halcyon.<name>Result` callbacks; each page defines its own message
set.

| URL | File | Notes |
|-----|------|-------|
| `chrome://newtab` | `chrome/browser/resources/halcyon_ntp/plume.*` | The Plume. Default NTP. |
| `chrome://settings` | `chrome/browser/resources/halcyon_settings/settings.*` | Redesigned Settings. |
| `chrome://tides` | `chrome/browser/resources/halcyon_tides/tides.*` | Tides (history). |
| `chrome://cairn` | `chrome/browser/resources/halcyon_cairn/cairn.*` | Cairn (bookmarks). |
| `chrome://cove` | `chrome/browser/resources/halcyon_cove/cove.*` | The Cove (private NTP). |
| `chrome://lantern` | `chrome/browser/resources/halcyon_lantern/lantern.*` | The Lantern popup. |
| `chrome://quill` | `chrome/browser/resources/halcyon_quill/quill.*` | Quill side panel (deferred). |

The C++ side of each WebUI page is in `chrome/browser/ui/webui/`, with
the `Halcyon*MessageHandler` class bridging `chrome.send` calls to
Chromium's existing services (HistoryService, AutocompleteController,
Profile prefs, etc.).

### Native Views

| File | Role |
|------|------|
| `chrome/browser/ui/views/frame/halcyon_browser_view.{h,cc}` | Top-level shell. Owns the toolbar, tab strip, contents container, and (off-screen) Tidemarks. |
| `chrome/browser/ui/views/frame/halcyon_frame_view.{h,cc}` | Window frame: 1px Sage border, no title bar text by default. |
| `chrome/browser/ui/views/toolbar/halcyon_toolbar_view.{h,cc}` | The toolbar container. |
| `chrome/browser/ui/views/toolbar/halcyon_menu_button.{h,cc}` | The single-line menu button (not three-line ≡). |
| `chrome/browser/ui/views/toolbar/halcyon_navigation_button.{h,cc}` | Back, forward, reload. |
| `chrome/browser/ui/views/tabs/halcyon_tab.{h,cc}` | A single tab. |
| `chrome/browser/ui/views/tabs/halcyon_tab_strip.{h,cc}` | The horizontal tab strip. |
| `chrome/browser/ui/views/tidemarks/halcyon_tidemarks_view.{h,cc}` | The vertical tab rail (original feature). |
| `chrome/browser/ui/views/location_bar/halcyon_omnibox_view.{h,cc}` | The omnibox shell — wraps upstream OmniboxView visually. |
| `chrome/browser/ui/views/privacy_lantern/halcyon_lantern_indicator_view.{h,cc}` | The Lantern indicator (original feature). |
| `chrome/browser/ui/views/bookmarks/halcyon_cairn_indicator_view.{h,cc}` | The Cairn indicator. |

### Resources and strings

| File | Role |
|------|------|
| `chrome/app/chromium_strings/chromium_strings.grd.halcyon` | The Halcyon string overrides. Inserted into the upstream grit file at build time via `patches/grit_overlay_halcyon.patch`. |
| `chrome/app/generated_resources/generated_resources.grd.halcyon` | Halcyon-specific resource overrides (icons, NTP assets, etc.). |
| `chrome/browser/resources/halcyon_shared/icons/icons.svg` | The icon sprite. Loaded by `<halcyon-icon>` and by `HalcyonIconLoader` for native. |
| `chrome/browser/resources/halcyon_shared/js/halcyon_icon.js` | The `<halcyon-icon>` custom element. |
| `chrome/browser/resources/halcyon_shared/js/halcyon_theme.js` | The theme controller. Sets `data-halcyon-theme` on `<html>` based on user pref and system. |
| `chrome/browser/resources/halcyon_ntp/quotes.json` | The curated quote set for the Plume. |

### The theme layer

Halcyon's theme provider is `chrome/browser/themes/halcyon_theme.cc`. It
subclasses upstream's `ThemeService` and overrides the color getters to
return Halcyon colors. The override is wired in by
`patches/theme_use_halcyon.patch`, which changes the `ThemeServiceFactory`
to create a `HalcyonTheme` instead of the upstream `ThemeImpl`.

When the user changes their theme preference (via Settings → Appearance →
Theme), the preference is updated and the theme service notifies all
observers (the WebUI pages pick up the change via `halcyon-theme-change`
CustomEvent; the native Views repaint with the new `NativeTheme` colors).

---

## How a new Halcyon feature is added

Suppose you want to add a new feature — say, a "Quiet Hours" schedule
that automatically toggles Quiet Mode at night. Here's the workflow:

1. **Read the brand book.** Does the feature fit the "calm the web"
   promise? Would it make the browser quieter or louder? If louder,
   it has to earn its way in.
2. **Add the design system tokens.** If the feature needs a new color,
   add it to `color_system.md` and `design_tokens.css` and
   `halcyon_color_palette.h`. If it needs a new icon, add it to
   `iconography.md` and draw the SVG in `icons.svg`.
3. **Write the WebUI** (if it has a chrome:// page). Add a directory
   under `chrome/browser/resources/halcyon_<name>/` with HTML/CSS/JS
   and a `BUILD.gn`. Add a C++ `Halcyon*MessageHandler` in
   `chrome/browser/ui/webui/`. Register the URL via a new patch in
   `patches/webui_register_halcyon_<name>.patch`.
4. **Write the native Views** (if it has a chrome surface). Add files
   under `chrome/browser/ui/views/<area>/`. Wire them into the
   BrowserView / ToolbarView / etc. via additional patches.
5. **Add the strings.** Add new message IDs to the relevant
   `.grd.halcyon` file.
6. **Update the GN graph.** Add the new targets to `chrome/browser/ui/BUILD.gn`
   via a patch.
7. **Document.** Add an entry in `ARCHITECTURE.md` and, if the feature
   changes the user experience significantly, in `brand_book.md`.
8. **Test.** Build, run, navigate to your feature, and verify it works
   against real websites (not just the WebUI page in isolation).

The Halcyon codebase is intentionally small (a few hundred files, not
thousands). The reason is the brand book: most proposed features would
make the browser louder, and they're rejected at step 1.

---

## What Halcyon does NOT modify

The Halcyon source touches *only* the user-facing layers. It does not
modify:

- The Blink rendering engine (third_party/blink).
- The V8 JavaScript engine (v8/).
- The network stack (net/, services/network/).
- The multi-process architecture (content/).
- The GPU compositor (gpu/, viz/).
- The extension API (extensions/).
- The devtools protocol and frontend (third_party/devtools-frontend).
- The sandbox and security model.
- The IPC layer (mojo/).
- The crash reporter.

Halcyon's modifications are in:

- The Views framework (chrome/browser/ui/views/).
- The WebUI layer (chrome/browser/resources/, chrome/browser/ui/webui/).
- The theme service (chrome/browser/themes/).
- The strings and resource files (chrome/app/).
- A handful of GN build files.

This scoping is what makes Halcyon a *real* browser, not a fork: any
upstream Chromium security or performance improvement lands in Halcyon as
soon as the Halcyon source tree is re-applied. There is no merge cost.

---

## Deferred work

The following are documented but not yet fully implemented. Each is a
follow-up task tracked separately.

- **Quill** side panel — the directory and stub files exist; the JS is
  not yet written.
- **Margin Notes** — the menu item is added; the per-URL storage model
  and the margin card UI are not yet implemented.
- **Anchored Tabs persistence** — the UI is there; the cross-session
  persistence (saving anchored tab URLs to a pref and reopening them on
  launch) is a TabStripModel change that needs its own patch.
- **The full icon catalog** — `icons.svg` has ~30 of the most-used icons;
  the remaining ~10 are documented in `iconography.md` and need to be
  drawn and added.
- **The bundled fonts** — the `halcyon_shared/fonts/` directory is
  created but the Inter / Inter Tight / Source Serif 4 / JetBrains Mono
  woff2 files are not bundled (they are OFL 1.1; the build process
  downloads them). The download script is in
  `src/halcyon/build/fetch_fonts.py`.
- **The smoke test** — referenced by `IMPLEMENTATION.md` as
  `out/Halcyon/halcyon_smoke_test`; the test source is not yet in this
  repo. To be added.
- **Localization** — all strings are currently in English. The Halcyon
  strings file declares `translateable="true"` for translatable entries,
  so once the source tree is applied to a real Chromium checkout the
  upstream translation pipeline will pick them up.

These are intentional deferrals, not oversights. Each is tracked
separately and has an owner per the project plan.

---

## Coding conventions

- **File naming:** Halcyon files are all prefixed `halcyon_*` (e.g.,
  `halcyon_toolbar_view.cc`). This makes them easy to find with `grep`
  and avoids any naming collisions with upstream Chromium files.
- **Class naming:** Halcyon C++ classes are all in the `halcyon::`
  namespace, e.g., `halcyon::HalcyonToolbarView`. Within the namespace,
  the `Halcyon` prefix is kept because the type is referenced from the
  `chrome` namespace and `chrome::halcyon::ToolbarView` would be too
  much shorter to justify.
- **CSS:** All Halcyon CSS classes are prefixed `halcyon-`. All CSS
  custom properties are prefixed `--halcyon-`.
- **JS:** All globals are under `window.halcyon.*`. The `<halcyon-icon>`
  and `<halcyon-toggle>` custom elements are the only global custom
  elements.
- **No upstream APIs are removed.** Where Halcyon replaces a class (e.g.,
  ToolbarView → HalcyonToolbarView), the upstream class is left in place;
  Halcyon simply doesn't reference it. This keeps the code mergeable.
- **Comments:** Every Halcyon source file has a copyright header and a
  top-of-file comment explaining what the file is and why it exists. The
  brand book is referenced by URL where relevant.

---

## Why this is not a fork

A "fork" implies divergence: the fork maintains its own copy of upstream
code and has to merge upstream changes manually. Halcyon does not do
this. The Halcyon source tree contains *only* new files (with the
upstream file paths) and *patches* against a small number of upstream
files (the registration glue). When a new Chromium version drops:

1. Re-apply the Halcyon source tree via `rsync`.
2. Re-apply the patches. Most will apply cleanly; the ones that fail
   document the change well enough to re-apply manually.
3. Build.

There is no merge of upstream code into the Halcyon tree because the
Halcyon tree does not contain upstream code. This is the same model that
Brave, Vivaldi, and several other Chromium-based browsers use.
