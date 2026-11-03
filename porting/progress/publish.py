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
PAGES = ["index.html", "history.html", "progress.svg", "progress.json"]


def bar(pct, width=20):
    filled = round(width * pct / 100)
    return "█" * filled + "░" * (width - filled)


def git(*args):
    return subprocess.run(["git", "-C", str(ROOT), *args], capture_output=True, text=True, check=True).stdout


def repo_stats():
    files = [f for f in git("ls-files", "-z").split("\0") if f]
    groups = {"cpp": (".cpp", ".h", ".inc"), "py": (".py",), "cs": (".cs",), "sh": (".sh",), "md": (".md",)}
    lines = dict.fromkeys(groups, 0)
    count = {k: sum(1 for f in files if f.endswith(e)) for k, e in groups.items()}
    for f in files:
        for k, e in groups.items():
            if f.endswith(e):
                try:
                    lines[k] += (ROOT / f).read_bytes().count(b"\n")
                except OSError:
                    pass
    under = lambda pre: sum(1 for f in files if f.startswith(pre))
    return {
        "commits": int(git("rev-list", "--count", "HEAD")),
        "files": len(files),
        "lines": lines,
        "count": count,
        "cpp_h": sum(1 for f in files if f.endswith(".h")),
        "cpp_c": sum(1 for f in files if f.endswith(".cpp")),
        "cpp_i": sum(1 for f in files if f.endswith(".inc")),
        "json": sum(1 for f in files if f.endswith(".json")),
        "patch": sum(1 for f in files if f.endswith(".patch")),
        "dirs": {k: under(f"porting/{k}/") for k in
                 ("native", "behavior", "frontend", "server", "asset_pipeline", "material_pipeline", "progress")},
        "branches": len([b for b in git("branch", "-a", "--format=%(refname:short)").split("\n") if b.strip()]),
        "first": git("log", "--reverse", "--format=%ad", "--date=short").split("\n")[0],
        "last": git("log", "-1", "--format=%ad", "--date=short").strip(),
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
    basis = lambda k: sum(1 for l in leaves if k in (l.get("basis") or ""))

    def tone(pct):
        return "2e8540" if pct >= 50 else "97ca00" if pct >= 25 else "dfb317" if pct >= 10 else "fe7d37" if pct >= 5 else "e05d44"

    short = {"platform": "platform", "assets": "assets", "maps": "maps", "gameplay": "gameplay",
             "net": "network", "backend": "backend", "audio": "audio", "visuals": "visuals", "ui": "frontend UI"}
    area = {g["id"]: badge(short.get(g["id"], g["id"]), f"{g['pct']:.1f}%", tone(g["pct"])) for g in d["groups"]}
    areas = [area[k] for k in short if k in area]
    ms = {m["id"]: "done" if m["done"] else "open" for m in d["milestones"]}
    mc = lambda k: "2e8540" if ms.get(k) == "done" else "9f9f9f"
    L, N, D = st["lines"], st["count"], st["dirs"]
    rows = [
        [  # progress
            badge("overall", f"{d['overall_pct']:.1f}%", "2a78d6"),
            badge("evidence", f"{d['measured_pct']:.1f}%", "3987e5"),
            badge("objective", f"{d['objective_pct']:.1f}%", "5a9cec"),
            badge("milestones", f"{d['milestones_done']}/{d['milestones_total']}", "d6a02a"),
            badge("units", f"{human(passed)}/{human(checks)}", "0e7490"),
            badge("snapshot", d["head"][:8], "9f9f9f", "git"),
            badge("updated", st["last"], "9f9f9f"),
        ],
        [  # milestones and scope
            badge("M1 menu", ms.get("m1", "open"), mc("m1")),
            badge("M2 lobby", ms.get("m2", "open"), mc("m2")),
            badge("M3 matchmaking", ms.get("m3", "open"), mc("m3")),
            badge("M4 match", ms.get("m4", "open"), mc("m4")),
            badge("client", "unstripped", "7d4cdb"),
            badge("server", "reconstructed", "e8a317"),
            badge("goal", "full A/V", "a41e22"),
        ],
        [  # target
            badge("Fortnite", "12.60", "7d4cdb", "epicgames"),
            badge("build", "CL 13477524", "6a3fc4"),
            badge("UE", "4.25.0", "0e1128", "unrealengine"),
            badge("platform", "Linux x64", "e8a317", "linux"),
            badge("renderer", "Vulkan", "a41e22", "vulkan"),
            badge("oracle", "Wine", "722f37", "wine"),
            badge("original", "Win64", "0078d4"),
        ],
        [  # files by kind
            badge("C++", f"{human(st['cpp_c'])} .cpp", "00599c", "cplusplus"),
            badge("headers", f"{human(st['cpp_h'])} .h", "0e7490"),
            badge("recovered", f"{human(st['cpp_i'])} .inc", "d97757"),
            badge("Python", f"{N['py']} scripts", "3776ab", "python"),
            badge("C#", f"{N['cs']} files", "512bd4"),
            badge("shell", f"{N['sh']} scripts", "4eaa25", "gnubash"),
            badge("patches", st["patch"], "6e5494"),
        ],
        [  # lines by kind
            badge("C++ lines", human(L["cpp"]), "00599c"),
            badge("Python lines", human(L["py"]), "3776ab"),
            badge("C# lines", human(L["cs"]), "512bd4"),
            badge("shell lines", human(L["sh"]), "4eaa25"),
            badge("docs", f"{human(L['md'])} lines", "0e7490"),
            badge("markdown", f"{N['md']} files", "6e5494"),
            badge("data", f"{st['json']} JSON", "b08800"),
        ],
        [  # repo
            badge("tracked files", human(st["files"]), "0e7490"),
            badge("commits", human(st["commits"]), "1f6feb", "git"),
            badge("branches", st["branches"], "7d4cdb"),
            badge("since", st["first"], "9f9f9f"),
            badge("authored by", "Codex", "10a37f"),
            badge("authored by", "Claude", "d97757", "anthropic"),
            badge("status", "WIP", "dfb317"),
        ],
        [  # game data (counts from porting/README.md "Current evidence")
            badge("paks", "31 · v10", "5b4a8a"),
            badge("pak entries", "963,427", "0e7490"),
            badge(".uasset", "450,951", "2e8540"),
            badge("maps", "3,694", "b08800"),
            badge("plugins", "123", "a41e22"),
            badge("modules", "6", "512bd4"),
            badge("API routes", "59", "e8a317"),
        ],
        areas[:7],
        [  # remaining areas and model shape
            *areas[7:],
            badge("components", len(leaves), "2a78d6"),
            badge("areas", len(d["groups"]), "7d4cdb"),
            *([badge("class parity", f"{parity.group(1)}/{parity.group(2)}", "2e8540")] if parity else []),
            badge("git-measured", f"{basis('git')} leaves", "4eaa25"),
            badge("manual", f"{basis('manual')} leaves", "9f9f9f"),
        ],
        [  # tracked files per tree
            badge("native", f"{human(D['native'])} files", "00599c"),
            badge("behavior", f"{human(D['behavior'])} files", "b5301f"),
            badge("frontend", f"{human(D['frontend'])} files", "7d4cdb"),
            badge("server", f"{D['server']} files", "e8a317"),
            badge("assets", f"{D['asset_pipeline']} files", "2e8540"),
            badge("materials", f"{D['material_pipeline']} files", "d97757"),
            badge("dashboard", f"{D['progress']} files", "2a78d6"),
        ],
        [  # toolchain
            badge("extraction", "CUE4Parse", "512bd4"),
            badge("decompiler", "Ghidra 12.2", "b5301f"),
            badge("pak index", "v10", "5b4a8a"),
            badge("net", "IpNetDriver", "a41e22"),
            badge("UI", "CommonUI", "7d4cdb"),
            badge("parity", "checksum", "2e8540"),
            badge("tooling", "Python", "3776ab", "python"),
        ],
        [  # process
            badge("agents", "≤3 at once", "d97757", "anthropic"),
            badge("commits", "each batch", "1f6feb"),
            badge("history", "per commit", "2a78d6"),
            badge("evidence", "ledgered", "2e8540"),
            badge("keys", "git-ignored", "e05d44"),
            badge("assets", "git-ignored", "fe7d37"),
            badge("checks", "batched", "dfb317"),
        ],
    ]
    # Reading order: status, areas, target, game data, toolchain, code, repo, process.
    order = [0, 1, 7, 8, 2, 6, 10, 3, 4, 9, 5, 11]
    return "\n".join([BSTART, *(f'<p align="center">{" ".join(rows[i])}</p>' for i in order), BEND])


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
