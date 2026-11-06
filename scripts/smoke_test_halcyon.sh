#!/usr/bin/env bash
# Copyright 2026 The Halcyon Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# smoke_test_halcyon.sh — verify a built Halcyon browser launches and
# renders. Runs in a CI environment with no display.
#
# Usage:
#   smoke_test_halcyon.sh <packaged_dir>
#
# Where <packaged_dir> is the staged browser directory produced by
# package_halcyon.sh (containing chrome + resources.pak + locales/ etc.).
#
# The script runs four tests, in order, and exits non-zero on the first
# failure:
#
#   1. Version check. The browser must print a version string. This
#      verifies the binary launches and the dynamic linker can find
#      every .so it needs.
#
#   2. About-page check. The chrome://halcyon page must load and contain
#      the literal string "Halcyon". This verifies that the Halcyon
#      WebUI registration patch (patches/webui_register_halcyon_pages.patch)
#      applied, the HalcyonAboutUI controller is built, and the about
#      page resources are bundled.
#
#   3. Render check. The browser must dump the DOM of about:blank. This
#      verifies that Blink, V8, and the compositor all start up — i.e.,
#      the browser is actually a browser, not just an empty shell.
#
#   4. Screenshot check. The browser must produce a non-empty screenshot
#      of about:blank. This verifies the GPU path works (with --swiftshader
#      as the software fallback).
#
# All four tests use --no-sandbox because the GitHub Actions runner
# doesn't allow setuid chrome-sandbox. The user-data-dir is set to a
# throwaway tmp directory so the test doesn't pollute the runner's
# $HOME.

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <packaged_dir>"
  exit 2
fi

PKG_DIR="$1"
CHROME="$PKG_DIR/chrome"
TMP_PROFILE="$(mktemp -d -t halcyon-smoke-XXXXXX)"
trap 'rm -rf "$TMP_PROFILE"' EXIT

if [[ ! -x "$CHROME" ]]; then
  echo "error: $CHROME not executable" >&2
  exit 1
fi

# Common flags. --no-sandbox is required on GitHub runners (no setuid
# support). --disable-gpu + --use-gl=swiftshader gives us a software
# OpenGL implementation that works in a headless CI container.
COMMON_FLAGS=(
  --no-sandbox
  --no-first-run
  --no-default-browser-check
  --disable-gpu
  --use-gl=swiftshader
  --enable-unsafe-swiftshader
  --user-data-dir="$TMP_PROFILE"
  --disable-extensions
  --enable-features=HalcyonUI
  --disable-features=DialMediaRouteProvider,TranslateUI
)

echo "=== smoke test 1/4: version check ==="
# --version should print immediately and exit 0.
"$CHROME" "${COMMON_FLAGS[@]}" --version
echo "ok: version printed"
echo ""

echo "=== smoke test 2/4: chrome://halcyon page loads and contains 'Halcyon' ==="
# Dump the DOM of the Halcyon about page. We expect to find the word
# "Halcyon" in the output.
ABOUT_DOM="$("$CHROME" "${COMMON_FLAGS[@]}" --headless --dump-dom chrome://halcyon 2>/dev/null)" || true
if [[ -z "$ABOUT_DOM" ]]; then
  echo "error: chrome://halcyon returned no DOM" >&2
  exit 1
fi
if ! grep -q "Halcyon" <<<"$ABOUT_DOM"; then
  echo "error: chrome://halcyon loaded but does not contain the string 'Halcyon'" >&2
  echo "  Got $(echo "$ABOUT_DOM" | wc -c) bytes of DOM; first 500:" >&2
  echo "$ABOUT_DOM" | head -c 500 >&2
  exit 1
fi
echo "ok: chrome://halcyon contains 'Halcyon'"
echo ""

echo "=== smoke test 3/4: render about:blank (Blink + V8 startup) ==="
BLANK_DOM="$("$CHROME" "${COMMON_FLAGS[@]}" --headless --dump-dom about:blank 2>/dev/null)" || true
if [[ -z "$BLANK_DOM" ]]; then
  echo "error: about:blank returned no DOM" >&2
  exit 1
fi
# about:blank should produce an empty-but-valid HTML document.
if ! grep -qi "<html" <<<"$BLANK_DOM"; then
  echo "error: about:blank DOM does not contain <html> tag" >&2
  echo "  Got: $BLANK_DOM" >&2
  exit 1
fi
echo "ok: about:blank rendered with <html> document"
echo ""

echo "=== smoke test 4/4: screenshot of about:blank (GPU path) ==="
SHOT="/tmp/halcyon-smoke-$(date +%s).png"
"$CHROME" "${COMMON_FLAGS[@]}" --headless --screenshot="$SHOT" --window-size=800,600 about:blank 2>/dev/null || true
if [[ ! -s "$SHOT" ]]; then
  echo "error: screenshot was not produced (or is empty)" >&2
  exit 1
fi
# Verify the PNG is a real image (PNG header).
if ! head -c 8 "$SHOT" | od -An -c | grep -q "P   N   G"; then
  echo "error: screenshot file is not a valid PNG" >&2
  exit 1
fi
SIZE=$(stat -c%s "$SHOT")
echo "ok: screenshot produced ($SIZE bytes)"
rm -f "$SHOT"

echo ""
echo "ok: all smoke tests passed"
