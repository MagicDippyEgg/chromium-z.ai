#!/usr/bin/env bash
# Copyright 2026 The Halcyon Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# package_halcyon.sh — package a built Halcyon browser into a downloadable
# archive that contains everything needed to run the browser on a clean
# Linux x86_64 machine.
#
# Usage:
#   package_halcyon.sh <build_out_dir> <dest_dir> [archive_path]
#
# Where:
#   <build_out_dir>  the Chromium build output directory
#                    (e.g. /path/to/chromium/src/out/Halcyon)
#   <dest_dir>       the staging directory for the packaged browser
#                    (e.g. ./halcyon-linux-x86_64)
#   [archive_path]   if given, also create a tar.gz at this path
#
# The script copies:
#   - The chrome binary itself (renamed to "halcyon" via a wrapper, with
#     the original "chrome" kept for compatibility with extension shell
#     integrations and scripts that look for a binary literally named
#     "chrome")
#   - All shared libraries (.so) the build produced
#   - The resource bundles (.pak, .dat, .bin) that contain locale strings,
#     the ICU tables, the V8 snapshot, and the Halcyon WebUI pages
#   - The locales/ directory (per-locale .pak files)
#   - The chrome-sandbox helper (setuid sandbox) so the browser can run
#     sandboxed without --no-sandbox
#   - The Libredirect helper (libffmpeg.so, libqt5_shim.so if present)
#   - A small wrapper script that launches the browser with sensible
#     defaults (no auto-update, no migration, clean profile by default)
#   - A README with launch instructions
#
# The script FAILS if any of the required runtime files are missing —
# we will not upload an incomplete package.
#
# The script is intentionally a single bash file with no dependencies
# beyond coreutils, gzip, and tar. It runs on any modern Linux.

set -euo pipefail

# --- Inputs --------------------------------------------------------------

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <build_out_dir> <dest_dir> [archive_path]"
  echo ""
  echo "Example:"
  echo "  $0 /path/to/chromium/src/out/Halcyon ./halcyon-linux-x86_64"
  echo "  $0 /path/to/chromium/src/out/Halcyon ./halcyon-linux-x86_64 halcyon-linux-x86_64.tar.gz"
  exit 2
fi

BUILD_OUT="$1"
DEST_DIR="$2"
ARCHIVE_PATH="${3:-}"

if [[ ! -d "$BUILD_OUT" ]]; then
  echo "error: build output directory does not exist: $BUILD_OUT" >&2
  exit 1
fi

# --- Verifications -------------------------------------------------------

# The chrome binary MUST exist; we will not package without it.
if [[ ! -x "$BUILD_OUT/chrome" ]]; then
  echo "error: chrome binary not found at $BUILD_OUT/chrome" >&2
  echo "  Did the build complete? Check the ninja output." >&2
  exit 1
fi

# A handful of runtime files are strictly required for the browser to
# start at all. If any are missing the build is incomplete.
REQUIRED_FILES=(
  "chrome"               # The main browser binary
  "icudtl.dat"            # ICU data table (no this = no rendering of any non-ASCII text)
  "resources.pak"         # Composite resource bundle (UI strings, icons, HTML)
  "chrome_100_percent.pak"
  "chrome_200_percent.pak"
  "snapshot_blob.bin"     # V8 startup snapshot
  "v8_context_snapshot.bin"
)
for f in "${REQUIRED_FILES[@]}"; do
  if [[ ! -f "$BUILD_OUT/$f" ]]; then
    echo "error: required runtime file missing: $f" >&2
    echo "  The build is incomplete. Refusing to package." >&2
    exit 1
  fi
done

# Locales directory must exist with at least en-US.pak.
if [[ ! -f "$BUILD_OUT/locales/en-US.pak" ]]; then
  echo "error: locales/en-US.pak missing" >&2
  exit 1
fi

echo "ok: all required runtime files present"

# --- Stage the package ---------------------------------------------------

rm -rf "$DEST_DIR"
mkdir -p "$DEST_DIR" "$DEST_DIR/locales"

# 1. The chrome binary. We keep the name "chrome" because:
#    - Extension manifests reference chrome as the launcher on Linux.
#    - Desktop-file associations and protocol handlers may use it.
#    - The Halcyon chrome renders on top of Chromium's "chrome" target.
#    We also install an "halcyon" symlink so users can invoke it by the
#    product name.
cp -av "$BUILD_OUT/chrome" "$DEST_DIR/chrome"
ln -sf chrome "$DEST_DIR/halcyon"

# 2. The sandbox helper (setuid). Optional but recommended.
if [[ -x "$BUILD_OUT/chrome_sandbox" ]]; then
  cp -av "$BUILD_OUT/chrome_sandbox" "$DEST_DIR/chrome_sandbox"
  chmod 4755 "$DEST_DIR/chrome_sandbox" 2>/dev/null || true
fi

# 3. The crashpad handler (out-of-process crash dumper).
if [[ -x "$BUILD_OUT/chrome_crashpad_handler" ]]; then
  cp -av "$BUILD_OUT/chrome_crashpad_handler" "$DEST_DIR/chrome_crashpad_handler"
fi

# 4. All shared libraries produced by a component build.
#    In a static build there will be few or none; in a component build
#    the bulk of the runtime lives here.
shopt -s nullglob
for so in "$BUILD_OUT"/lib*.so "$BUILD_OUT"/lib*.so.*; do
  cp -av "$so" "$DEST_DIR/"
done

# 5. Resource bundles.
for f in "$BUILD_OUT"/*.pak "$BUILD_OUT"/*.dat "$BUILD_OUT"/*.bin; do
  [[ -f "$f" ]] && cp -av "$f" "$DEST_DIR/"
done

# 6. Locales (only the en-US plus any others present — keeps the
#    archive smaller than bundling all 100+ locales).
for f in "$BUILD_OUT"/locales/*.pak; do
  cp -av "$f" "$DEST_DIR/locales/"
done

# 7. Optional: Vulkan ICD binaries (newer Chromium uses Vulkan for some
#    compositing paths; bundled ICD lets the browser run on systems
#    without a Vulkan driver).
if [[ -d "$BUILD_OUT/vulkan" ]]; then
  mkdir -p "$DEST_DIR/vulkan"
  cp -av "$BUILD_OUT"/vulkan/* "$DEST_DIR/vulkan/" 2>/dev/null || true
fi

# 8. Optional: Widevine CDM (DRM for premium video). We bundle if
#    present; some builds ship without it.
if [[ -d "$BUILD_OUT/widevine" ]]; then
  cp -av "$BUILD_OUT/widevine" "$DEST_DIR/"
fi

# 9. Optional: notification helper (libnotify bridge).
for helper in notification_helper xdisplaycheck; do
  if [[ -x "$BUILD_OUT/$helper" ]]; then
    cp -av "$BUILD_OUT/$helper" "$DEST_DIR/$helper"
  fi
done

shopt -u nullglob

# --- Verify Halcyon branding is present ---------------------------------

# The Halcyon string overrides live in chrome/app/chromium_strings.grd.halcyon
# and chrome/app/generated_resources/generated_resources.grd.halcyon, applied
# via patches/theme_use_halcyon.patch (and friends). The strings get baked
# into resources.pak at build time.
#
# We verify them by grepping the binary AND the pak for "Halcyon". If
# neither contains the string, the branding patches didn't apply and we
# accidentally packaged stock Chromium — fail the build.

BRAND_OK=0
if strings "$DEST_DIR/chrome" 2>/dev/null | grep -qi "Halcyon"; then
  echo "ok: chrome binary contains 'Halcyon' branding"
  BRAND_OK=1
fi
if strings "$DEST_DIR/resources.pak" 2>/dev/null | grep -qi "Halcyon"; then
  echo "ok: resources.pak contains 'Halcyon' branding"
  BRAND_OK=1
fi
if [[ $BRAND_OK -eq 0 ]]; then
  echo "error: Halcyon branding not found in the binary or resources.pak" >&2
  echo "  This means the brand-override patches didn't apply or didn't" >&2
  echo "  take effect. Refusing to upload stock Chromium as Halcyon." >&2
  exit 1
fi

# --- Write the launch wrapper ------------------------------------------

# A small wrapper that:
#   - Points the binary at its own directory for resources (CHROME_DIR)
#   - Sets a sensible default user-data-dir under the user's home
#   - Disables the auto-update check (we ship via the artifact, not
#     Google's update server)
cat > "$DEST_DIR/halcyon-launcher" <<EOF
#!/usr/bin/env bash
# Halcyon launcher.
# Wraps the chrome binary with sensible defaults so the browser works
# out of the box on any Linux system without system-wide installation.
set -e
HERE="\$(cd "\$(dirname "\$0")" && pwd)"
exec "\$HERE/chrome" \\
  --user-data-dir="\${HALCYON_USER_DATA_DIR:-\$HOME/.halcyon/profile}" \\
  --disable-domain-reliability \\
  --no-pings \\
  --no-default-browser-check \\
  --no-first-run \\
  "\$@"
EOF
chmod +x "$DEST_DIR/halcyon-launcher"

# --- Write the README ---------------------------------------------------

cat > "$DEST_DIR/README.txt" <<'EOF'
Halcyon Browser — Linux x86_64 build
====================================

Halcyon is a calm browser built on Chromium. The engine is Blink; the
JavaScript engine is V8. The chrome (toolbar, tabs, omnibox, menus,
settings, NTP, history, bookmarks, private browsing) is the Halcyon
redesign — see the README.md in the source repository for the full
story.

To launch
---------

  tar -xzf halcyon-linux-x86_64.tar.gz
  cd halcyon-linux-x86_64
  ./halcyon-launcher

Or, equivalently:

  ./chrome --user-data-dir=$HOME/.halcyon/profile --no-first-run --no-default-browser-check

The first launch creates a fresh user profile in $HOME/.halcyon/profile.
To use a different profile location, set HALCYON_USER_DATA_DIR:

  HALCYON_USER_DATA_DIR=/tmp/halcyon-test ./halcyon-launcher

What's in this archive
----------------------

  chrome                      The browser binary (the Halcyon chrome
                              rendered on top of the Chromium engine).
  halcyon                     A symlink to chrome. Invoke by either name.
  halcyon-launcher            A wrapper that launches chrome with the
                              right resource paths and a clean default
                              profile directory.
  chrome_sandbox              The setuid sandbox helper. Allows the
                              browser to use the sandbox without
                              running as root.
  chrome_crashpad_handler     The out-of-process crash dumper.
  lib*.so                     Shared libraries produced by the build.
  *.pak                       Resource bundles (UI strings, icons, the
                              Halcyon WebUI pages).
  *.dat                       ICU data table (icudtl.dat).
  *.bin                       V8 startup snapshots.
  locales/*.pak                Per-locale UI string bundles.
  README.txt                  This file.

Notes
-----

  - The binary is named "chrome" rather than "halcyon" so that Linux
    desktop integration (.desktop files, MIME handlers, extension
    shell scripts) that look for "chrome" still work.

  - The Halcyon chrome renders on top of the Chromium engine. To
    verify you're running Halcyon and not stock Chromium, open
    chrome://halcyon — the Halcyon About page is a single page of
    plain text and says "Halcyon is a calm browser built on
    Chromium." If you see the upstream Chromium About page instead,
    the brand-override patches didn't apply.

  - Halcyon ships with Quiet Mode on by default. Notifications,
    popups, and autoplay are muted across all tabs. Toggle with
    Ctrl+Shift+M.

Source code
-----------

The Halcyon source tree is at https://github.com/MagicDippyEgg/chromium-z.ai
The build instructions are in IMPLEMENTATION.md and halcyon/build/BUILD.md.
EOF

# --- Verify the staged package -----------------------------------------

echo ""
echo "=== packaged files ==="
ls -la "$DEST_DIR"
echo ""
echo "=== total size ==="
du -sh "$DEST_DIR"

# Sanity check: the staged directory should have at least the chrome
# binary and the resources.pak. (We already verified these above; this is
# just a paranoia check that the copy didn't fail silently.)
[[ -x "$DEST_DIR/chrome" ]] || { echo "error: chrome binary missing from staged dir"; exit 1; }
[[ -f "$DEST_DIR/resources.pak" ]] || { echo "error: resources.pak missing from staged dir"; exit 1; }

# --- Optionally create the archive --------------------------------------

if [[ -n "$ARCHIVE_PATH" ]]; then
  echo ""
  echo "=== creating archive $ARCHIVE_PATH ==="
  # Use pigz if available (parallel gzip) for faster compression; fall
  # back to gzip.
  if command -v pigz >/dev/null 2>&1; then
    tar -I pigz -cf "$ARCHIVE_PATH" -C "$(dirname "$DEST_DIR")" "$(basename "$DEST_DIR")"
  else
    tar -czf "$ARCHIVE_PATH" -C "$(dirname "$DEST_DIR")" "$(basename "$DEST_DIR")"
  fi
  ls -lh "$ARCHIVE_PATH"
fi

echo ""
echo "ok: package complete"
