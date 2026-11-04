#!/usr/bin/env bash
# fetch_source.sh — download and extract the pinned Chromium source tarball.
#
# Why a tarball instead of `gclient sync`:
#   - The tarball is ~5 GB compressed vs ~30 GB for a `gclient sync
#     --no-history --shallow` checkout. This is the difference between
#     fitting and not fitting on a GitHub Actions free runner.
#   - The tarball contains all third-party deps at the right versions,
#     pinned to the release tag.
#   - Download is ~5 minutes vs ~30 minutes for gclient sync.
#
# Usage:
#   fetch_source.sh <dest_dir>
#
# Where <dest_dir> is the directory where chromium-159.0.8141.0/ will
# be extracted. The script creates a `.downloaded.stamp` file inside
# <dest_dir> so subsequent runs skip the download (idempotent).
#
# Environment variables:
#   HALCYON_CHROMIUM_TARBALL_URL — the tarball URL (defaults to the
#                                  pinned URL from CHROMIUM_PIN.txt)
#   HALCYON_CHROMIUM_VERSION      — the version string (e.g. "159.0.8141.0")
#
# Exit codes:
#   0 — sources are present (either freshly downloaded or already there)
#   1 — download or extraction failed
#   2 — invalid arguments

set -euo pipefail

# --- Inputs --------------------------------------------------------------

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <dest_dir>" >&2
  exit 2
fi

DEST_DIR="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Read the pinned tarball URL + version from CHROMIUM_PIN.txt.
PIN_FILE="${REPO_ROOT}/halcyon/build/CHROMIUM_PIN.txt"
if [[ ! -f "$PIN_FILE" ]]; then
  echo "error: pin file not found: $PIN_FILE" >&2
  exit 1
fi
# shellcheck disable=SC1090
source "$PIN_FILE"

TARBALL_URL="${HALCYON_CHROMIUM_TARBALL_URL:?}"
TARBALL_VERSION="${HALCYON_CHROMIUM_VERSION:?}"
TARBALL_NAME="chromium-${TARBALL_VERSION}.tar.xz"
TARBALL_PATH="/tmp/${TARBALL_NAME}"
EXTRACTED_DIR_NAME="chromium-${TARBALL_VERSION}"
STAMP_FILE="${DEST_DIR}/.downloaded.stamp"

# --- Idempotent skip -----------------------------------------------------

if [[ -f "$STAMP_FILE" ]]; then
  echo "ok: sources already present at ${DEST_DIR} (stamp file present)"
  exit 0
fi

# --- Disk check ----------------------------------------------------------

mkdir -p "$DEST_DIR"
DEST_FREE_GB=$(df -BG --output=avail "$DEST_DIR" | tail -1 | tr -dc '0-9')
echo "info: free disk at $DEST_DIR: ${DEST_FREE_GB} GB"
if [[ "$DEST_FREE_GB" -lt 25 ]]; then
  echo "error: need at least 25 GB free at $DEST_DIR for the extracted source + build output" >&2
  echo "  free up disk space and re-run." >&2
  exit 1
fi

# --- Download ------------------------------------------------------------

if [[ ! -f "$TARBALL_PATH" ]]; then
  echo "info: downloading $TARBALL_NAME from $TARBALL_URL..."
  echo "  size: ~5 GB; this takes 1-5 minutes depending on bandwidth."
  curl --fail --retry 3 --retry-delay 5 \
       --connect-timeout 30 --max-time 1800 \
       -o "$TARBALL_PATH.tmp" \
       "$TARBALL_URL"
  mv "$TARBALL_PATH.tmp" "$TARBALL_PATH"
  echo "ok: downloaded $(du -h "$TARBALL_PATH" | cut -f1)"
else
  echo "info: tarball already present at $TARBALL_PATH ($(du -h "$TARBALL_PATH" | cut -f1))"
fi

# --- Verify integrity ----------------------------------------------------

# We don't have a published sha256 for the tarball, but we can verify the
# tarball isn't corrupt by listing it.
echo "info: verifying tarball integrity..."
if ! tar -tJf "$TARBALL_PATH" "${EXTRACTED_DIR_NAME}/chrome/VERSION" >/dev/null 2>&1; then
  echo "error: tarball is corrupt or doesn't contain expected layout" >&2
  echo "  expected top-level dir: ${EXTRACTED_DIR_NAME}/" >&2
  rm -f "$TARBALL_PATH"
  exit 1
fi
echo "ok: tarball contains ${EXTRACTED_DIR_NAME}/chrome/VERSION"

# --- Extract -------------------------------------------------------------

echo "info: extracting ${TARBALL_NAME} to ${DEST_DIR}..."
echo "  this takes ~5 minutes for ~25 GB of extracted source."
tar -xJf "$TARBALL_PATH" -C "$DEST_DIR"
echo "ok: extracted to ${DEST_DIR}/${EXTRACTED_DIR_NAME}"

# --- Move into place -----------------------------------------------------

# The build scripts expect the source tree at $DEST_DIR/src/. The
# tarball extracts to $DEST_DIR/chromium-<version>/. Move it.
if [[ -d "$DEST_DIR/src" ]]; then
  echo "info: $DEST_DIR/src already exists; leaving extracted tarball at $DEST_DIR/$EXTRACTED_DIR_NAME"
else
  mv "$DEST_DIR/$EXTRACTED_DIR_NAME" "$DEST_DIR/src"
  echo "ok: source tree at $DEST_DIR/src"
fi

# Remove the tarball to free ~5 GB of disk for the build.
rm -f "$TARBALL_PATH"
echo "ok: removed tarball; freed ~5 GB"

# --- Stamp ---------------------------------------------------------------

echo "$(date -Iseconds)" > "$STAMP_FILE"
echo "ok: source fetch complete. stamp: $STAMP_FILE"
