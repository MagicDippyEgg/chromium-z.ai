# Halcyon — Implementation Guide

This document explains precisely how to apply the Halcyon source tree in
this repository to a real Chromium source checkout, build the resulting
browser, and verify that it works.

This is the procedure a Chromium engineer would follow. It assumes
familiarity with the upstream build (`depot_tools`, GN, Ninja, `gclient
sync`).

---

## Prerequisites

The same as upstream Chromium:

- A workstation with **64 GB RAM minimum, 128 GB+ recommended**. (Anything
  less and the linker will OOM.)
- **~150 GB free disk space** for the source tree + build artifacts.
- **6–12 hours** for a clean build, less for incremental.
- **depot_tools** on your PATH (see
  https://www.chromium.org/developers/how-tos/install-depot-tools).
- Linux (Ubuntu 22.04+), macOS 13+, or Windows 11. (ChromeOS is not
  a target for Halcyon at this point.)

---

## Step 1: Get a Chromium source checkout

Follow the upstream "Get the code" guide for your platform:
https://www.chromium.org/developers/how-tos/get-the-code.

```bash
mkdir chromium && cd chromium
fetch --nohooks --no-history chromium
cd src
gclient sync --no-history --with_branch_heads -D
```

This takes several hours and ~30 GB. The end state is a `chromium/src`
directory with the full source tree at the upstream `HEAD`.

If you want to build against a specific Chromium version, do so here:

```bash
git checkout <version-tag>     # e.g. 130.0.6723.0
gclient sync --with_branch_heads -D
```

Halcyon is currently developed against `main` (the version at the time of
writing — November 2026).

---

## Step 2: Apply the Halcyon source tree

```bash
cd /path/to/chromium/src
rsync -av /path/to/halcyon/src/ .
```

The Halcyon source tree mirrors the upstream layout exactly. The `rsync`
adds new files at their canonical paths and overwrites the few upstream
files that Halcyon intentionally replaces (see `patches/` for the list).

If you want to see exactly which files are touched before applying:

```bash
rsync -avn /path/to/halcyon/src/ . | less
```

---

## Step 3: Apply the integration patches

The Halcyon source tree adds new files; the integration patches modify
existing upstream files to point at the new code. Each patch is a unified
diff that applies cleanly to the version of Chromium noted in the patch
header.

```bash
for p in /path/to/halcyon/patches/*.patch; do
  echo "Applying: $p"
  git apply --index --reject "$p" || \
    echo "FAILED: $p (manual application may be needed; see patch header)"
done
```

The patches in this repository:

| Patch | Touches | Purpose |
|-------|---------|---------|
| `patches/webui_register_halcyon_ntp.patch` | `chrome/browser/ui/webui/webui_configurations.cc` | Register `chrome://newtab` to serve the Plume |
| `patches/webui_register_halcyon_settings.patch` | `chrome/browser/ui/webui/webui_configurations.cc` | Register `chrome://settings` to serve the Halcyon Settings |
| `patches/webui_register_halcyon_tides.patch` | `chrome/browser/ui/webui/webui_configurations.cc` | Register `chrome://tides` (history) |
| `patches/webui_register_halcyon_cairn.patch` | `chrome/browser/ui/webui/webui_configurations.cc` | Register `chrome://cairn` (bookmarks) |
| `patches/webui_register_halcyon_cove.patch` | `chrome/browser/ui/webui/webui_configurations.cc` | Register `chrome://cove` (private NTP) |
| `patches/webui_register_halcyon_lantern.patch` | `chrome/browser/ui/webui/webui_configurations.cc` | Register `chrome://lantern` (popup) |
| `patches/browser_view_replace_toolbar.patch` | `chrome/browser/ui/views/frame/browser_view.*` | Use HalcyonToolbarView instead of ToolbarView |
| `patches/browser_view_replace_tab_strip.patch` | `chrome/browser/ui/views/frame/browser_view.*` | Use HalcyonTabStrip |
| `patches/browser_frame_replace_frame_view.patch` | `chrome/browser/ui/views/frame/browser_frame.*` | Use HalcyonFrameView |
| `patches/chrome_command_ids_halcyon.patch` | `chrome/browser/ui/commands/chrome_command_ids.h` | Add Halcyon commands (Tidemarks, Quiet Mode) |
| `patches/grit_overlay_halcyon.patch` | `chrome/app/chromium_strings/chromium_strings.grd` | Insert the Halcyon grit-part |
| `patches/generated_resources_overlay_halcyon.patch` | `chrome/app/generated_resources/generated_resources.grd` | Insert the Halcyon grit-part |
| `patches/grd_resource_halcyon_ntp.patch` | `chrome/browser/resources/halcyon_ntp/BUILD.gn` | Register NTP resources with grit |
| `patches/gyp_enable_halcyon.patch` | `BUILD.gn`, `chrome/browser/ui/BUILD.gn` | Include the Halcyon targets in the build graph |
| `patches/theme_use_halcyon.patch` | `chrome/browser/themes/theme_service_*.cc` | Use HalcyonNativeTheme |
| `patches/strings_apply_halcyon.patch` | (no source change — the `.grd.halcyon` file is applied by grit during build via `patches/grit_overlay_halcyon.patch`) | String overrides are applied at build time |

If a patch fails to apply (e.g., because you're on a newer Chromium
version than the patch was generated against), the patch header documents
the surrounding lines and the intended change so a manual application is
straightforward. The Halcyon patches are kept small and self-contained for
this reason.

---

## Step 4: Configure the build

```bash
gn gen out/Halcyon --args='
  is_official_build = true
  is_debug = false
  symbol_level = 0
  target_cpu = "x64"
  use_goma = false        # set to true if you have a Goma account
  treat_warnings_as_errors = false
  # Halcyon-specific:
  enable_halcyon = true   # enables the Halcyon UI build targets
  # Disable a few Chrome features that Halcyon doesn't ship:
  enable_nacl = false
  enable_remoting = false
  enable_service_discovery = false
  # Bundle the Halcyon fonts:
  halcyon_bundle_fonts = true
'
```

The `enable_halcyon` GN arg is introduced by `patches/gyp_enable_halcyon.patch`
and gates the Halcyon targets in `chrome/browser/ui/BUILD.gn` and
`chrome/browser/resources/*/BUILD.gn`. When `false`, the build produces
unmodified upstream Chrome. This lets you toggle Halcyon on/off from a
single `gn arg` for testing.

The full set of Halcyon-specific `gn args` is documented in
`src/halcyon/build/BUILD.md`.

---

## Step 5: Build

```bash
ninja -C out/Halcyon chrome
```

This builds the `chrome` target, which (after the patches) is the Halcyon
browser. The output binary is named `chrome` upstream; the
`patches/rename_output_to_halcyon.patch` (only applied if you want the
binary named `Halcyon` instead of `chrome`) renames it.

If you want to skip tests during the build (faster):

```bash
ninja -C out/Halcyon chrome -j64
```

---

## Step 6: Run

```bash
out/Halcyon/chrome      # Linux
out/Halcyon/Halcyon.app/Contents/MacOS/Halcyon   # macOS
out/Halcyon/chrome.exe  # Windows
```

The browser launches. You'll see:

- A slim, single-sage-border frame.
- A horizontal tab strip across the top (or, if Tidemarks is enabled in
  Settings, a vertical rail on the left).
- A toolbar with back/forward/reload on the left, the omnibox in the
  center, and the Lantern, Cairn, and menu buttons on the right.
- A new tab page — the Plume — with the greeting, quote, search field,
  intention line, and softly-faded recently-visited list.

Try:
- `⌘T` / `Ctrl+T` — new tab. The Plume appears.
- `⌘,` / `Ctrl+,` — Settings. The Halcyon Settings page opens.
- `⌘Y` / `Ctrl+Y` — Tides (history).
- `⌘⇧B` / `Ctrl+Shift+B` — Cairn (bookmarks).
- `⌘⇧N` / `Ctrl+Shift+N` — New Cove (private) window.
- `⌘⇧L` / `Ctrl+Shift+L` — Toggle Tidemarks (vertical tabs).
- `⌘⇧M` / `Ctrl+Shift+M` — Toggle Quiet Mode.
- `⌘⇧N` / `Ctrl+Shift+N` (on a non-private window) — Wait, that's the
  private window. Use `⌘⇧K` / `Ctrl+Shift+K` for Quill instead. (See
  the keyboard shortcut reference in Settings → About.)

---

## Step 7: Verify the build

The Halcyon build comes with a small smoke test:

```bash
out/Halcyon/halcyon_smoke_test
```

This launches the browser, opens the Plume, navigates to a test page,
opens Settings, and verifies that all the Halcyon chrome is present. It
exits 0 on success, non-zero on failure with a diff of what was expected
vs. found.

For full functional testing (browsing real websites, tab management,
download flows), use the browser interactively.

---

## Troubleshooting

### A patch fails to apply

The patch was generated against a specific Chromium version (noted in
the patch header). If your checkout is newer, the surrounding context may
have changed. Manually apply the change — the patch file documents what
should change and where, even if the diff doesn't apply cleanly.

### `ninja: error: unknown target 'halcyon_smoke_test'`

You didn't run `gn gen` after applying the patches. Rerun step 4.

### The Plume shows a blank page

Open `chrome://newtab` directly. If you see the upstream Chrome NTP, the
`patches/webui_register_halcyon_ntp.patch` didn't apply correctly. If you
see a blank page with "ERR_FILE_NOT_FOUND", the NTP resources didn't get
packed — check that `chrome/browser/resources/halcyon_ntp/BUILD.gn` was
included in the build graph (via `patches/grd_resource_halcyon_ntp.patch`).

### The Lantern indicator is always dim

The Lantern subscribes to the HalcyonLanternController, which observes
per-page privacy events. If the controller isn't being created (e.g.,
because `patches/browser_view_replace_toolbar.patch` didn't apply), the
indicator has no source of observations. Check the browser's `--v=1`
output for "HalcyonLanternController" lines.

### Tabs look like Chrome's tabs

The HalcyonTabStrip isn't being used. Check that
`patches/browser_view_replace_tab_strip.patch` applied; if it didn't,
the upstream TabStrip is rendering.

### The menu opens Chrome's three-line menu, not Halcyon's single-line menu

Same issue as above — `patches/browser_view_replace_toolbar.patch` didn't
apply. The HalcyonMenuButton is created by HalcyonToolbarView, which is
created by the patched BrowserView.

### Halcyon runs but everything looks like Chrome

Make sure `enable_halcyon = true` is in your `gn args`. Re-run `gn gen`
and `ninja`.

### Halcyon runs but the dark mode looks like Chrome's dark mode

Halcyon's dark mode is its own palette ("Eclipse"), not Chrome's. If you
see Chrome's dark mode, `patches/theme_use_halcyon.patch` didn't apply.

---

## Files not in this repo but referenced by the source

These are referenced by the Halcyon code but live in upstream Chromium
(or are external) and are not bundled here:

- The Chromium source tree itself (~30 GB). Get it via `fetch`.
- `depot_tools` (build tooling).
- The Inter, Inter Tight, Source Serif 4, and JetBrains Mono font files
  (OFL 1.1). Place them in `chrome/browser/resources/halcyon_shared/fonts/`
  before building. See `src/halcyon/build/BUILD.md` for the file names
  and a download script.

---

## Why this is structured as a partial tree + patches

A full Chromium checkout is ~30 GB; bundling it would make this repo
unworkable. The structure here lets a Chromium engineer drop the Halcyon
modifications on top of a fresh upstream checkout, see exactly what
changed (via `git diff`), and build a working browser with a single
`ninja` invocation.

The trade-off is that you can't `git clone` this repo and build it
directly. You have to also have a Chromium checkout. This is the same
trade-off the Brave browser makes (Brave's `brave-core` is also a layer
on top of upstream Chromium, applied via a tool called `brave-patches`).
Halcyon's approach is conceptually identical.

---

## Contact

For Halcyon-specific questions: file an issue in this repo.
For upstream Chromium build questions: see
https://www.chromium.org/developers/.

---

## Verifying the design without building

If you want to see what the Halcyon chrome looks like without the
~6–12-hour build, open this file in any modern browser:

```
halcyon/preview/halcyon_preview.html
```

It's a self-contained HTML file (no external dependencies). It renders
the slim window frame, the calm tab strip, the sage toolbar with the
breathing Lantern indicator, the omnibox with a Margin Notes mark, and
the Plume NTP behind it. Every color and motion value in the preview is
a faithful copy of `halcyon/design/design_tokens.css`; the C++ side
mirrors the same values via `halcyon/src/ui/base/halcyon_color_palette.h`.

The preview responds to `prefers-color-scheme` — switch your OS to dark
mode (or use your browser's dev-tools "emulate media" panel) to see
the Eclipse theme.

Click anywhere outside the Lantern to toggle its breathing animation
on and off — the Lantern only animates when there's something to know
about.

## Verifying the design system is in sync

Before each commit, run the design-system consistency checker:

```
python3 halcyon/design/verify_tokens.py
```

The script parses both `halcyon/design/design_tokens.css` and
`halcyon/src/ui/base/halcyon_color_palette.h` and exits non-zero if any
color has a different value in the two files. Warnings about asymmetry
(colors defined in one file but not the other) are printed but don't
fail the check — they're documented in `halcyon/branding/color_system.md`
as deliberate aliases.

The script is intentionally a single Python file with no third-party
imports; it runs on the Python that ships with the OS.

## Verifying the WebUI pages without building

Each Halcyon WebUI page is a small HTML/CSS/JS app that talks to the
C++ side via `chrome.send`. To verify the pages render correctly
without building:

1. Open the HTML file directly in a browser (it works without
   `chrome://` because the JS gracefully degrades when `chrome.send`
   isn't available — the page just shows the empty state).

2. To exercise the chrome.send bridge, you can either:
   - Build the browser (per Steps 1–6 above) and load the page in
     the real chrome:// URL, or
   - Mock the chrome.send calls from the dev-tools console. The pages
     expose the `window.halcyon.<callbackName>` functions; calling
     them with synthetic data is the simplest way to see the page
     in a populated state. For example, on the Lantern Report page:

     ```javascript
     window.halcyon.lanternReport({
       rangeStart: Date.now() - 7 * 24 * 60 * 60 * 1000,
       rangeEnd:   Date.now(),
       summary:    "This week, Halcyon blocked 47 trackers and 12 cookies.",
       breakdown: [
         {label: "Trackers blocked",       count: 47, weight: 1.0},
         {label: "Third-party cookies",    count: 12, weight: 0.3},
         {label: "Fingerprinting",         count:  3, weight: 0.1},
         {label: "Permission prompts",     count:  0, weight: 0.0},
         {label: "Autoplay muted",         count:  4, weight: 0.1},
       ],
       sites: [
         {origin: "example.com",     total: 12, items: ["6 trackers", "4 cookies", "2 fingerprinting"]},
         {origin: "news.example.com", total:  8, items: ["4 trackers", "4 cookies"]},
       ],
     });
     ```

   This is the pattern every Halcyon WebUI page follows: the JS side
   exposes `window.halcyon.<name>(<data>)` and the C++ side calls those
   functions with real data once the user opens the page in the actual
   browser.

## Verifying the patches apply cleanly

If you want to verify that the patches in `patches/` still apply
against a given Chromium revision without doing a full build:

```
cd /path/to/chromium/src
git fetch origin main
git checkout origin/main

# Apply the Halcyon source tree:
rsync -av /path/to/halcyon/src/ .

# Try each patch in turn:
for p in /path/to/halcyon/patches/*.patch; do
  echo "--- $p ---"
  git apply --check "$p" || echo "FAILED: $p"
done
```

If a patch fails to apply, the manual change is described at the top
of the patch file (under the `---`/`+++`/`@@` lines). The patches are
intentionally minimal — most only touch a few lines.

If a patch has rotted significantly, re-create it with `git diff` and
the same line numbers; the surrounding code in upstream Chromium is
stable across releases.

## Verifying the fonts are present

The Halcyon bundle expects the OFL 1.1 fonts to live at
`chrome/browser/resources/halcyon_shared/fonts/`. The build fails if
any are missing.

To download them all (from Google Fonts):

```
python3 halcyon/build/fetch_fonts.py
```

This downloads the four families (Inter, Inter Tight, Source Serif 4,
JetBrains Mono) in their full weight range. The total is ~12 MB.
The script is idempotent; it skips files that already exist.

