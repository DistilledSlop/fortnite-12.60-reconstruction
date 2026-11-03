#!/usr/bin/env python3
"""Publish the rendered progress dashboard for GitHub.

Copies the generated pages from porting/progress/out/ into docs/ (served by
GitHub Pages) and rewrites the marked progress block in the root README.md
from out/progress.json. Stdlib only, offline, deterministic for a given out/.

    python3 porting/progress/progress.py --no-record   # render HEAD first
    python3 porting/progress/publish.py                # then publish
    python3 porting/progress/publish.py --check        # exit 1 if docs/README are stale
"""
import json
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "porting" / "progress" / "out"
DOCS = ROOT / "docs"
README = ROOT / "README.md"
START, END = "<!-- progress:start -->", "<!-- progress:end -->"
BSTART, BEND = "<!-- badges:start -->", "<!-- badges:end -->"
PAGES = ["index.html", "history.html", "progress.svg", "progress.json"]


def bar(pct, width=20):
    filled = round(width * pct / 100)
    return "█" * filled + "░" * (width - filled)


def badges(d):
    pct = f"{d['overall_pct']:.1f}%25"
    return "\n".join([
        BSTART,
        f"![overall](https://img.shields.io/badge/overall-{pct}-2a78d6) "
        f"![milestones](https://img.shields.io/badge/milestones-{d['milestones_done']}%2F{d['milestones_total']}-d6a02a) "
        f"![snapshot](https://img.shields.io/badge/snapshot-{d['head'][:8]}-52514e)",
        BEND,
    ])


def block(d):
    short = d["head"][:8]
    date = d["date"][:10]
    lines = [START, ""]
    lines.append(
        f"**Overall: {d['overall_pct']:.1f}%** · evidence-based {d['measured_pct']:.1f}% · "
        f"objective {d['objective_pct']:.1f}% · milestones {d['milestones_done']}/{d['milestones_total']} · "
        f"snapshot `{short}` ({date})"
    )
    lines += ["", "| Milestone | Status |", "|---|---|"]
    for m in d["milestones"]:
        lines.append(f"| {m['text']} | {'✅ done' if m['done'] else '⬜ open'} |")
    lines += ["", "| Area | Progress | % | Effort weight |", "|---|---|--:|--:|"]
    total = sum(g["weight"] for g in d["groups"]) or 1
    for g in d["groups"]:
        lines.append(
            f"| {g['name']} | `{bar(g['pct'])}` | {g['pct']:.1f}% | {100 * g['weight'] / total:.0f}% |"
        )
    lines += [
        "",
        "<sub>Percentages are effort-weighted estimates of reconstruction work, not a measure "
        "of the playable-game definition, which is far from met. "
        "`objective` counts only computed evidence; `evidence-based` adds documented counts; "
        "the rest is manual estimate. Method: "
        "[porting/progress/README.md](porting/progress/README.md).</sub>",
        "",
        END,
    ]
    return "\n".join(lines)


def render_readme(d):
    text = README.read_text()
    for (a, b), make in (((BSTART, BEND), badges), ((START, END), block)):
        s, e = text.index(a), text.index(b) + len(b)
        text = text[:s] + make(d) + text[e:]
    return text


def main():
    check = "--check" in sys.argv
    d = json.loads((OUT / "progress.json").read_text())
    files = {n: (OUT / n).read_bytes() for n in PAGES}
    for p in sorted(OUT.glob("*.txt")):
        files[p.name] = p.read_bytes()
    stale = []
    readme = render_readme(d)
    if README.read_text() != readme:
        stale.append("README.md")
    for n, data in files.items():
        f = DOCS / n
        if not f.exists() or f.read_bytes() != data:
            stale.append(f"docs/{n}")
    if check:
        print("stale: " + ", ".join(stale) if stale else "up to date")
        sys.exit(1 if stale else 0)
    DOCS.mkdir(exist_ok=True)
    for n, data in files.items():
        (DOCS / n).write_bytes(data)
    (DOCS / ".nojekyll").write_text("")
    README.write_text(readme)
    print(f"published {len(files)} files to docs/ and refreshed README.md ({d['overall_pct']:.1f}% at {d['head'][:8]})")


if __name__ == "__main__":
    main()
