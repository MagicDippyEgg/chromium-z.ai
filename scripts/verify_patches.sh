#!/bin/bash
# Verify all 8 Halcyon patches apply cleanly in order against the
# pinned Chromium revision.
#
# Usage: ./verify_patches.sh

set -e

PINNED_SHA="ff897141277d798d72ea3ae0a94ff3a51956b267"
WORK="/tmp/halcyon-verify"
PATCHES_DIR="/home/z/my-project/chromium-z.ai/patches"

# The patches MUST be applied in this exact order. The patches/
# directory has a README that documents this.
PATCH_ORDER=(
  "gyp_enable_halcyon.patch"
  "chrome_command_ids_halcyon.patch"
  "theme_use_halcyon.patch"
  "browser_view_replace_toolbar.patch"
  "browser_view_replace_tab_strip.patch"
  "webui_register_halcyon_ntp.patch"
  "webui_register_halcyon_pages.patch"
  "context_menu_add_margin_note.patch"
)

echo "=== Verifying Halcyon integration patches ==="
echo "  pinned SHA: $PINNED_SHA"
echo "  work dir:   $WORK"
echo ""

# Set up a fresh sparse checkout of the pinned revision
rm -rf "$WORK"
mkdir -p "$WORK"
cd "$WORK"
git init -q
git sparse-checkout init --cone
git sparse-checkout set \
  chrome/app \
  chrome/browser/ui \
  chrome/browser/themes \
  chrome/common \
  build/config \
  components/renderer_context_menu \
  ui/base
git remote add origin https://chromium.googlesource.com/chromium/src.git
echo "  fetching pinned revision..."
git fetch --depth 1 --filter=blob:none origin "$PINNED_SHA" -q
git checkout FETCH_HEAD -q
echo "  fetched: $(git rev-parse HEAD)"
echo ""

# Apply each patch in order
ALL_OK=1
for p in "${PATCH_ORDER[@]}"; do
  PATCH_PATH="$PATCHES_DIR/$p"
  if [ ! -s "$PATCH_PATH" ]; then
    echo "  [SKIP] $p (empty/deferred patch — use --allow-empty)"
    git apply --allow-empty "$PATCH_PATH" || { echo "  [FAIL] $p"; ALL_OK=0; }
    continue
  fi
  if git apply --check "$PATCH_PATH" 2>&1; then
    git apply "$PATCH_PATH"
    echo "  [OK]   $p"
  else
    echo "  [FAIL] $p"
    ALL_OK=0
  fi
done

echo ""
if [ "$ALL_OK" -eq 1 ]; then
  echo "=== ALL 8 PATCHES APPLY CLEANLY ==="
else
  echo "=== SOME PATCHES FAILED ==="
  exit 1
fi

# Verify all Halcyon changes are present in the working tree
echo ""
echo "=== verifying Halcyon symbols are present ==="
echo "  enable_halcyon imported in BUILD.gn:        $(grep -c 'halcyon_features.gni' chrome/browser/ui/BUILD.gn)"
echo "  source_set('halcyon') in BUILD.gn:            $(grep -c 'source_set(\"halcyon\")' chrome/browser/ui/BUILD.gn)"
echo "  kHalcyonUI in chrome_features.cc:            $(grep -c 'kHalcyonUI' chrome/common/chrome_features.cc)"
echo "  kHalcyonUI declared in chrome_features.h:    $(grep -c 'kHalcyonUI' chrome/common/chrome_features.h)"
echo "  IDC_HALCYON_* in chrome_command_ids.h:       $(grep -c 'IDC_HALCYON' chrome/app/chrome_command_ids.h)"
echo "  HalcyonTheme in theme_service_factory.cc:    $(grep -c 'HalcyonTheme' chrome/browser/themes/theme_service_factory.cc)"
echo "  HalcyonToolbarView in browser_view.cc:       $(grep -c 'HalcyonToolbarView' chrome/browser/ui/views/frame/browser_view.cc)"
echo "  HalcyonTabStrip in browser_view.cc:          $(grep -c 'HalcyonTabStrip' chrome/browser/ui/views/frame/browser_view.cc)"
echo "  HalcyonNtpUI in chrome_web_ui_configs.cc:    $(grep -c 'HalcyonNtpUI' chrome/browser/ui/webui/chrome_web_ui_configs.cc)"
echo "  HalcyonAboutUI in chrome_web_ui_configs.cc:  $(grep -c 'HalcyonAboutUI' chrome/browser/ui/webui/chrome_web_ui_configs.cc)"
echo "  HalcyonQuillUI in chrome_web_ui_configs.cc:  $(grep -c 'HalcyonQuillUI' chrome/browser/ui/webui/chrome_web_ui_configs.cc)"
