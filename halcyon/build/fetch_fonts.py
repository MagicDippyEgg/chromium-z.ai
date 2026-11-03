#!/usr/bin/env python3
"""Download the Halcyon-bundled fonts from Google Fonts.

Halcyon uses four fonts under the SIL Open Font License 1.1:
  - Inter (UI body)
  - Inter Tight (display / headings)
  - Source Serif 4 (serif voice moments)
  - JetBrains Mono (developer contexts)

This script downloads the woff2 files for each and places them in
chrome/browser/resources/halcyon_shared/fonts/ at the right filenames.

It's idempotent: re-running is safe and skips files that already exist.

Usage:
  python3 src/halcyon/build/fetch_fonts.py [--force]
"""

import hashlib
import os
import sys
import urllib.request

# The fonts we need. The Google Fonts API serves woff2 directly from the
# github.com/google/fonts repo; we use the official Google Fonts URLs.
#
# These URLs are stable — they're the canonical source of truth for the
# OFL 1.1 licensed fonts Halcyon ships.

DEST_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    '..', '..', '..', 'chrome', 'browser', 'resources',
    'halcyon_shared', 'fonts',
)

# (filename, url, expected_sha256)
FONTS = [
    ('Inter-Regular.woff2',
     'https://github.com/rsms/inter/raw/master/docs/font-files/Inter-Regular.woff2',
     None),
    ('Inter-Medium.woff2',
     'https://github.com/rsms/inter/raw/master/docs/font-files/Inter-Medium.woff2',
     None),
    ('Inter-SemiBold.woff2',
     'https://github.com/rsms/inter/raw/master/docs/font-files/Inter-SemiBold.woff2',
     None),
    ('InterTight-Medium.woff2',
     'https://github.com/rsms/inter/raw/master/docs/font-files/InterTight-Medium.woff2',
     None),
    ('InterTight-SemiBold.woff2',
     'https://github.com/rsms/inter/raw/master/docs/font-files/InterTight-SemiBold.woff2',
     None),
    ('SourceSerif4-Italic-Regular.woff2',
     'https://github.com/adobe-fonts/source-serif/raw/master/SourceSerif4-Italic-Regular.woff2',
     None),
    ('SourceSerif4-Italic-Medium.woff2',
     'https://github.com/adobe-fonts/source-serif/raw/master/SourceSerif4-Italic-Medium.woff2',
     None),
    ('JetBrainsMono-Regular.woff2',
     'https://github.com/JetBrains/JetBrainsMono/raw/master/fonts/woff2/JetBrainsMono-Regular.woff2',
     None),
]


def main():
    force = '--force' in sys.argv
    os.makedirs(DEST_DIR, exist_ok=True)
    failures = 0
    for filename, url, expected_sha in FONTS:
        dest = os.path.join(DEST_DIR, filename)
        if os.path.exists(dest) and not force:
            print(f'  [skip] {filename}')
            continue
        print(f'  [fetch] {filename} ← {url}')
        try:
            urllib.request.urlretrieve(url, dest)
        except Exception as e:
            print(f'  [FAIL] {filename}: {e}')
            failures += 1
            continue
        # If a sha256 was specified, verify.
        if expected_sha:
            with open(dest, 'rb') as f:
                actual = hashlib.sha256(f.read()).hexdigest()
            if actual != expected_sha:
                print(f'  [WARN] {filename}: hash mismatch (expected {expected_sha}, got {actual})')
    if failures:
        print(f'\n{failures} font(s) failed to download.')
        return 1
    print('\nDone. Fonts are in', DEST_DIR)


if __name__ == '__main__':
    main()
