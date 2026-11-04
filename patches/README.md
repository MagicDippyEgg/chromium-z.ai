# Halcyon Integration Patches

This directory contains the 8 integration patches that wire the Halcyon
source tree (in `halcyon/`) into the upstream Chromium build.

## Pinned Chromium revision

All patches in this directory are generated against a specific Chromium
revision. They are NOT version-agnostic; if the upstream files they touch
have shifted, the patches need to be regenerated against the new revision.

**Pinned revision (current):**
```
SHA:         f0ad4fd4ee3dc3ff81e176260e3f1bf441937d29
Version:     Chromium 159.0.8141.0
Tag:         refs/tags/159.0.8141.0
Tarball URL: https://commondatastorage.googleapis.com/chromium-browser-official/chromium-159.0.8141.0.tar.xz
Date:        2026-11-04 (pinned at the time of writing)
```

The pin uses the official Chromium source tarball (~5 GB) instead of
`gclient sync` (~30 GB). This is essential for fitting the build on
GitHub Actions free runners. The tarball is published for every
Chromium version that has a release branch; we chose 159.0.8141.0
because it is the latest 159.x with a published tarball.

The pinned SHA is captured in `halcyon/build/CHROMIUM_PIN.txt` for
machine-readable access.

## Applying the patches

The patches MUST be applied in dependency order. Each patch may reference
includes or symbols that were added by an earlier patch.

**Apply order:**
```
1. gyp_enable_halcyon.patch
2. chrome_command_ids_halcyon.patch
3. theme_use_halcyon.patch
4. browser_view_replace_toolbar.patch
5. browser_view_replace_tab_strip.patch
6. webui_register_halcyon_ntp.patch
7. webui_register_halcyon_pages.patch
8. context_menu_add_margin_note.patch   (currently empty — see "Deferred" below)
```

To apply all 8 in order against a fresh Chromium checkout at the pinned
revision:

```bash
cd /path/to/chromium/src
git checkout ff897141277d798d72ea3ae0a94ff3a51956b267
for p in \
  gyp_enable_halcyon.patch \
  chrome_command_ids_halcyon.patch \
  theme_use_halcyon.patch \
  browser_view_replace_toolbar.patch \
  browser_view_replace_tab_strip.patch \
  webui_register_halcyon_ntp.patch \
  webui_register_halcyon_pages.patch \
  context_menu_add_margin_note.patch
do
  git apply --allow-empty /path/to/halcyon/patches/$p
done
```

The `--allow-empty` flag is needed for patch 8 (which is currently a
deferred stub — see below).

## Verifying the patches

A verification script is provided:

```bash
# Re-fetches the pinned revision (sparse checkout, ~550 MB) and applies
# all 8 patches in order. Exits non-zero on any failure.
python3 scripts/generate_patches.py  # regenerates the patches
bash scripts/verify_patches.sh        # applies them to a fresh checkout
```

## What each patch does

| # | Patch | Files modified | Purpose |
|---|-------|----------------|---------|
| 1 | `gyp_enable_halcyon.patch` | `chrome/browser/ui/BUILD.gn`, `chrome/common/chrome_features.cc`, `chrome/common/chrome_features.h` | Imports `halcyon_features.gni`, adds `source_set("halcyon")` to the build, declares the `kHalcyonUI` runtime feature flag. |
| 2 | `chrome_command_ids_halcyon.patch` | `chrome/app/chrome_command_ids.h` | Adds 9 `IDC_HALCYON_*` command IDs in the 53000-53008 range. |
| 3 | `theme_use_halcyon.patch` | `chrome/browser/themes/theme_service_factory.cc` | Branches `GetForProfile` on `kHalcyonUI` to return the HalcyonTheme singleton. |
| 4 | `browser_view_replace_toolbar.patch` | `chrome/browser/ui/views/frame/browser_view.cc` | Conditionally creates `HalcyonToolbarView` instead of upstream `ToolbarView` when `kHalcyonUI` is enabled. |
| 5 | `browser_view_replace_tab_strip.patch` | `chrome/browser/ui/views/frame/browser_view.cc` | Conditionally instantiates `HalcyonTabStrip` in place of the upstream `TabStrip` when `kHalcyonUI` is enabled. |
| 6 | `context_menu_add_margin_note.patch` | (deferred — see below) | Originally intended to add the "Add a margin note" item to the page context menu. Currently an empty patch. |
| 7 | `webui_register_halcyon_ntp.patch` | `chrome/browser/ui/webui/chrome_web_ui_configs.cc` | Registers `HalcyonNtpUI` (the Plume) at `chrome://newtab` when `kHalcyonUI` is enabled. |
| 8 | `webui_register_halcyon_pages.patch` | `chrome/browser/ui/webui/chrome_web_ui_configs.cc` | Registers the 4 additional Halcyon chrome:// pages: `chrome://halcyon`, `chrome://halcyon-quill`, `chrome://halcyon-lantern-report`, `chrome://halcyon-margin-notes`. |

## Deferred: patch 6 (context_menu_add_margin_note)

The original skeleton patch referenced an upstream file
`chrome/browser/ui/views/chrome_javascript_app_menu_controller.cc` that
does not exist in the current Chromium revision. The Chromium UI menu
code has been refactored; the closest equivalent is
`chrome/browser/ui/views/renderer_context_menu/render_view_context_menu_views.cc`,
but wiring a new menu item through the current menu-building pipeline
requires more investigation than fit in this round.

**What works without patch 6:**
- The `IDC_HALCYON_ADD_MARGIN_NOTE` command ID IS defined (via patch 2).
- The Margin Notes feature's per-URL storage, overlay UI, omnibox
  indicator, and chrome://halcyon-margin-notes WebUI page all work.
- A future patch can wire the menu item by referencing the existing
  command ID through whatever the current context-menu pipeline is.

**What doesn't work without patch 6:**
- The right-click → "Add a margin note" menu item is not present.
  Users can still create Margin Notes via the "Add" button in the
  overlay's column header, or programmatically via the
  `HalcyonMarginNotesHost::AddNoteFromSelection` API.

## Regenerating the patches

The patches are generated by `scripts/generate_patches.py`. The script:

1. Sets up a sparse checkout of the pinned Chromium revision at
   `/tmp/halcyon-patch-work/chromium-pin/`.
2. For each patch, applies the change to the working tree, generates the
   diff via `git diff --cached`, commits the change (so the next patch's
   diff has correct line numbers), and writes the patch file here.

To regenerate after modifying the integration logic:

```bash
python3 scripts/generate_patches.py
bash scripts/verify_patches.sh   # sanity-check
```

To pin a NEW Chromium revision:

1. Update `PINNED_SHA` and `PINNED_VERSION` in
   `scripts/generate_patches.py`.
2. Update `halcyon/build/CHROMIUM_PIN.txt` with the new SHA + version.
3. Run `python3 scripts/generate_patches.py`.
4. Run `bash scripts/verify_patches.sh` to confirm the new patches apply.
5. If any patch's anchor has shifted in the new revision, fix the
   `replace_once` call in the script that targets the shifted line.
6. Commit the regenerated patches and the updated pin file.

## Files touched (summary)

The 7 non-empty patches touch a total of 6 upstream files:

- `chrome/app/chrome_command_ids.h`                     (patch 2)
- `chrome/browser/themes/theme_service_factory.cc`       (patch 3)
- `chrome/browser/ui/BUILD.gn`                          (patch 1)
- `chrome/browser/ui/views/frame/browser_view.cc`       (patches 4, 5)
- `chrome/browser/ui/webui/chrome_web_ui_configs.cc`    (patches 7, 8)
- `chrome/common/chrome_features.cc`                     (patch 1)
- `chrome/common/chrome_features.h`                     (patch 1)

Total upstream diff: ~280 lines added, ~10 lines modified.
