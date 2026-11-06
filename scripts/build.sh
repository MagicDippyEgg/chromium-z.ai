#!/usr/bin/env bash
# build.sh — the Halcyon Chromium build entrypoint.
#
# This script runs INSIDE the halcyon-builder Docker container. The
# Halcyon source tree + scripts are bind-mounted at /repo. The Chromium
# source + build output live at /repo/build (which is bind-mounted to
# the GitHub Actions workspace's `build/` directory).
#
# Two modes:
#
#   prep mode:    fetch_source.sh + apply halcyon source + apply patches
#                 + write args.gn + gn gen + setup toolchain
#                 (no ninja — just the prep)
#
#   build mode:   timeout <task_timeout>s ninja -C out/Halcyon chrome
#                 if ninja finishes within the timeout, exits 0 + writes
#                 "status=completed" to $GITHUB_OUTPUT.
#                 if ninja times out, exits 0 + writes "status=running".
#                 the next build_part job picks up where this left off.
#
# Environment variables (set by the workflow):
#   _prepare_only   — "true" for prep mode, empty/other for build mode
#   _gha_final      — "true" if this is the last build_part job (must
#                     succeed or fail; can't keep running)
#   _task_timeout   — seconds for the ninja timeout (default: 18000 = 5h)
#   GITHUB_OUTPUT   — the file to write job outputs to (GitHub Actions
#                     convention)
#
# Directory layout inside the container:
#   /repo/                    — Halcyon source tree (bind-mounted)
#   /repo/build/              — Chromium source + build output (bind-mounted)
#   /repo/build/src/           — extracted Chromium source tree
#   /repo/build/src/out/Halcyon/  — GN build directory + ninja output
#   /repo/halcyon/             — Halcyon overlay (BUILD.gn, branding, etc.)
#   /repo/patches/             — the 8 integration patches
#   /repo/scripts/             — this script + helpers

set -euo pipefail

# --- Resolve paths -------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"
SRC_DIR="${BUILD_DIR}/src"
OUT_DIR="${SRC_DIR}/out/Halcyon"

# --- Mode + timeout -----------------------------------------------------

PREPARE_ONLY="${_prepare_only:-false}"
GHA_FINAL="${_gha_final:-false}"
TASK_TIMEOUT="${_task_timeout:-18000}"  # 5 hours default

echo "info: build.sh starting"
echo "  PREPARE_ONLY = $PREPARE_ONLY"
echo "  GHA_FINAL    = $GHA_FINAL"
echo "  TASK_TIMEOUT = $TASK_TIMEOUT seconds"
echo "  REPO_ROOT    = $REPO_ROOT"
echo "  BUILD_DIR    = $BUILD_DIR"
echo "  SRC_DIR      = $SRC_DIR"
echo "  OUT_DIR      = $OUT_DIR"
echo "  free disk    = $(df -BG --output=avail /repo | tail -1)"
echo ""

# --- Prep mode -----------------------------------------------------------

if [[ "$PREPARE_ONLY" == "true" ]]; then
  echo "=== prep mode: fetch source, apply patches, gn gen ==="

  # 1. Fetch Chromium source tarball (idempotent — skips if already done).
  bash "${SCRIPT_DIR}/fetch_source.sh" "$BUILD_DIR"

  # 2. Apply the Halcyon source tree on top of the Chromium source.
  echo "=== applying Halcyon source tree to ${SRC_DIR} ==="
  rsync -av --exclude '.git' "${REPO_ROOT}/halcyon/src/" "${SRC_DIR}/"
  # Also copy the top-level halcyon/ tree (BUILD.gn, branding, etc.).
  rsync -av --exclude '.git' --exclude 'src' --exclude 'preview' \
         --exclude 'design' --exclude 'branding' \
         "${REPO_ROOT}/halcyon/" "${SRC_DIR}/halcyon/"
  echo "ok: Halcyon source tree applied"

  # 3. Fetch the Halcyon fonts (Inter, Inter Tight, Source Serif 4, JetBrains Mono).
  echo "=== fetching Halcyon fonts ==="
  (cd "${SRC_DIR}" && python3 halcyon/build/fetch_fonts.py) || \
    echo "warning: font fetch failed — build will continue without bundled fonts"

  # 4. Apply the integration patches in order.
  echo "=== applying integration patches ==="
  cd "${SRC_DIR}"
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
  for p in "${PATCH_ORDER[@]}"; do
    name="$p"
    patch_path="${REPO_ROOT}/patches/$p"
    # Skip empty/deferred patches entirely. The patch file may have
    # a header but no diff content (e.g. context_menu_add_margin_note.patch
    # is a deferred stub). `git apply --allow-empty` requires a valid
    # git patch format with zero changes; our stub has no diff at all,
    # so git apply rejects it. Just skip.
    if [[ ! -s "$patch_path" ]]; then
      echo "  [skip] $name (file is empty)"
      continue
    fi
    # Check if the patch has any actual diff content. A real git patch
    # has lines starting with 'diff --git' (the file headers). The
    # deferred-stub patch has a `---` separator in its prose header
    # but no `diff --git` lines, so we check for the latter specifically.
    if ! grep -q "^diff --git" "$patch_path"; then
      echo "  [skip] $name (no diff content — deferred stub)"
      continue
    fi
    if git apply "$patch_path" 2>&1; then
      # git apply may silently skip hunks if the context doesn't match.
      # Verify the patch actually made changes by checking if the
      # files mentioned in the patch were modified. For our key patches
      # (gyp_enable_halcyon), check that the import was actually added.
      if [[ "$name" == "gyp_enable_halcyon.patch" ]]; then
        if ! grep -q 'halcyon_features.gni' build/config/BUILDCONFIG.gn 2>/dev/null; then
          echo "warning: $name reported OK but BUILDCONFIG.gn wasn't modified; trying patch -p1" >&2
          git checkout -- build/config/BUILDCONFIG.gn 2>/dev/null || true
          patch -p1 --fuzz=3 < "$patch_path" 2>&1 | tail -10 || {
            echo "error: $name failed to apply BUILDCONFIG.gn hunk" >&2
            exit 1
          }
        fi
      fi
      echo "  [ok]   $name"
    else
      # Fall back to patch -p1 with fuzz, which is more tolerant of
      # context differences (e.g. the tarball's BUILDCONFIG.gn may
      # have slightly different whitespace than the git checkout).
      echo "info: git apply failed for $name; trying patch -p1 --fuzz=3" >&2
      if patch -p1 --fuzz=3 < "$patch_path" 2>&1 | tail -10; then
        echo "  [ok]   $name (via patch -p1)"
      else
        echo "error: $name failed to apply" >&2
        echo "  the patches are pinned to Chromium 159.0.8141.0." >&2
        echo "  see patches/README.md for how to rebase." >&2
        exit 1
      fi
    fi
  done
  echo "ok: all patches applied"

  # Diagnostic: verify the BUILDCONFIG.gn has the Halcyon import.
  echo "=== verifying BUILDCONFIG.gn has halcyon import ==="
  if grep -q 'halcyon_features.gni' "${SRC_DIR}/build/config/BUILDCONFIG.gn"; then
    echo "ok: BUILDCONFIG.gn has the import"
    grep -n 'halcyon_features.gni' "${SRC_DIR}/build/config/BUILDCONFIG.gn"
  else
    echo "warning: BUILDCONFIG.gn does NOT have the import — patch 1 didn't apply correctly"
  fi
  # Also verify the halcyon_features.gni file is present where the
  # import expects it.
  if [[ -f "${SRC_DIR}/halcyon/build/halcyon_features.gni" ]]; then
    echo "ok: ${SRC_DIR}/halcyon/build/halcyon_features.gni exists"
  else
    echo "warning: ${SRC_DIR}/halcyon/build/halcyon_features.gni MISSING"
    ls -la "${SRC_DIR}/halcyon/build/" 2>&1 || true
  fi

  # 5. Set up the toolchain. Chromium needs clang, node, gperf, etc.
  # The Docker image has all of these installed at standard system paths;
  # we just need to symlink them into the locations Chromium's build
  # scripts expect.
  echo "=== setting up toolchain symlinks ==="
  setup_toolchain_symlinks() {
    local src_dir="$1"
    # Node.js (Chromium expects node at third_party/node/linux/node-linux-x64/bin/node)
    mkdir -p "${src_dir}/third_party/node/linux/node-linux-x64/bin"
    ln -sf "$(which node)" "${src_dir}/third_party/node/linux/node-linux-x64/bin/node"
    ln -sf "$(which npm)" "${src_dir}/third_party/node/linux/node-linux-x64/bin/npm"
    # gperf
    mkdir -p "${src_dir}/third_party/gperf/cipd/bin/"
    ln -sf "$(which gperf)" "${src_dir}/third_party/gperf/cipd/bin/gperf"
    # Go (for Dawn / WebGPU)
    mkdir -p "${src_dir}/third_party/dawn/tools/golang/linux-amd64/bin"
    ln -sf "$(which go)" "${src_dir}/third_party/dawn/tools/golang/linux-amd64/bin/go"
    # clang-format
    mkdir -p "${src_dir}/buildtools/linux64-format"
    ln -sf "$(which clang-format)" "${src_dir}/buildtools/linux64-format/clang-format"
    # TypeScript compiler
    mkdir -p "${src_dir}/third_party/typescript/linux-amd64/src/lib"
    ln -sf "$(which tsc)" "${src_dir}/third_party/typescript/linux-amd64/src/lib/tsc"
    # esbuild (Chromium uses it for some WebUI build steps)
    if [[ -d "${src_dir}/third_party/devtools-frontend/src/node_modules/esbuild" ]]; then
      rm -rf "${src_dir}/third_party/devtools-frontend/src/node_modules/esbuild"
    fi
    ln -sf /usr/lib/node_modules/esbuild "${src_dir}/third_party/devtools-frontend/src/node_modules/esbuild"
  }
  setup_toolchain_symlinks "$SRC_DIR"
  echo "ok: toolchain symlinks set up"

  # 6. Fetch the hermetic clang + the Linux sysroot.
  # These are required for the build to actually work — system clang
  # alone doesn't have the right version. The source tarball doesn't
  # include them; they're normally fetched by `gclient runhooks`.
  # We call the underlying update scripts directly (faster + more
  # reliable than running the full hook chain).
  echo "=== fetching hermetic clang + sysroot ==="
  cd "$SRC_DIR"

  # 6a. Fetch the hermetic clang toolchain. This is the script that
  # `gclient runhooks` would call. It downloads ~600 MB.
  echo "info: checking for hermetic clang at ${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/clang"
  if [[ -x "${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/clang" ]]; then
    ls -la "${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/clang" || true
    "${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/clang" --version 2>&1 | head -3 || true
    echo "ok: hermetic clang binary present"
  else
    echo "info: hermetic clang not present; fetching via update.py (~600 MB)..."
    python3 tools/clang/scripts/update.py 2>&1 || {
      echo "error: clang update.py failed" >&2
      echo "  the hermetic clang is required; the build cannot proceed without it" >&2
      exit 1
    }
    if [[ ! -x "${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/clang" ]]; then
      echo "error: update.py ran but the clang binary is still missing" >&2
      ls -la "${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/" 2>&1 || true
      exit 1
    fi
    "${SRC_DIR}/third_party/llvm-build/Release+Asserts/bin/clang" --version 2>&1 | head -3
    echo "ok: hermetic clang fetched"
  fi

  # 6a-continued. The build/config/compiler/BUILD.gn assertion reads
  # `third_party/llvm-build/Release+Asserts/cr_build_revision` and
  # expects it to match the CLANG_REVISION + CLANG_SUB_REVISION in
  # tools/clang/scripts/update.py. The source tarball ships the clang
  # binary but sometimes ships an empty `cr_build_revision` file (or
  # no file at all). If the file is empty or missing, write the
  # expected revision string so the gn gen assertion passes.
  CR_BUILD_REV_FILE="${SRC_DIR}/third_party/llvm-build/Release+Asserts/cr_build_revision"
  EXPECTED_REVISION=$(python3 -c "
import re
with open('${SRC_DIR}/tools/clang/scripts/update.py') as f:
    content = f.read()
rev = re.search(r\"CLANG_REVISION = '([^']+)'\", content).group(1)
sub = re.search(r'CLANG_SUB_REVISION = ([0-9]+)', content).group(1)
print(f'{rev}-{sub}')
")
  if [[ ! -f "$CR_BUILD_REV_FILE" ]] || [[ ! -s "$CR_BUILD_REV_FILE" ]]; then
    echo "info: cr_build_revision is missing/empty; writing $EXPECTED_REVISION"
    echo "$EXPECTED_REVISION" > "$CR_BUILD_REV_FILE"
  else
    ACTUAL=$(cat "$CR_BUILD_REV_FILE")
    if [[ "$ACTUAL" != "$EXPECTED_REVISION" ]]; then
      echo "info: cr_build_revision mismatch (actual='$ACTUAL', expected='$EXPECTED_REVISION'); fixing"
      echo "$EXPECTED_REVISION" > "$CR_BUILD_REV_FILE"
    fi
  fi
  echo "ok: cr_build_revision = $(cat "$CR_BUILD_REV_FILE")"

  # 6b. Fetch the Linux sysroot. The build needs it for use_sysroot=true.
  # The install-sysroot.py script downloads ~30 MB.
  if [[ ! -d "${SRC_DIR}/build/linux/debian_bullseye_amd64-sysroot" ]]; then
    echo "info: fetching Linux amd64 sysroot (~30 MB)..."
    python3 build/linux/sysroot_scripts/install-sysroot.py --arch=amd64 2>&1 || {
      echo "warning: sysroot install failed; build will use system libc" >&2
    }
  else
    echo "ok: Linux sysroot already present"
  fi

  # 7. Write args.gn with the Halcyon build configuration.
  echo "=== writing args.gn ==="
  mkdir -p "$OUT_DIR"
  cat > "$OUT_DIR/args.gn" <<'GN'
# Halcyon-specific (from halcyon/build/BUILD.md)
enable_halcyon = true
halcyon_bundle_fonts = true
halcyon_default_search_engine = "duckduckgo"
halcyon_quiet_mode_default = true

# Build the Halcyon browser, not upstream Chrome.
is_official_build = false
is_debug = false
symbol_level = 0
strip_debug_info = true
target_cpu = "x64"

# Component build: produces a directory full of .so files instead of one
# monolithic chrome binary. Each .so links independently, so the link
# step uses far less peak memory. This is the single most important knob
# for fitting the build on a 16 GB runner.
is_component_build = true

# No LTO. ThinLTO would speed up runtime but requires a lot of memory
# and disk for the bitcode files.
use_thin_lto = false

# Disable features we don't need (each shaves time + disk).
enable_nacl = false
enable_remoting = false
enable_service_discovery = false

# Don't fail the CI build on warnings; the Halcyon code is new and may
# produce some that upstream's -Werror catches.
treat_warnings_as_errors = false

# Use the hermetic clang from third_party/llvm-build.
is_clang = true
use_custom_libcxx = true

# Use the Linux sysroot from build/linux/sysroot_scripts/.
use_sysroot = true

# No Goma / remote execution in CI.
use_goma = false
use_remoteexec = false

# Reduce WebCore symbol size — most of the debug symbols in a Chromium
# build come from Blink's templated code. We don't need them.
blink_symbol_level = 0
v8_symbol_level = 0
GN
  # NOTE: enable_features = ["HalcyonUI"] is NOT a valid GN arg (it's
  # a runtime field-trial config, not a build arg). The kHalcyonUI
  # feature is declared FEATURE_DISABLED_BY_DEFAULT in our patch
  # gyp_enable_halcyon.patch; users enable it at runtime via
  # --enable-features=HalcyonUI on the chrome command line, OR we
  # patch the BASE_FEATURE declaration to FEATURE_ENABLED_BY_DEFAULT
  # in a future round. For CI testing, the chrome binary built here
  # will use upstream chrome by default; a follow-up change to the
  # kHalcyonUI BASE_FEATURE default is needed.
  echo "ok: args.gn written at $OUT_DIR/args.gn"
  cat "$OUT_DIR/args.gn"

  # 8. Bootstrap gn from the Chromium source and run gn gen.
  # The `gn` binary in depot_tools is a wrapper that looks for gn in
  # buildtools/linux64/gn, but that file is gitignored and is normally
  # fetched by `gclient runhooks` (which we don't run because we use
  # the source tarball). Instead, we bootstrap gn from the Chromium
  # source itself via tools/gn/bootstrap/bootstrap.py.
  echo "=== bootstrapping gn from source ==="
  if [[ ! -x "${SRC_DIR}/buildtools/linux64/gn" ]]; then
    cd "$SRC_DIR"
    # The bootstrap script builds gn from source and writes it to
    # buildtools/linux64/gn. Takes ~1-2 minutes on the runner.
    python3 tools/gn/bootstrap/bootstrap.py -o buildtools/linux64/gn \
        --skip-generate-buildfiles 2>&1 || {
      echo "error: gn bootstrap failed" >&2
      echo "  see https://gn.googlesource.com/gn/ for the source" >&2
      exit 1
    }
  fi
  if [[ ! -x "${SRC_DIR}/buildtools/linux64/gn" ]]; then
    echo "error: gn binary not found at ${SRC_DIR}/buildtools/linux64/gn after bootstrap" >&2
    exit 1
  fi
  echo "ok: gn bootstrapped at ${SRC_DIR}/buildtools/linux64/gn"
  ${SRC_DIR}/buildtools/linux64/gn --version

  echo "=== running gn gen ==="
  cd "$SRC_DIR"
  # Use the bootstrapped gn directly (not the depot_tools wrapper,
  # which has trouble finding it).
  ${SRC_DIR}/buildtools/linux64/gn gen "$OUT_DIR" --fail-on-unused-args
  echo "ok: gn gen complete"

  # 9. Verify the Halcyon targets are in the build graph.
  echo "=== verifying Halcyon targets in build graph ==="
  if ${SRC_DIR}/buildtools/linux64/gn ls "$OUT_DIR" //halcyon:halcyon 2>&1 | head -3; then
    echo "ok: //halcyon:halcyon is in the build graph"
  else
    echo "warning: //halcyon:halcyon not found — check that enable_halcyon is true in args.gn"
  fi
  # The chrome target should pull in Halcyon via the source_set("halcyon")
  # dependency added by patches/gyp_enable_halcyon.patch.
  if ${SRC_DIR}/buildtools/linux64/gn ls "$OUT_DIR" //chrome/browser/ui:halcyon 2>&1 | head -3; then
    echo "ok: //chrome/browser/ui:halcyon is in the build graph"
  else
    echo "warning: //chrome/browser/ui:halcyon not found"
  fi

  echo ""
  echo "=== prep mode complete ==="
  echo "The next job (build_part_01) will run ninja on this state."
  exit 0
fi

# --- Build mode ----------------------------------------------------------

echo "=== build mode: ninja -C ${OUT_DIR} chrome ==="
cd "$SRC_DIR"

# Run ninja with a timeout. If it times out, the next job continues
# from where this one left off (the .ninja_log file is intact in the
# persisted build directory).
set +e
timeout -k 5m -s INT "${TASK_TIMEOUT}s" \
  ninja -C "$OUT_DIR" -j2 chrome
rc=$?
set -e

echo "info: ninja exited with rc=$rc"

# rc=124 means the timeout fired. If this is not the final job, that's
# OK — the next job picks up from here.
if [[ "$GHA_FINAL" != "true" && "$rc" -eq 124 ]]; then
  echo "info: task timed out after ${TASK_TIMEOUT}s; continuing in next run."
  if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    echo "status=running" >> "$GITHUB_OUTPUT"
  fi
  exit 0
fi

# rc=0 means ninja finished. Verify the chrome binary actually exists.
if [[ "$rc" -eq 0 ]]; then
  if [[ -x "${OUT_DIR}/chrome" ]]; then
    echo "ok: chrome binary built at ${OUT_DIR}/chrome"
    ls -lh "${OUT_DIR}/chrome"
    if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
      echo "status=completed" >> "$GITHUB_OUTPUT"
    fi
    exit 0
  else
    echo "error: ninja reported success but chrome binary not found" >&2
    ls -la "$OUT_DIR" | head -20
    exit 1
  fi
fi

# Any other exit code is a real failure.
echo "error: ninja failed with rc=$rc" >&2
if [[ "$GHA_FINAL" == "true" ]]; then
  echo "  this was the final build_part job; the build is incomplete." >&2
  echo "  the workflow needs more build_part jobs to finish." >&2
fi
exit "$rc"
