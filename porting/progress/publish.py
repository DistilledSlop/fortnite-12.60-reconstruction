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
import re
import shutil
import subprocess
import sys
import urllib.parse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "porting" / "progress" / "out"
DOCS = ROOT / "docs"
README = ROOT / "README.md"
START, END = "<!-- progress:start -->", "<!-- progress:end -->"
BSTART, BEND = "<!-- badges:start -->", "<!-- badges:end -->"
CODE_EXTS = (".h", ".cpp", ".inc", ".py", ".cs", ".sh")
PAGES = ["index.html", "history.html", "progress.svg", "progress.json"]


def bar(pct, width=20):
    filled = round(width * pct / 100)
    return "█" * filled + "░" * (width - filled)


def git(*args):
    return subprocess.run(["git", "-C", str(ROOT), *args], capture_output=True, text=True, check=True).stdout


def repo_stats():
    files = git("ls-files", "-z").split("\0")
    files = [f for f in files if f]
    lines = 0
    for f in files:
        if f.endswith(CODE_EXTS):
            try:
                lines += (ROOT / f).read_bytes().count(b"\n")
            except OSError:
                pass
    by_ext = lambda *e: sum(1 for f in files if f.endswith(e))
    return {
        "commits": int(git("rev-list", "--count", "HEAD")),
        "files": len(files),
        "loc": lines,
        "cpp": by_ext(".cpp", ".h", ".inc"),
        "py": by_ext(".py"),
        "cs": by_ext(".cs"),
        "sh": by_ext(".sh"),
        "first": git("log", "--reverse", "--format=%ad", "--date=short").split("\n")[0],
    }


def human(n):
    if n >= 1_000_000:
        return f"{n / 1_000_000:.2f}M"
    return f"{n / 1000:.1f}k" if n >= 10000 else f"{n:,}"


def badge(label, message, color, logo=None):
    """Default shields.io flat badge as an <img>, so it can sit in an HTML table cell."""
    q = {"label": label, "message": str(message), "color": color}
    if logo:
        q["logo"] = logo
    url = "https://img.shields.io/static/v1?" + urllib.parse.urlencode(q, quote_via=urllib.parse.quote)
    return f'<img alt="{label}: {message}" src="{url.replace("&", "&amp;")}">'


def badges(d):
    st = repo_stats()
    leaves = d["leaves"]
    checks = sum(l.get("total") or 0 for l in leaves)
    passed = sum(l.get("done") or 0 for l in leaves)
    parity = re.search(r"([\d,]+) of ([\d,]+) generated classes", " ".join(m.get("evidence", "") for m in d["milestones"]))
    cols = [
        ("Progress", [
            badge("overall", f"{d['overall_pct']:.1f}%", "2a78d6"),
            badge("evidence-based", f"{d['measured_pct']:.1f}%", "3987e5"),
            badge("objective", f"{d['objective_pct']:.1f}%", "5a9cec"),
            badge("milestones", f"{d['milestones_done']}/{d['milestones_total']}", "d6a02a"),
            badge("snapshot", d["head"][:8], "9f9f9f", "git"),
        ]),
        ("Target", [
            badge("Fortnite", "12.60", "7d4cdb", "epicgames"),
            badge("build", "CL 13477524", "6a3fc4"),
            badge("Unreal Engine", "4.25.0", "0e1128", "unrealengine"),
            badge("platform", "Linux x64", "e8a317", "linux"),
            badge("renderer", "Vulkan", "a41e22", "vulkan"),
            badge("paks", "31 · v10", "9f9f9f"),
            badge("oracle", "Wine", "722f37", "wine"),
        ]),
        ("Stack", [
            badge("C++", f"{human(st['cpp'])} files", "00599c", "cplusplus"),
            badge("Python", f"{st['py']} scripts", "3776ab", "python"),
            badge("C#", f"{st['cs']} files", "512bd4"),
            badge("shell", f"{st['sh']} scripts", "4eaa25", "gnubash"),
            badge("code", f"{human(st['loc'])} lines", "2f6f4f"),
        ]),
        ("Evidence", [
            badge("areas", len(d["groups"]), "0e7490"),
            badge("components", len(leaves), "0e7490"),
            badge("units done", f"{human(passed)}/{human(checks)}", "0e7490"),
            *([badge("class parity", f"{parity.group(1)}/{parity.group(2)}", "2e8540")] if parity else []),
        ]),
        ("History", [
            badge("commits", human(st["commits"]), "1f6feb", "git"),
            badge("tracked files", human(st["files"]), "1f6feb"),
            badge("since", st["first"], "9f9f9f"),
            badge("status", "work in progress", "d6a02a"),
            badge("authored by", "Codex", "10a37f"),
            badge("co-authored by", "Claude", "d97757", "anthropic"),
        ]),
    ]
    depth = max(len(c) for _, c in cols)
    out = [BSTART, '<table align="center">', "<thead><tr>"]
    out += [f'<th align="center">{title}</th>' for title, _ in cols]
    out += ["</tr></thead>", "<tbody>"]
    for i in range(depth):
        cells = "".join(f'<td align="center" valign="middle">{c[i] if i < len(c) else ""}</td>' for _, c in cols)
        out.append(f"<tr>{cells}</tr>")
    out += ["</tbody>", "</table>", BEND]
    return "\n".join(out)


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
