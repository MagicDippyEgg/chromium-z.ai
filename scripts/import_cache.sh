#!/usr/bin/env bash
# import_cache.sh — extract a previously-exported build cache archive
# back into the build/ directory.
#
# Used at the start of each build_part_NN job to restore the state from
# the previous job. Combined with export_cache.sh, this implements the
# chunked-build pattern: each job downloads the previous job's archive,
# extracts it, runs ninja for up to 5 hours, then re-archives and uploads
# the new state.
#
# Also frees up ~10 GB of preinstalled runner tooling before extracting,
# to make room for the ~8 GB extracted archive + the ninja output.
#
# Usage:
#   import_cache.sh <archive_path>
#
# Where <archive_path> is the path to the build-cache-*.tar.zst file
# downloaded by the workflow's download-artifact step.

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <archive_path>" >&2
  exit 2
fi

ARCHIVE_PATH="$1"

if [[ ! -f "$ARCHIVE_PATH" ]]; then
  echo "error: archive not found at $ARCHIVE_PATH" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"

echo "=== importing build cache from $ARCHIVE_PATH ==="
echo "  target: $BUILD_DIR"
echo "  archive size: $(du -h "$ARCHIVE_PATH" | cut -f1)"
echo "  free disk before cleanup: $(df -BG --output=avail / | tail -1)"

# Free up ~10 GB of preinstalled runner tooling before extracting the
# archive. This is the same cleanup the ungoogled-chromium-portablelinux
# project uses.
if [[ -d /usr/local/lib/android ]]; then
  sudo rm -rf /usr/local/lib/android
fi
if [[ -d /usr/local/.ghcup ]]; then
  sudo rm -rf /usr/local/.ghcup
fi
if [[ -d /usr/lib/jvm ]]; then
  sudo rm -rf /usr/lib/jvm
fi
if [[ -d /usr/lib/google-cloud-sdk ]]; then
  sudo rm -rf /usr/lib/google-cloud-sdk
fi
if [[ -d /usr/lib/dotnet ]]; then
  sudo rm -rf /usr/lib/dotnet
fi
if [[ -d /usr/share/swift ]]; then
  sudo rm -rf /usr/share/swift
fi

echo "  free disk after cleanup: $(df -BG --output=avail / | tail -1)"

# Create the target directory.
mkdir -p "$BUILD_DIR"

# Extract the archive. The archive was created with `tar -C build_dir
# --file archive`, so we extract into the build/ directory.
echo "info: extracting archive (this takes ~5 minutes)..."
cd "$BUILD_DIR"
zstd -d -c "$ARCHIVE_PATH" | tar -xf -

# We don't need the archive once it's extracted — delete it to free disk.
rm -f "$ARCHIVE_PATH"

echo "ok: cache imported"
echo "  free disk now: $(df -BG --output=avail / | tail -1)"
echo "  contents of $BUILD_DIR:"
ls -la "$BUILD_DIR" | head -10
if [[ -d "$BUILD_DIR/src" ]]; then
  echo "  contents of $BUILD_DIR/src/out/Halcyon (if present):"
  ls -la "$BUILD_DIR/src/out/Halcyon" 2>/dev/null | head -10 || true
fi
