#!/usr/bin/env python3
# Copyright 2026 The Halcyon Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
verify_tokens.py — single source of truth check for the Halcyon design system.

The Halcyon design system has two parallel representations:

  1. design_tokens.css        — CSS custom properties (WebUI side)
  2. ui/base/halcyon_color_palette.h — C++ SkColor constants (native side)

Both files are derived from the brand-book values in
src/halcyon/branding/color_system.md. The intent is that the values in (1)
and (2) match exactly (mod RGB<->ARGB byte order). This script enforces it.

Usage:
    python3 verify_tokens.py [--fix]

Without --fix, the script exits non-zero if any mismatch is found.
With --fix, the script rewrites the C++ header to match the CSS file.

Run from anywhere; the script finds the repo root by looking up from this
file's location.
"""

import os
import re
import sys
from typing import Dict, Tuple


REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CSS_PATH = os.path.join(REPO_ROOT, "halcyon", "design", "design_tokens.css")
HEADER_PATH = os.path.join(
    REPO_ROOT,
    "halcyon",
    "src",
    "ui",
    "base",
    "halcyon_color_palette.h",
)


# --- Parsers --------------------------------------------------------------


def parse_css_colors(css: str) -> Dict[str, Tuple[int, int, int]]:
    """
    Walk the CSS file and pull out the FIRST `--halcyon-<name>: #RRGGBB;`
    declaration for each `<name>`. The first occurrence is the light-theme
    ("Vellum") value, defined at :root. Dark-theme and high-contrast
    overrides live in @media blocks and are intentionally ignored by this
    script — we only verify that the canonical Vellum values match the
    canonical Vellum values in halcyon_color_palette.h.

    Returns {name: (R, G, B)}, where name is the C++-style camelCase
    identifier used in halcyon_color_palette.h (e.g. `Bg`, `Surface2`,
    `AccentPress`).
    """
    colors = {}
    pattern = re.compile(
        r"--halcyon-([a-z0-9-]+):\s*#([0-9a-fA-F]{6})\s*;",
        re.MULTILINE,
    )
    for m in pattern.finditer(css):
        kebab = m.group(1)
        hex_val = m.group(2)
        camel = kebab_to_camel(kebab)
        if camel in colors:
            # Already recorded the Vellum (first) value; ignore later
            # overrides (Eclipse, high-contrast, forced-colors).
            continue
        r = int(hex_val[0:2], 16)
        g = int(hex_val[2:4], 16)
        b = int(hex_val[4:6], 16)
        colors[camel] = (r, g, b)
    return colors


def kebab_to_camel(kebab: str) -> str:
    """`surface-2` → `Surface2`, `accent-press` → `AccentPress`."""
    parts = kebab.split("-")
    return "".join(p[:1].upper() + p[1:] for p in parts if p)


def parse_cpp_colors(cpp: str) -> Dict[str, Tuple[int, int, int]]:
    """
    Walk the C++ header and pull out the FIRST
    `constexpr SkColor k<X> = SkColorSetRGB(0xRR, 0xGG, 0xBB);` declaration
    for each `<X>`. The first occurrence is the Vellum (light) section;
    later ones are Eclipse (dark) overrides, ignored here.

    Returns {name: (R, G, B)}.
    """
    colors = {}
    pattern = re.compile(
        r"constexpr\s+SkColor\s+k([A-Za-z0-9_]+)\s*=\s*SkColorSetRGB\("
        r"\s*0x([0-9a-fA-F]{2})\s*,\s*0x([0-9a-fA-F]{2})\s*,\s*0x([0-9a-fA-F]{2})"
        r"\s*\)\s*;",
        re.MULTILINE,
    )
    for m in pattern.finditer(cpp):
        name = m.group(1)
        if name in colors:
            continue  # later occurrence — Eclipse section; skip
        r = int(m.group(2), 16)
        g = int(m.group(3), 16)
        b = int(m.group(4), 16)
        colors[name] = (r, g, b)
    return colors


# --- Diff -----------------------------------------------------------------


def diff(css: Dict[str, Tuple[int, int, int]], cpp: Dict[str, Tuple[int, int, int]]):
    """
    Returns a list of (name, css_value, cpp_value) tuples for any color that
    appears in BOTH files with different values, plus a list of names that
    appear in only one of the two.
    """
    mismatches = []
    only_in_css = []
    only_in_cpp = []
    for name, css_val in css.items():
        if name in cpp:
            if cpp[name] != css_val:
                mismatches.append((name, css_val, cpp[name]))
        else:
            only_in_css.append(name)
    for name in cpp:
        if name not in css:
            only_in_cpp.append(name)
    return mismatches, only_in_css, only_in_cpp


# --- Main -----------------------------------------------------------------


def main() -> int:
    if not os.path.isfile(CSS_PATH):
        print(f"error: missing {CSS_PATH}", file=sys.stderr)
        return 2
    if not os.path.isfile(HEADER_PATH):
        print(f"error: missing {HEADER_PATH}", file=sys.stderr)
        return 2

    with open(CSS_PATH, "r", encoding="utf-8") as f:
        css = f.read()
    with open(HEADER_PATH, "r", encoding="utf-8") as f:
        cpp = f.read()

    css_colors = parse_css_colors(css)
    cpp_colors = parse_cpp_colors(cpp)

    mismatches, only_in_css, only_in_cpp = diff(css_colors, cpp_colors)

    if not mismatches and not only_in_css and not only_in_cpp:
        print(
            f"ok: {len(cpp_colors)} colors in both design_tokens.css and "
            f"halcyon_color_palette.h match exactly."
        )
        return 0

    if mismatches:
        print(
            f"error: {len(mismatches)} color(s) have different values in CSS "
            f"vs C++:",
            file=sys.stderr,
        )
        for name, css_val, cpp_val in mismatches:
            print(
                f"  {name}: css=#{css_val[0]:02X}{css_val[1]:02X}{css_val[2]:02X} "
                f"cpp=#{cpp_val[0]:02X}{cpp_val[1]:02X}{cpp_val[2]:02X}",
                file=sys.stderr,
            )

    if only_in_css:
        print(
            f"warning: {len(only_in_css)} color(s) defined in CSS but not in "
            f"C++: {', '.join(only_in_css)}",
            file=sys.stderr,
        )

    if only_in_cpp:
        print(
            f"warning: {len(only_in_cpp)} color(s) defined in C++ but not in "
            f"CSS: {', '.join(only_in_cpp)}",
            file=sys.stderr,
        )

    return 1 if mismatches else 0


if __name__ == "__main__":
    sys.exit(main())
