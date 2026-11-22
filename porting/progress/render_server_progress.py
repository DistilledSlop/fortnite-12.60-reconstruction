#!/usr/bin/env python3
"""Generate the server treemap and evidence table from one acceptance ledger."""
import argparse
from collections import Counter
from html import escape
from pathlib import Path
import re

import progress

ROOT = Path(__file__).resolve().parents[2]
LEDGER = ROOT / "porting/progress/server_checks.txt"
BOARD = ROOT / "docs/progress/server/index.html"
OUTPUT = BOARD.with_name("server-implementation-treemap-v4.svg")
REPO_URL = "https://github.com/DistilledSlop/fortnite-12.60-reconstruction"
START = "<!-- checks:start -->"
END = "<!-- checks:end -->"
STATUSES = {
    "verified": "Verified",
    "implemented": "Implemented",
    "open": "Open",
    "client": "Client only",
}


def read_checks():
    areas = {}
    seen = set()
    area = None
    for line_number, line in enumerate(LEDGER.read_text().splitlines(), 1):
        line = line.strip()
        if line.startswith("## "):
            area = line.removeprefix("## ")
            if area in areas:
                raise ValueError(f"Duplicate area at line {line_number}")
            areas[area] = {}
        elif line and not line.startswith("#"):
            fields = [field.strip() for field in line.split("|")]
            if area is None or len(fields) not in (3, 4):
                raise ValueError(f"Malformed check at line {line_number}")
            name, status, check, *refs = fields
            if (not name or not check or status not in STATUSES or check in seen
                    or (status in ("verified", "implemented")) != bool(refs and refs[0])):
                raise ValueError(f"Invalid or duplicate check at line {line_number}")
            seen.add(check)
            areas[area].setdefault(name, []).append((status, check, refs[0] if refs else ""))
    return areas


def node(name, counts, **details):
    done = counts["verified"] + counts["implemented"]
    total = counts.total() - counts["client"]
    if total <= 0:
        raise ValueError(f"No server acceptance checks in {name}")
    return dict(id=name, name=name, weight=total, pct=100 * done / total,
                done=done, total=total, basis="equal-weight server implementation checks", **details)


def measure(areas):
    groups = []
    all_counts = Counter()
    for area, entries in areas.items():
        area_counts = Counter()
        children = []
        for name, checks in entries.items():
            counts = Counter(status for status, _, _ in checks)
            details = "\n".join(f"{STATUSES[status]}: {check}" for status, check, _ in checks)
            children.append(node(name, counts, evidence=details))
            area_counts.update(counts)
        groups.append(node(area, area_counts, children=children))
        all_counts.update(area_counts)
    return groups, all_counts


def render_svg(groups, counts):
    total = counts.total() - counts["client"]
    done = counts["verified"] + counts["implemented"]
    width, height = progress.STANDALONE_W, progress.STANDALONE_H
    body = progress.render_treemap(groups, width, height - 88, prefix="server")
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" role="img" aria-label="Dedicated-server implementation treemap" font-family="system-ui,-apple-system,Segoe UI,sans-serif">
<style>{progress.TREEMAP_CSS}</style>
<rect class="tm-bg" width="{width}" height="{height}"/>
<text x="12" y="28" font-size="20" font-weight="700" class="tm-head">Fortnite 12.60 server progress: {done / total:.1%} implemented</text>
<text x="12" y="48" font-size="12" class="tm-head">{done} of {total} server checks implemented | {counts["verified"]} verified | {counts["open"]} open</text>
<g transform="translate(0,60)">{body}</g>
<text x="12" y="{height - 10}" font-size="11" class="tm-head">Tile area is check count. Blue is implementation coverage. Client-only checks are listed on the board.</text>
</svg>
'''


def evidence_url(ref):
    if re.fullmatch(r"[0-9a-f]{7,40}", ref):
        return f"{REPO_URL}/commit/{ref}"
    if not (ROOT / ref).is_file():
        raise ValueError(f"Missing evidence file: {ref}")
    if ref.startswith("docs/"):
        return "../../" + ref.removeprefix("docs/")
    return f"{REPO_URL}/blob/main/{ref}"


def render_board(areas, groups, counts):
    total = counts.total() - counts["client"]
    done = counts["verified"] + counts["implemented"]
    parts = [
        START,
        f'<p><strong>{done}/{total} server checks implemented ({done / total:.1%}).</strong> '
        f'{counts["verified"]} are verified, {counts["implemented"]} await verification, {counts["open"]} remain open, '
        f'and {counts["client"]} client-only checks are tracked outside the server score.</p>',
        '<p>Coverage = (implemented checks + verified checks) ÷ all server checks. Area and capability scores use the same formula. '
        'Every check has equal weight. Verified means the stated behavior has a recorded headless test, '
        'service test, or original-client observation. Implementation-only checks have a source reference. '
        'Each check contributes at most one point.</p>',
    ]
    for (area, entries), group in zip(areas.items(), groups, strict=True):
        parts.extend([
            f'<h2>{escape(area)} <small>{group["done"]}/{group["total"]} implemented ({group["pct"]:.1f}%)</small></h2>',
            f'<div class="table"><table aria-label="{escape(area)}">',
            '<thead><tr><th scope="col">Capability</th><th scope="col">Status</th>'
            '<th scope="col">Acceptance check</th><th scope="col">Evidence</th></tr></thead><tbody>',
        ])
        for name, checks in entries.items():
            for status, check, ref in checks:
                link = f'<a href="{escape(evidence_url(ref))}">Source</a>' if ref else "—"
                parts.append(f'<tr><th scope="row">{escape(name)}</th><td>{STATUSES[status]}</td>'
                             f'<td>{escape(check)}</td><td>{link}</td></tr>')
        parts.append("</tbody></table></div>")
    parts.append(END)
    original = BOARD.read_text()
    start = original.index(START)
    end = original.index(END, start) + len(END)
    return original[:start] + "\n".join(parts) + original[end:]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if either generated view is stale")
    args = parser.parse_args()
    areas = read_checks()
    groups, counts = measure(areas)
    outputs = {OUTPUT: render_svg(groups, counts), BOARD: render_board(areas, groups, counts)}
    for path, content in outputs.items():
        if args.check:
            if not path.is_file() or path.read_text() != content:
                parser.exit(1, f"Stale server progress view: {path.relative_to(ROOT)}\n")
        else:
            path.write_text(content)


if __name__ == "__main__":
    main()
