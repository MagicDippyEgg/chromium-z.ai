#!/usr/bin/env bash
# export_cache.sh — compress the build/ directory into a tar.zst archive
# suitable for upload as a GitHub Actions artifact.
#
# The build/ directory contains:
#   - The full Chromium source tree (~25 GB extracted)
#   - The GN build directory + ninja output (~5-15 GB for a component build)
#   - The .ninja_log file (essential for incremental builds)
#   - Any downloaded toolchain bits
#
# We compress with zstd at level 3 (multi-threaded) for a good balance
# of speed and size. The resulting archive is typically 3-8 GB, well
# within the GitHub Actions artifact size limits.
#
# We EXCLUDE:
#   - build/download_cache/  — re-downloaded by fetch_source.sh if needed
#   - build/src/out/Halcyon/obj/  — intermediate object files; ninja
#                                 regenerates these from the .ninja_log
#                                 state, and they're huge (10+ GB).
#                                 Including them would blow past the
#                                 10 GB per-artifact limit.
#   - build/src/third_party/llvm-build/Release+Asserts/lib/clang/*/Modules/
#                              — clang module cache; regenerated on build.
#
# We INCLUDE:
#   - build/src/             — the patched source tree (so the next job
#                             doesn't need to re-apply patches)
#   - build/src/out/Halcyon/*.ninja* — ninja build files + log (essential)
#   - build/src/out/Halcyon/gen/    — generated source files (regenerating
#                                     these takes 10+ minutes; better to
#                                     keep them)
#   - build/src/out/Halcyon/args.gn  — the build configuration
#   - The .gclient + .downloaded.stamp files
#
# Usage:
#   export_cache.sh <archive_path>
#
# Environment:
#   ARCH    — the architecture (used to name the archive)
#   ZSTD_LEVEL — zstd compression level (default: 3)

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <archive_path>" >&2
  exit 2
fi

ARCHIVE_PATH="$1"
ZSTD_LEVEL="${ZSTD_LEVEL:-3}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"

if [[ ! -d "$BUILD_DIR" ]]; then
  echo "error: build directory not found at $BUILD_DIR" >&2
  exit 1
fi

if [[ ! -d "$BUILD_DIR/src" ]]; then
  echo "error: $BUILD_DIR/src not found — has fetch_source.sh been run?" >&2
  exit 1
fi

echo "=== exporting build cache to $ARCHIVE_PATH ==="
echo "  source:    $BUILD_DIR"
echo "  zstd level: $ZSTD_LEVEL"
echo "  free disk: $(df -BG --output=avail "$(dirname "$ARCHIVE_PATH")" | tail -1)"

# Create the parent directory of the archive if needed.
mkdir -p "$(dirname "$ARCHIVE_PATH")"

# Remove any previous archive at this path.
rm -f "$ARCHIVE_PATH"

# Clean up the build/download_cache/ (we re-download if needed).
rm -rf "${BUILD_DIR}/download_cache"

# Remove the chrome binary itself — it's huge (1+ GB for static, ~50 MB
# for component but linked .so files are 10+ GB) and the package step
# will re-link it from the .ninja_log state. Actually, for a component
# build we WANT to keep the .so files because re-linking them takes time.
# Compromise: keep the .so files, remove the chrome binary itself.
rm -f "${BUILD_DIR}/src/out/Halcyon/chrome"

# NOTE: We do NOT remove obj/ even if disk is tight, because obj/
# contains the per-target .ninja files that ninja needs to resume
# the build. The tar excludes already filter out .o/.obj/.a files.

# Build the exclude list. The pattern is relative to BUILD_DIR.
# We EXCLUDE the obj/ directory's actual object files (.o, .obj) but
# KEEP the .ninja files that GN generates in obj/ (ninja needs these
# to know how to build each target). We also exclude other large
# intermediate files.
EXCLUDES=(
  "--exclude=./src/out/Halcyon/obj/*.o"
  "--exclude=./src/out/Halcyon/obj/*.obj"
  "--exclude=./src/out/Halcyon/obj/*.a"
  "--exclude=./src/out/Halcyon/third_party/llvm-build/Release+Asserts/lib/clang/*/Modules"
  "--exclude=./src/out/Halcyon/*.ninja_lock"
)

# Tar + zstd. -T0 uses all available cores for compression.
cd "$BUILD_DIR"
echo "info: tarring + zstd-compressing $BUILD_DIR..."
tar --use-compress-program="zstd -T0 -${ZSTD_LEVEL} -f" \
    --create \
    "${EXCLUDES[@]}" \
    --file "$ARCHIVE_PATH" \
    ./

echo "ok: archive created"
ls -lh "$ARCHIVE_PATH"
echo "  free disk now: $(df -BG --output=avail / | tail -1)"
