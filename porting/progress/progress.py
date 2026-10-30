#!/usr/bin/env python3
"""Generate the Fortnite 12.60 port progress dashboard (stdlib only, offline).

Inputs:
  porting/progress/components.json  hand-edited model (weights, manual %, measures)
  git commit contents under porting/ (HEAD, or any commit when seeding)
  ignored local outputs under porting/generated/ (read from disk; these are
  current, not historical, so history replays mark them as carried)

Outputs (regenerated, git-ignored):
  porting/progress/out/index.html, progress.svg, progress.json, history.html
  porting/progress/history.json  one point per commit (git-ignored)

Usage:
  python3 porting/progress/progress.py            record HEAD, then render
  python3 porting/progress/progress.py --seed     replay every commit, then render
  python3 porting/progress/progress.py --no-record render from HEAD without writing history
  --quiet suppresses the summary line (used by the post-commit hook).
"""

import argparse
import fcntl
import fnmatch
import html
import io
import json
import math
import os
import re
import subprocess
import sys
import tarfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
COMPONENTS = os.path.join(HERE, "components.json")
OUT = os.path.join(HERE, "out")
HISTORY = os.path.join(HERE, "history.json")
LOCK = os.path.join(OUT, ".lock")

# Five colour bins, red -> amber -> green. Validated for colour-vision separation
# (adjacent CVD deltaE >= 14.9, normal-vision >= 15.4, OKLab x100); see README.
BINS = [
    ("#a8102a", "0-19%"),
    ("#e0573a", "20-39%"),
    ("#f2b705", "40-59%"),
    ("#3b9a3f", "60-79%"),
    ("#006b3c", "80-100%"),
]

TREEMAP_W, TREEMAP_H = 1000, 640
STANDALONE_W, STANDALONE_H = 1000, 720


# ---------------------------------------------------------------- git / snapshots

def git(*args):
    return subprocess.run(["git", "-C", REPO, *args], capture_output=True,
                          text=True, check=True).stdout


def resolve_head():
    sha = git("rev-parse", "HEAD").strip()
    subject = git("log", "-1", "--format=%s", sha).strip()
    date = git("log", "-1", "--format=%cI", sha).strip()
    return sha, subject, date


def snapshot(rev):
    """Committed files under porting/ at rev, as {repo-relative path: text}."""
    proc = subprocess.run(["git", "-C", REPO, "archive", "--format=tar", rev, "porting"],
                          capture_output=True)
    if proc.returncode != 0:
        # Commits before porting/ existed have no tree to measure.
        return {}
    raw = proc.stdout
    files = {}
    with tarfile.open(fileobj=io.BytesIO(raw)) as tf:
        for member in tf.getmembers():
            if not member.isfile() or member.size > 4 << 20:
                continue
            data = tf.extractfile(member).read()
            if b"\0" in data[:4096]:
                continue
            files[member.name] = data.decode("utf-8", "replace")
    return files


class Ctx:
    """Measurement context: committed snapshot plus ignored local outputs."""

    def __init__(self, files, replay, census_order):
        self.files = files
        self.replay = replay          # True when measuring a historical commit
        self.census_order = census_order
        self.used_local = False       # set per leaf
        self._cache = {}
        self._assign = None

    def reset_leaf(self):
        self.used_local = False

    def tracked(self, pattern):
        pats = pattern if isinstance(pattern, list) else [pattern]
        return [(p, t) for p, t in self.files.items()
                if any(fnmatch.fnmatchcase(p, pat) for pat in pats)]

    def local_path(self, rel):
        return os.path.join(REPO, rel)

    def local_text(self, rel):
        path = self.local_path(rel)
        if not os.path.isfile(path):
            return None
        self.used_local = True
        with open(path, encoding="utf-8", errors="replace") as fh:
            return fh.read()

    def local_exists(self, rel):
        ok = os.path.exists(self.local_path(rel))
        if ok:
            self.used_local = True
        return ok

    # --- derived inputs (cached) ---
    def routes(self):
        if "routes" not in self._cache:
            text = self.local_text("porting/generated/routes.json")
            self._cache["routes"] = json.loads(text).get("routes", []) if text else None
        return self._cache["routes"]

    def declared_natives(self):
        if "declared" not in self._cache:
            declared = set()
            for _, text in self.tracked("porting/native/Source/**"):
                declared.update(re.findall(r"class\s+FORTNITEGAME_API\s+(\w+)", text))
            self._cache["declared"] = declared
        return self._cache["declared"]

    def census_names(self):
        if "census" not in self._cache:
            text = self.local_text("porting/generated/map-native-imports.jsonl")
            if text is None:
                self._cache["census"] = None
            else:
                names = set()
                for line in text.splitlines():
                    if not line.startswith("{"):
                        continue
                    try:
                        row = json.loads(line)
                    except ValueError:
                        continue
                    for imp in row.get("nativeImports", []):
                        ref = imp.split("|")[0]
                        prefix = "/Script/FortniteGame."
                        if ref.startswith(prefix):
                            names.add(ref[len(prefix):])
                self._cache["census"] = sorted(names)
        return self._cache["census"]

    def census_assignment(self):
        """Each FortniteGame import belongs to exactly one census leaf (first match wins)."""
        if self._assign is None:
            self._assign = {}
            names = self.census_names() or []
            remainder = None
            for leaf_id, pattern in self.census_order:
                if pattern == "*remainder*":
                    remainder = leaf_id
            for name in names:
                owner = None
                for leaf_id, pattern in self.census_order:
                    if pattern != "*remainder*" and re.search(pattern, name):
                        owner = leaf_id
                        break
                self._assign[name] = owner or remainder
        return self._assign


def is_declared(name, declared):
    return any((prefix + name) in declared for prefix in ("A", "U", "F", "E", "")) or name in declared


def route_stem(route):
    stem = re.split(r"[`{%?]", route)[0].rstrip("/")
    return stem


# ---------------------------------------------------------------- measurers

def m_checks(spec, ctx):
    checks = spec["checks"]
    passed = 0
    lines = []
    for chk in checks:
        label = chk["label"]
        if "file" in chk:
            ok = chk["file"] in ctx.files
        elif "local" in chk:
            ok = ctx.local_exists(chk["local"])
        else:
            ok = False
            for _, text in ctx.tracked(chk["in"]):
                if re.search(chk["pattern"], text):
                    ok = True
                    break
        passed += bool(ok)
        lines.append(("ok " if ok else "missing ") + label)
    total = len(checks)
    credit = spec.get("stub_credit", 1.0)
    pct = 100.0 * passed / total * credit if total else 0.0
    evidence = "; ".join(lines)
    if credit != 1.0:
        evidence += f" (credit {credit:g}: present in source, not exercised against the client)"
    return {"pct": pct, "done": passed, "total": total, "evidence": evidence}


def m_routes(spec, ctx):
    routes = ctx.routes()
    if routes is None:
        return {"pct": 0.0, "done": 0, "total": 0,
                "evidence": "routes inventory missing (porting/generated/routes.json)"}
    include = spec.get("include", [])
    exclude = spec.get("exclude", [])
    selected = [r for r in routes
                if any(k in r for k in include) and not any(k in r for k in exclude)]
    server_text = "\n".join(text for _, text in ctx.tracked("porting/server/**"))
    covered = [r for r in selected if route_stem(r) and route_stem(r) in server_text]
    credit = spec.get("stub_credit", 1.0)
    total = len(selected)
    pct = 100.0 * len(covered) / total * credit if total else 0.0
    return {"pct": pct, "done": len(covered), "total": total,
            "evidence": f"{len(covered)} of {total} client route templates have their literal path in committed server code "
                        f"(stub credit {credit:g}); templates from the static client scan"}


def m_census(spec, ctx):
    names = ctx.census_names()
    if names is None:
        return {"pct": 0.0, "done": 0, "total": 0,
                "evidence": "census missing (porting/generated/map-native-imports.jsonl)"}
    leaf_id = spec["_leaf_id"]
    members = sorted(n for n, owner in ctx.census_assignment().items() if owner == leaf_id)
    declared = ctx.declared_natives()
    done = [n for n in members if is_declared(n, declared)]
    credit = spec.get("stub_credit", 1.0)
    total = len(members)
    pct = 100.0 * len(done) / total * credit if total else 0.0
    sample = ", ".join(done[:6]) + (" ..." if len(done) > 6 else "")
    ev = f"{len(done)} of {total} /Script/FortniteGame imports in this group have a native declaration"
    if done:
        ev += f" ({sample})"
    ev += f"; declared stubs credited at {credit:g}, not behaviour"
    return {"pct": pct, "done": len(done), "total": total, "evidence": ev}


def m_fraction(spec, ctx):
    total = spec["total"]
    if "done" in spec:
        done = spec["done"]
        ev = f"{done} of {total} ({spec.get('source', 'documented')})"
    else:
        text = ctx.local_text(spec["count_file"])
        if text is None:
            done = 0
            ev = f"local data missing: {spec['count_file']}"
        else:
            node = json.loads(text)
            for key in filter(None, spec.get("count_key", "").split(".")):
                node = node[key]
            done = len(node) if isinstance(node, (list, dict)) else int(node)
            ev = f"{done} of {total} (local output {os.path.basename(spec['count_file'])})"
    pct = 100.0 * done / total if total else 0.0
    return {"pct": pct, "done": done, "total": total, "evidence": ev}


def m_jsonl_rows(spec, ctx):
    text = ctx.local_text(spec["file"])
    total = spec["total"]
    if text is None:
        return {"pct": 0.0, "done": 0, "total": total, "evidence": f"local data missing: {spec['file']}"}
    done = 0
    for line in text.splitlines():
        if line.startswith("{"):
            try:
                json.loads(line)
                done += 1
            except ValueError:
                pass
    pct = 100.0 * min(done, total) / total
    return {"pct": pct, "done": min(done, total), "total": total,
            "evidence": f"{done} map header rows parsed of {total} (local census output)"}


def m_local_glob(spec, ctx):
    total = spec["total"]
    suffix = spec.get("suffix", ".uasset")
    count = 0
    for root in spec["roots"]:
        base = ctx.local_path(root)
        if not os.path.isdir(base):
            continue
        ctx.used_local = True
        for _, _, names in os.walk(base):
            count += sum(1 for n in names if n.endswith(suffix))
    pct = 100.0 * min(count, total) / total
    return {"pct": pct, "done": count, "total": total,
            "evidence": f"{count} local {suffix} files of {total} indexed (recovered or converted; ignored outputs)"}


def m_manual(spec, ctx):
    return {"pct": float(spec["pct"]), "done": None, "total": None,
            "evidence": "manual estimate (see rationale)"}


def m_census_dispatch(spec, ctx):
    return m_census(spec, ctx)


MEASURERS = {
    "checks": m_checks,
    "routes": m_routes,
    "census": m_census_dispatch,
    "fraction": m_fraction,
    "jsonl_rows": m_jsonl_rows,
    "local_glob": m_local_glob,
    "manual": m_manual,
}

BASIS_BY_KIND = {
    "checks": "git",
    "routes": "git+local",
    "census": "git+local",
    "fraction": "local",
    "jsonl_rows": "local",
    "local_glob": "local",
    "manual": "manual",
}


def measure_leaf(node, ctx):
    spec = dict(node["measure"])
    spec["_leaf_id"] = node["id"]
    kind = spec["kind"]
    ctx.reset_leaf()
    result = MEASURERS[kind](spec, ctx)
    pct = result["pct"]
    evidence = result["evidence"]
    if not ctx.files:
        # Commits from before porting/ existed: nothing of the port is present.
        pct = 0.0
        result["done"] = 0
        evidence = "no porting/ tree at this commit"
    if "ceiling" in node and pct > node["ceiling"]:
        evidence += f"; capped at {node['ceiling']}% (ceiling in components.json)"
        pct = float(node["ceiling"])
    if kind == "manual":
        parts = ["manual"]
    elif kind == "fraction" and "done" in spec:
        parts = ["documented"]
    else:
        parts = []
        if kind in ("checks", "routes", "census"):
            parts.append("git")
        if ctx.used_local or kind in ("fraction", "jsonl_rows", "local_glob"):
            parts.append("local")
    carried = bool(ctx.replay and ctx.used_local and "local" in parts)
    if carried:
        parts.append("carried")
    basis = "+".join(parts)
    return {
        "id": node["id"], "name": node["name"], "kind": "leaf",
        "weight": float(node["weight"]), "pct": round(pct, 3),
        "done": result["done"], "total": result["total"],
        "evidence": evidence, "basis": basis, "carried": carried,
        "rationale": node.get("rationale", ""),
    }


def evaluate(node, ctx):
    if "children" in node:
        kids = [evaluate(c, ctx) for c in node["children"]]
        weight = sum(k["weight"] for k in kids)
        pct = sum(k["weight"] * k["pct"] for k in kids) / weight if weight else 0.0
        return {"id": node["id"], "name": node["name"], "kind": "group",
                "weight": weight, "pct": round(pct, 3), "children": kids}
    return measure_leaf(node, ctx)


def walk_leaves(node):
    if node.get("kind") == "leaf":
        yield node
    for child in node.get("children", []):
        yield from walk_leaves(child)


def walk_spec(nodes):
    for node in nodes:
        if "children" in node:
            yield from walk_spec(node["children"])
        else:
            yield node


def census_order_from_spec(tree):
    order = []
    for node in walk_spec(tree):
        if node.get("measure", {}).get("kind") == "census":
            order.append((node["id"], node["measure"]["match"]))
    return order


def measure_tree(model, files, replay):
    ctx = Ctx(files, replay, census_order_from_spec(model["tree"]))
    groups = [evaluate(node, ctx) for node in model["tree"]]
    leaves = [leaf for g in groups for leaf in walk_leaves(g)]
    total_w = sum(leaf["weight"] for leaf in leaves)
    overall = sum(leaf["weight"] * leaf["pct"] for leaf in leaves) / total_w if total_w else 0.0
    milestones = model.get("milestones", [])
    return {
        "overall_pct": round(overall, 3),
        "total_weight": round(total_w, 3),
        "groups": groups,
        "leaves": leaves,
        "milestones_done": sum(1 for m in milestones if m.get("done")),
        "milestones_total": len(milestones),
        "milestones": milestones,
    }


# ---------------------------------------------------------------- layout

def squarify(weights, x, y, w, h):
    """Bruls et al. squarified treemap. Returns rectangles in input order."""
    n = len(weights)
    if n == 0 or w <= 0 or h <= 0:
        return [(x, y, 0.0, 0.0)] * n
    weights = [max(float(v), 1e-9) for v in weights]
    total = sum(weights)
    scale = w * h / total
    order = sorted(range(n), key=lambda i: -weights[i])
    areas = [weights[i] * scale for i in order]
    rects = [None] * n

    def worst(row, side):
        s = sum(row)
        return max(side * side * max(row) / (s * s), (s * s) / (side * side * min(row)))

    i = 0
    while i < n:
        side = min(w, h)
        row = [areas[i]]
        idx = [i]
        j = i + 1
        while j < n and worst(row + [areas[j]], side) <= worst(row, side):
            row.append(areas[j])
            idx.append(j)
            j += 1
        s = sum(row)
        if w >= h:
            col_w = s / h
            cy = y
            for k, area in zip(idx, row):
                seg = area / col_w
                rects[order[k]] = (x, cy, col_w, seg)
                cy += seg
            x += col_w
            w -= col_w
        else:
            row_h = s / w
            cx = x
            for k, area in zip(idx, row):
                seg = area / row_h
                rects[order[k]] = (cx, y, seg, row_h)
                cx += seg
            y += row_h
            h -= row_h
        i = j
    return rects


def pct_color(pct):
    bin_index = min(len(BINS) - 1, max(0, int(pct // 20)))
    return BINS[bin_index][0]


def _lum(hex_color):
    def chan(c):
        c = c / 255.0
        return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4
    h = hex_color.lstrip("#")
    r, g, b = (int(h[i:i + 2], 16) for i in (0, 2, 4))
    return 0.2126 * chan(r) + 0.7152 * chan(g) + 0.0722 * chan(b)


def text_on(fill):
    lf = _lum(fill)
    dark, light = _lum("#111111"), _lum("#ffffff")
    return "#111111" if (lf + 0.05) / (dark + 0.05) >= (light + 0.05) / (lf + 0.05) else "#ffffff"


def fmt_pct(pct):
    return f"{pct:.1f}%" if pct < 10 else f"{pct:.0f}%"


def wrap_text(text, width_px, font_px, max_lines):
    cpl = max(4, int((width_px - 8) / (font_px * 0.56)))
    words = text.split()
    lines, cur = [], ""
    for word in words:
        cand = (cur + " " + word).strip()
        if len(cand) <= cpl:
            cur = cand
        else:
            if cur:
                lines.append(cur)
            cur = word
    if cur:
        lines.append(cur)
    lines = [ln if len(ln) <= cpl else ln[:cpl - 1] + "…" for ln in lines]
    if len(lines) > max_lines:
        lines = lines[:max_lines]
        lines[-1] = (lines[-1][:cpl - 1] + "…")
    return lines


def tooltip_text(node):
    basis = node.get("basis", "")
    done = node.get("done")
    total = node.get("total")
    frac = f" ({done}/{total})" if done is not None and total else ""
    return (f"{node['name']}\n{node['pct']:.1f}% complete{frac}\n"
            f"effort weight {node['weight']:g} | source: {basis or 'derived'}\n"
            f"{node.get('evidence', '')}")


def render_treemap(groups, width, height, css_class="tm"):
    """Squarified treemap: categories > components > sub-components as nested tiles."""
    parts = []
    leaves_out = []   # (node, rect)
    groups_out = []   # (node, rect, depth)

    def place(nodes, x, y, w, h, depth):
        ordered = sorted(nodes, key=lambda n: -n["weight"])
        rects = squarify([n["weight"] for n in ordered], x, y, w, h)
        for node, (rx, ry, rw, rh) in zip(ordered, rects):
            if "children" in node:
                head = 24 if depth == 0 else 17
                pad = 3
                groups_out.append((node, rx, ry, rw, rh, depth, head))
                if rw > 2 * pad + 24 and rh > head + 2 * pad + 18:
                    place(node["children"], rx + pad, ry + head, rw - 2 * pad, rh - head - 2 * pad, depth + 1)
                else:
                    leaves_out.append((node, rx, ry, rw, rh))
            else:
                leaves_out.append((node, rx, ry, rw, rh))

    place(groups, 0, 0, width, height, 0)

    parts.append(f'<rect class="{css_class}-bg" x="0" y="0" width="{width}" height="{height}"/>')
    for node, x, y, w, h, depth, head in groups_out:
        title = html.escape(tooltip_text(node))
        cls = "grp0" if depth == 0 else "grp1"
        parts.append(f'<g><title>{title}</title>')
        parts.append(f'<rect class="{css_class}-{cls}" x="{x:.1f}" y="{y:.1f}" width="{max(w, 0):.1f}" height="{max(h, 0):.1f}"/>')
        if w > 90 and h > head + 6:
            label = f"{node['name']}  ·  {fmt_pct(node['pct'])}"
            maxc = int((w - 10) / 6.6)
            if len(label) > maxc:
                label = label[:max(4, maxc - 1)] + "…"
            font = 13 if depth == 0 else 11
            parts.append(f'<text class="{css_class}-head" x="{x + 6:.1f}" y="{y + head - 6:.1f}" font-size="{font}">{html.escape(label)}</text>')
        parts.append('</g>')

    for node, x, y, w, h in leaves_out:
        fill = pct_color(node["pct"])
        ink = text_on(fill)
        inset = 1.0
        rx, ry, rw, rh = x + inset, y + inset, max(w - 2 * inset, 0), max(h - 2 * inset, 0)
        parts.append(f'<g><title>{html.escape(tooltip_text(node))}</title>')
        parts.append(f'<rect x="{rx:.1f}" y="{ry:.1f}" width="{rw:.1f}" height="{rh:.1f}" rx="3" fill="{fill}"/>')
        if rw >= 60 and rh >= 34:
            lines = wrap_text(node["name"], rw, 11, max(1, int((rh - 30) / 13)))
            pct_s = fmt_pct(node["pct"])
            ty = ry + 15
            for ln in lines:
                parts.append(f'<text x="{rx + 6:.1f}" y="{ty:.1f}" font-size="11" fill="{ink}">{html.escape(ln)}</text>')
                ty += 13
            if rh >= 34 + 13 * len(lines):
                parts.append(f'<text x="{rx + 6:.1f}" y="{ry + rh - 7:.1f}" font-size="15" font-weight="700" fill="{ink}">{pct_s}</text>')
        elif rw >= 36 and rh >= 22:
            parts.append(f'<text x="{rx + 5:.1f}" y="{ry + rh / 2 + 5:.1f}" font-size="12" font-weight="700" fill="{ink}">{fmt_pct(node["pct"])}</text>')
        parts.append('</g>')

    return "\n".join(parts)


TREEMAP_CSS = """
.tm-bg{fill:#fcfcfb}
.tm-grp0{fill:none;stroke:#8a8a86;stroke-width:1.5}
.tm-grp1{fill:none;stroke:#6f6f6a;stroke-width:1}
.tm-head{font-family:system-ui,-apple-system,Segoe UI,sans-serif;font-weight:600;fill:#1d1d1b}
@media (prefers-color-scheme: dark){
  .tm-bg{fill:#1a1a19}
  .tm-grp0{stroke:#7d7d78}
  .tm-grp1{stroke:#5f5f5a}
  .tm-head{fill:#f0efec}
}
""".strip()


def svg_block(groups, width, height, extra_head=""):
    body = render_treemap(groups, width, height)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
            f'width="100%" role="img" aria-label="Treemap of port progress: area is effort weight, colour is percent complete" '
            f'font-family="system-ui,-apple-system,Segoe UI,sans-serif">'
            f'<style>{TREEMAP_CSS}</style>{extra_head}{body}</svg>')


# ---------------------------------------------------------------- rendering helpers

def esc(s):
    return html.escape(str(s))


def flatten_rows(groups, depth=0):
    rows = []
    for g in groups:
        if "children" in g:
            rows.append((depth, g))
            rows.extend(flatten_rows(g["children"], depth + 1))
        else:
            rows.append((depth, g))
    return rows


def legend_html():
    items = []
    for color, label in BINS:
        items.append(f'<li><span class="swatch" style="background:{color}"></span>{esc(label)}</li>')
    return '<ul class="legend">' + "".join(items) + "</ul>"


PAGE_CSS = """
:root{color-scheme:light;--surface:#fcfcfb;--ink:#161614;--ink2:#4f4e4a;--line:#d9d8d3;--card:#f3f2ee;--accent:#2a78d6;--warn:#8a4b00;--warnbg:#fff4df}
@media (prefers-color-scheme: dark){
  :root:not([data-theme="light"]){color-scheme:dark;--surface:#1a1a19;--ink:#f0efec;--ink2:#c3c2b7;--line:#3a3a37;--card:#23231f;--accent:#3987e5;--warn:#f5c26b;--warnbg:#3a2c12}
}
:root[data-theme="dark"]{color-scheme:dark;--surface:#1a1a19;--ink:#f0efec;--ink2:#c3c2b7;--line:#3a3a37;--card:#23231f;--accent:#3987e5;--warn:#f5c26b;--warnbg:#3a2c12}
*{box-sizing:border-box}
body{margin:0;background:var(--surface);color:var(--ink);font:15px/1.5 system-ui,-apple-system,Segoe UI,sans-serif}
main{max-width:1200px;margin:0 auto;padding:16px}
h1{font-size:1.6rem;margin:.2rem 0 .4rem}
h2{font-size:1.2rem;margin:1.6rem 0 .5rem;border-bottom:1px solid var(--line);padding-bottom:.2rem}
a{color:var(--accent)}
code{font-size:.9em}
.meta{color:var(--ink2);font-size:.9rem}
.defn{background:var(--card);border-left:4px solid var(--accent);padding:12px 14px;border-radius:6px}
.defn strong{display:block;margin-bottom:.2rem}
.warn{background:var(--warnbg);color:var(--warn);padding:8px 12px;border-radius:6px}
.stats{display:flex;flex-wrap:wrap;gap:12px;margin:14px 0}
.stat{background:var(--card);border-radius:8px;padding:10px 14px;min-width:150px}
.stat .big{font-size:2rem;font-weight:700;line-height:1.1}
.stat .lbl{color:var(--ink2);font-size:.85rem}
.legend{list-style:none;display:flex;flex-wrap:wrap;gap:8px 14px;padding:0;margin:8px 0;font-size:.9rem}
.legend li{display:flex;align-items:center;gap:6px}
.swatch{display:inline-block;width:18px;height:18px;border-radius:3px;border:1px solid var(--line)}
.tm-wrap{width:100%}
.tm-wrap svg{display:block;width:100%;height:auto}
@media (max-width:640px){
  /* Keep tile labels legible: the chart scrolls inside its own box, the page does not. */
  .tm-wrap{overflow-x:auto}
  .tm-wrap svg{min-width:760px}
}
table{width:100%;border-collapse:collapse;font-size:.9rem}
th,td{text-align:left;padding:5px 6px;border-bottom:1px solid var(--line);vertical-align:top}
th{color:var(--ink2);font-weight:600}
td.num{text-align:right;white-space:nowrap;font-variant-numeric:tabular-nums}
tr.group td{font-weight:700;background:var(--card)}
tr.sub td.name{padding-left:18px}
.tablewrap{overflow-x:auto}
.ev{color:var(--ink2);font-size:.85rem}
ul.ms{padding-left:1.2rem}
footer{color:var(--ink2);font-size:.85rem;margin:2rem 0 1rem}
@media (max-width:640px){
  main{padding:12px}
  h1{font-size:1.3rem}
  .stat .big{font-size:1.6rem}
  table thead{display:none}
  table,tr,td{display:block;width:100%}
  tr{border-bottom:1px solid var(--line);padding:6px 0}
  td{border:0;padding:2px 4px}
  td.num{text-align:left}
  td.ev{font-size:.8rem}
}
""".strip()


def index_html(res, head, subject, date, model, when_note):
    overall = res["overall_pct"]
    rows = []
    for depth, node in flatten_rows(res["groups"]):
        if "children" in node:
            rows.append(
                f'<tr class="group"><td class="name" colspan="2">{esc(node["name"])}</td>'
                f'<td class="num">{node["weight"]:g}</td><td class="num">{fmt_pct(node["pct"])}</td>'
                f'<td colspan="2"></td></tr>')
        else:
            cls = "sub" if depth > 1 else ""
            frac = f'{node["done"]}/{node["total"]}' if node.get("done") is not None and node.get("total") else "-"
            rows.append(
                f'<tr class="{cls}"><td class="name">{esc(node["name"])}</td>'
                f'<td class="num">{node["weight"]:g}</td>'
                f'<td class="num">{frac}</td>'
                f'<td class="num"><span class="swatch" style="background:{pct_color(node["pct"])};width:10px;height:10px;display:inline-block"></span> {node["pct"]:.1f}%</td>'
                f'<td class="ev">{esc(node.get("basis", ""))}</td>'
                f'<td class="ev">{esc(node.get("evidence", ""))}<br><em>{esc(node.get("rationale", ""))}</em></td></tr>')
    milestones = "".join(
        f'<li>{"[x]" if m.get("done") else "[ ]"} {esc(m["text"])}<br><span class="ev">{esc(m.get("evidence", ""))}</span></li>'
        for m in res["milestones"])
    cats = "".join(f'<div class="stat"><div class="lbl">{esc(g["name"])}</div><div class="big" style="font-size:1.3rem">{fmt_pct(g["pct"])}</div></div>'
                   for g in res["groups"])
    svg = svg_block(res["groups"], TREEMAP_W, TREEMAP_H)
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fortnite 12.60 port progress</title>
<style>{PAGE_CSS}</style></head>
<body><main>
<header>
<h1>Fortnite 12.60 Linux port: progress</h1>
<p class="meta">Measured at commit <code>{esc(head[:7])}</code> &middot; {esc(subject)} &middot; {esc(date)}.
{esc(when_note)} &middot; <a href="history.html">history</a> &middot; <a href="progress.svg">progress.svg</a> &middot; <a href="progress.json">progress.json</a></p>
</header>

<section class="defn">
<strong>Definition of 100%</strong>
{esc(model["definition"])}
</section>
<p class="warn" style="margin-top:10px">Overall progress is an effort-weighted estimate of reconstruction work. It is not a measure of the Definition, which is far from met. Milestones reached: {res["milestones_done"]} of {res["milestones_total"]}.</p>

<div class="stats">
<div class="stat"><div class="lbl">Overall (effort-weighted)</div><div class="big">{overall:.1f}%</div></div>
<div class="stat"><div class="lbl">Components measured</div><div class="big">{len(res["leaves"])}</div></div>
<div class="stat"><div class="lbl">Milestones reached</div><div class="big">{res["milestones_done"]} / {res["milestones_total"]}</div></div>
</div>

<h2>Milestones toward the Definition</h2>
<ul class="ms">{milestones}</ul>

<h2>Treemap</h2>
<p class="meta">Tile area = estimated effort weight. Tile colour = percent complete (red to green, five bins). Each tile shows its percentage, so colour is never the only cue. Hover a tile for done/total, source and evidence.</p>
{legend_html()}
<div class="tm-wrap">{svg}</div>

<h2>Category summary</h2>
<div class="stats">{cats}</div>

<h2>Components</h2>
<p class="meta">Basis: <em>git</em> = committed sources at HEAD; <em>local</em> = ignored outputs under porting/generated/ read from disk; <em>manual</em> = estimate in components.json; <em>documented</em> = count taken from the porting docs; <em>carried</em> = a history replay that used current local data. Ceilings cap a measured value where the measure overstates progress.</p>
<div class="tablewrap"><table>
<thead><tr><th>Component</th><th>Weight</th><th>Done / total</th><th>Complete</th><th>Basis</th><th>Evidence and rationale</th></tr></thead>
<tbody>{''.join(rows)}</tbody>
</table></div>

<h2>How this is measured</h2>
<p class="meta">Weights and manual percentages are edited in <code>porting/progress/components.json</code>. The generator is <code>porting/progress/progress.py</code>, which reads committed files via <code>git archive</code> at HEAD and ignored local outputs. It records one history point per commit through <code>.githooks/post-commit</code>. Measures only count what is committed; uncommitted stubs and generated artifacts do not add progress unless they are local outputs named in a measure.</p>
<footer>Generated offline by porting/progress/progress.py. No external scripts or fonts are loaded.</footer>
</main></body></html>
"""


def standalone_svg(res, head, subject, date):
    width, height = STANDALONE_W, STANDALONE_H
    header = (f'<text x="12" y="28" font-size="20" font-weight="700" class="tm-head">'
              f'Fortnite 12.60 port progress: overall {res["overall_pct"]:.1f}% (effort-weighted)</text>'
              f'<text x="12" y="48" font-size="12" class="tm-head" font-weight="400">'
              f'Commit {esc(head[:7])} | {esc(subject[:90])} | {esc(date)} | milestones {res["milestones_done"]}/{res["milestones_total"]}</text>')
    body = render_treemap(res["groups"], width, height - 60)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" '
            f'role="img" aria-label="Port progress treemap" font-family="system-ui,-apple-system,Segoe UI,sans-serif">'
            f'<style>{TREEMAP_CSS}</style>'
            f'<rect class="tm-bg" x="0" y="0" width="{width}" height="{height}"/>{header}'
            f'<g transform="translate(0,60)">{body}</g>'
            f'<text x="12" y="{height - 10}" font-size="11" class="tm-head" font-weight="400">'
            f'Definition of 100%: the original unmodified Fortnite 12.60 client under Linux/Wine reaches the lobby, matchmakes to our reconstructed server and plays a match with all visuals and audio.</text>'
            f'</svg>')


def history_html(history, model):
    entries = history
    n = len(entries)
    W, H = 900, 300
    pl, pr, pt, pb = 52, 16, 16, 34
    ymax = max([10.0] + [math.ceil(e["overall_pct"] / 5.0) * 5.0 for e in entries] + [5.0])
    ymax = max(ymax, 10.0)
    inner_w, inner_h = W - pl - pr, H - pt - pb

    def xy(i, v):
        x = pl + (inner_w * (i / (n - 1)) if n > 1 else inner_w / 2)
        y = pt + inner_h * (1 - v / ymax)
        return x, y

    grid = []
    for t in range(0, int(ymax) + 1, 5 if ymax > 10 else 2):
        _, y = xy(0, t)
        grid.append(f'<line x1="{pl}" x2="{W - pr}" y1="{y:.1f}" y2="{y:.1f}" style="stroke:var(--line)" stroke-width="1"/>'
                    f'<text x="{pl - 6}" y="{y + 4:.1f}" text-anchor="end" font-size="11" style="fill:var(--ink2)">{t}%</text>')
    pts = []
    dots = []
    for i, e in enumerate(entries):
        x, y = xy(i, e["overall_pct"])
        pts.append(f"{x:.1f},{y:.1f}")
        tip = esc(f'{e["short"]} {e["date"][:10]}: {e["overall_pct"]:.2f}% | {e["subject"]}')
        dots.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="4" fill="var(--accent)"><title>{tip}</title></circle>')
    line = f'<polyline points="{" ".join(pts)}" fill="none" stroke="var(--accent)" stroke-width="2"/>' if n > 1 else ""
    xlabels = ""
    if n:
        first, last = entries[0], entries[-1]
        xlabels = (f'<text x="{pl}" y="{H - 10}" font-size="11" fill="var(--ink2)">{esc(first["short"])}</text>'
                   f'<text x="{W - pr}" y="{H - 10}" text-anchor="end" font-size="11" fill="var(--ink2)">{esc(last["short"])}</text>')
    chart = (f'<svg viewBox="0 0 {W} {H}" width="100%" role="img" aria-label="Overall progress per commit">'
             f'{"".join(grid)}{line}{"".join(dots)}{xlabels}</svg>')

    cat_ids = [g["id"] for g in model["tree"]]
    cat_names = {g["id"]: g["name"] for g in model["tree"]}
    head_cells = "".join(f'<th>{esc(cat_names[c])}</th>' for c in cat_ids)
    rows = []
    for e in reversed(entries):
        cells = "".join(f'<td class="num">{e["categories"].get(c, 0):.1f}%</td>' for c in cat_ids)
        flag = ("replayed: manual estimates and local outputs as of now" if e.get("seeded")
                else "recorded at commit time")
        rows.append(f'<tr><td>{esc(e["date"][:16].replace("T", " "))}</td><td><code>{esc(e["short"])}</code></td>'
                    f'<td>{esc(e["subject"])}</td><td class="num"><strong>{e["overall_pct"]:.2f}%</strong></td>{cells}'
                    f'<td class="ev">{esc(flag)}</td></tr>')
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Port progress history</title>
<style>{PAGE_CSS}
svg text{{font-family:system-ui,-apple-system,Segoe UI,sans-serif}}
</style></head>
<body><main>
<header>
<h1>Port progress history</h1>
<p class="meta">One point per commit: overall effort-weighted progress. <a href="index.html">back to the dashboard</a> &middot; <a href="progress.json">progress.json</a></p>
</header>
<p class="warn">Definition of 100% is the original unmodified Fortnite 12.60 client under Linux/Wine reaching the lobby, matchmaking to our reconstructed server and playing a match with all visuals and audio. Overall values here are estimates, not that definition.</p>
<h2>Overall progress per commit</h2>
<div class="tm-wrap">{chart}</div>
<p class="meta">Points marked replay used the current components.json manual estimates and current ignored local outputs, so they are indicative. Hook-recorded points measure HEAD at commit time.</p>
<h2>Commits</h2>
<div class="tablewrap"><table>
<thead><tr><th>Date (UTC)</th><th>Commit</th><th>Subject</th><th>Overall</th>{head_cells}<th>Source</th></tr></thead>
<tbody>{''.join(rows)}</tbody>
</table></div>
<footer>Generated offline by porting/progress/progress.py.</footer>
</main></body></html>
"""


# ---------------------------------------------------------------- history

def entry_from(res, sha, subject, date, seeded):
    return {
        "commit": sha,
        "short": sha[:7],
        "subject": subject,
        "date": date,
        "overall_pct": res["overall_pct"],
        "categories": {g["id"]: round(g["pct"], 3) for g in res["groups"]},
        "leaves": {leaf["id"]: leaf["pct"] for leaf in res["leaves"]},
        "carried": sorted(leaf["id"] for leaf in res["leaves"] if leaf.get("carried")),
        "milestones_done": res["milestones_done"],
        "seeded": seeded,
    }


def load_json(path, default):
    try:
        with open(path, encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return default


def write_atomic(path, text):
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        fh.write(text)
    os.replace(tmp, path)


def measure_commit(model, sha, subject, date, head_sha, seeded):
    files = snapshot(sha)
    replay = sha != head_sha
    res = measure_tree(model, files, replay)
    return res, entry_from(res, sha, subject, date, seeded)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--seed", action="store_true", help="replay all commits and rebuild history.json")
    parser.add_argument("--no-record", action="store_true", help="render without writing history.json")
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args(argv)

    started = time.time()
    os.makedirs(OUT, exist_ok=True)
    with open(LOCK, "w") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        with open(COMPONENTS, encoding="utf-8") as fh:
            model = json.load(fh)
        head, subject, date = resolve_head()
        history = load_json(HISTORY, [])

        if args.seed:
            revs = git("rev-list", "--reverse", "HEAD").split()
            history = []
            for sha in revs:
                s_subject = git("log", "-1", "--format=%s", sha).strip()
                s_date = git("log", "-1", "--format=%cI", sha).strip()
                _, entry = measure_commit(model, sha, s_subject, s_date, head, seeded=(sha != head))
                history.append(entry)
            head_res, _ = measure_commit(model, head, subject, date, head, seeded=False)
        else:
            head_res, head_entry = measure_commit(model, head, subject, date, head, seeded=False)
            if not args.no_record:
                history = [e for e in history if e.get("commit") != head]
                history.append(head_entry)

        if not args.no_record or args.seed:
            write_atomic(HISTORY, json.dumps(history, indent=1, sort_keys=False) + "\n")

        # Render from the HEAD measurement. Overall, categories and milestones are the same
        # whether read from history or measured fresh.
        when_note = "Generated by porting/progress/progress.py."
        write_atomic(os.path.join(OUT, "index.html"), index_html(head_res, head, subject, date, model, when_note))
        write_atomic(os.path.join(OUT, "progress.svg"), standalone_svg(head_res, head, subject, date))
        progress_doc = {
            "definition": model["definition"],
            "head": head,
            "subject": subject,
            "date": date,
            "overall_pct": head_res["overall_pct"],
            "milestones": head_res["milestones"],
            "milestones_done": head_res["milestones_done"],
            "milestones_total": head_res["milestones_total"],
            "groups": head_res["groups"],
            "leaves": head_res["leaves"],
        }
        write_atomic(os.path.join(OUT, "progress.json"), json.dumps(progress_doc, indent=1) + "\n")
        write_atomic(os.path.join(OUT, "history.html"), history_html(history, model))

    if not args.quiet:
        print(f"progress: HEAD {head[:7]} overall {head_res['overall_pct']:.2f}% "
              f"({len(head_res['leaves'])} components, {len(history)} history points, "
              f"{time.time() - started:.1f}s) -> {os.path.relpath(OUT, REPO)}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except subprocess.CalledProcessError as exc:
        print(f"progress: git failed: {exc}", file=sys.stderr)
        sys.exit(1)
