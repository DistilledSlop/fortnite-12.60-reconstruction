#!/usr/bin/env python3
"""Generate a shields-style SVG badge for gallons of water used.

Usage:
  water_badge.py --from-codex [-o docs/water_badge.svg]   # estimate from token usage
  water_badge.py GALLONS [-o water_badge.svg]             # explicit value

Estimate: ~0.000085 gal (0.32 mL) per query, a published vendor figure, spread
over an assumed ~400 tokens per query = 2.1e-7 gal/token. It is applied to
uncached input + output tokens summed from ~/.codex/sessions; cached input
reads are excluded. This is a rough order-of-magnitude figure, not a measurement.
"""
import argparse
import glob
import json
import os

GAL_PER_TOKEN = 0.000085 / 400


def codex_tokens():
    """Sum final per-session token totals from Codex session logs."""
    billed = 0
    for f in glob.glob(os.path.expanduser("~/.codex/sessions/**/*.jsonl"), recursive=True):
        tot = None
        with open(f, errors="ignore") as fp:
            for line in fp:
                if '"token_count"' not in line:
                    continue
                try:
                    tu = ((json.loads(line).get("payload") or {}).get("info") or {}).get("total_token_usage")
                except Exception:
                    continue
                if tu:
                    tot = tu
        if tot:
            billed += max(0, tot.get("input_tokens", 0) - tot.get("cached_input_tokens", 0))
            billed += tot.get("output_tokens", 0)
    return billed
from xml.sax.saxutils import escape


def text_width(s):
    # Rough Verdana 11px estimate; good enough for a badge.
    return int(len(s) * 6.6) + 10


def color_for(gallons):
    if gallons < 10:
        return "#4c1"      # green
    if gallons < 100:
        return "#dfb317"   # yellow
    if gallons < 1000:
        return "#fe7d37"   # orange
    return "#e05d44"       # red


def make_badge(gallons, label="water used"):
    value = f"{gallons:,.1f}".rstrip("0").rstrip(".") + " gal"
    drop = 18  # space for the water-drop icon
    lw = text_width(label) + drop
    vw = text_width(value)
    w = lw + vw
    color = color_for(gallons)
    label, value = escape(label), escape(value)
    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="20" role="img" aria-label="{label}: {value}">
  <title>{label}: {value}</title>
  <linearGradient id="s" x2="0" y2="100%"><stop offset="0" stop-color="#bbb" stop-opacity=".1"/><stop offset="1" stop-opacity=".1"/></linearGradient>
  <clipPath id="r"><rect width="{w}" height="20" rx="3" fill="#fff"/></clipPath>
  <g clip-path="url(#r)">
    <rect width="{lw}" height="20" fill="#555"/>
    <rect x="{lw}" width="{vw}" height="20" fill="{color}"/>
    <rect width="{w}" height="20" fill="url(#s)"/>
  </g>
  <path d="M10 3.5 C10 3.5 5.5 9 5.5 12 a4.5 4.5 0 0 0 9 0 C14.5 9 10 3.5 10 3.5 Z" fill="#4fc3f7"/>
  <g fill="#fff" text-anchor="middle" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="11">
    <text x="{drop + (lw - drop) / 2}" y="15" fill="#010101" fill-opacity=".3">{label}</text>
    <text x="{drop + (lw - drop) / 2}" y="14">{label}</text>
    <text x="{lw + vw / 2}" y="15" fill="#010101" fill-opacity=".3">{value}</text>
    <text x="{lw + vw / 2}" y="14">{value}</text>
  </g>
</svg>
"""


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("gallons", type=float, nargs="?")
    ap.add_argument("--from-codex", action="store_true")
    ap.add_argument("-o", "--output", default="water_badge.svg")
    ap.add_argument("--label", default="water used")
    a = ap.parse_args()
    if a.from_codex:
        a.gallons = codex_tokens() * GAL_PER_TOKEN
    elif a.gallons is None:
        ap.error("give GALLONS or --from-codex")
    with open(a.output, "w") as f:
        f.write(make_badge(a.gallons, a.label))
    print(f"wrote {a.output}")
