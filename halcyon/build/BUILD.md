# Halcyon — Build Instructions

This file documents the build arguments, environment, and font-bundling
step needed to build Halcyon. For the application procedure (how to put
the Halcyon source tree on top of a real Chromium checkout), see
`IMPLEMENTATION.md` in the repo root.

---

## Required environment

- **OS:** Linux (Ubuntu 22.04+), macOS 13+, or Windows 11.
- **RAM:** 64 GB minimum, 128 GB+ recommended. Anything less and the
  linker will run out of memory on the larger targets.
- **Disk:** 150 GB free.
- **Tooling:** `depot_tools` on your PATH.

## gn args

Create `out/Halcyon/args.gn` with the following:

```gn
is_official_build = true
is_debug = false
symbol_level = 0
target_cpu = "x64"      # or "arm64" on Apple Silicon / Linux ARM

# Build faster; safe to disable for Halcyon:
enable_nacl = false
enable_remoting = false
enable_service_discovery = false

# Halcyon-specific:
enable_halcyon = true
halcyon_bundle_fonts = true
halcyon_default_search_engine = "duckduckgo"
halcyon_quiet_mode_default = true

# Optional: enable more diagnostics during Halcyon development:
# halcyon_strict_voice_check = true   # (build-time check on strings)
```

### Halcyon-specific gn args

| Arg | Default | Purpose |
|-----|---------|---------|
| `enable_halcyon` | `false` | When true, the build produces the Halcyon browser instead of upstream Chrome. |
| `halcyon_bundle_fonts` | `false` | When true, the Inter / Inter Tight / Source Serif 4 / JetBrains Mono woff2 files are bundled into the build's `chrome://resources/halcyon_shared/fonts/` directory. |
| `halcyon_default_search_engine` | `"duckduckgo"` | The default search engine for new users. Must be one of: `duckduckgo`, `google`, `brave`, `startpage`, `mojeek`, `halcyon`. |
| `halcyon_quiet_mode_default` | `true` | Whether Quiet Mode is on for new users. |
| `halcyon_strict_voice_check` | `false` | When true, the build fails on any Halcyon string that contains an exclamation mark, "Welcome", or another banned word. Used during development. |

---

## Building

```bash
ninja -C out/Halcyon chrome
```

This builds the Halcyon browser. The first clean build takes 6–12 hours
depending on your hardware. Subsequent incremental builds are fast
(minutes).

If you have a Goma account:

```bash
gn gen out/Halcyon --args='
  ...
  use_goma = true
'
ninja -C out/Halcyon chrome -j300
```

---

## Bundling the fonts

Halcyon uses four fonts under the SIL Open Font License. They are not
included in this repo (the repo's design system references them by
filename, but the binary font files themselves are external). A helper
script downloads and places them:

```bash
python3 src/halcyon/build/fetch_fonts.py
```

This script downloads:

- Inter (Regular, Medium, SemiBold) → `chrome/browser/resources/halcyon_shared/fonts/Inter-Regular.woff2`, etc.
- Inter Tight (Medium, SemiBold) → `.../InterTight-Medium.woff2`, etc.
- Source Serif 4 (Italic Regular, Italic Medium) → `.../SourceSerif4-Italic-Regular.woff2`, etc.
- JetBrains Mono (Regular) → `.../JetBrainsMono-Regular.woff2`

from Google Fonts. The total size is ~240 KB.

The script is idempotent: re-running it is safe and skips files that
already exist.

---

## Running

After a successful build:

```bash
out/Halcyon/chrome        # Linux
out/Halcyon/Halcyon.app/Contents/MacOS/Halcyon   # macOS
out/Halcyon/chrome.exe    # Windows
```

The binary is named `chrome` upstream; `patches/rename_output_to_halcyon.patch`
renames it to `Halcyon` if you prefer.

For a clean test profile (recommended for Halcyon development):

```bash
out/Halcyon/chrome --user-data-dir=/tmp/halcyon-dev-profile
```

---

## Build troubleshooting

### `error: 'halcyon_color_palette.h' file not found`

The Halcyon source tree wasn't applied at the right path. Run
`IMPLEMENTATION.md` Step 2 again.

### `undefined reference to 'halcyon::HalcyonBrowserView::Init(...)'`

The patch `patches/gyp_enable_halcyon.patch` didn't include the Halcyon
sources in the build graph. Re-apply it.

### `chrome://newtab` shows the upstream Chrome NTP

The `patches/webui_register_halcyon_ntp.patch` didn't apply. Re-apply it.

### Build succeeds but the binary crashes on launch with `dlopen: ...libffmpeg.so...`

Standard Chromium issue, unrelated to Halcyon. Run
`chrome --no-sandbox` once to populate the system fonts cache, then
re-launch normally.

### `error: unrecognized command-line option '-enable-halcyon'`

You passed a GN arg as a command-line flag. GN args go in `args.gn`, not
on the `chrome` command line.

---

## Performance

Halcyon's runtime performance is identical to upstream Chrome's: the
engine is unchanged, and the chrome (Views + WebUI) is a small fraction
of the total cost.

Build-time performance is also essentially identical. The Halcyon
sources are a few thousand lines; the upstream Chromium tree is several
million. Adding Halcyon doesn't materially change build times.
