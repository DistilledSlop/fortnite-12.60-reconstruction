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
    manual_w = sum(leaf["weight"] for leaf in leaves if leaf["basis"] == "manual")
    manual_c = sum(leaf["weight"] * leaf["pct"] for leaf in leaves if leaf["basis"] == "manual")
    evidence_c = overall * total_w - manual_c if total_w else 0.0
    milestones = model.get("milestones", [])
    return {
        "overall_pct": round(overall, 3),
        # The objective part of the overall: everything except manual estimates.
        "measured_pct": round(evidence_c / total_w, 3) if total_w else 0.0,
        "estimated_pct": round(manual_c / total_w, 3) if total_w else 0.0,
        "manual_weight_pct": round(100.0 * manual_w / total_w, 3) if total_w else 0.0,
        "manual_full_pct": round((evidence_c + manual_w * 100.0) / total_w, 3) if total_w else 0.0,
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


# Per-character advance width in em: the maximum across the widest common
# sans families available to the viewers of this dashboard — DejaVu Sans Bold
# and Liberation Sans Bold (Arial metrics) — measured at each font's native
# 2048 upem, plus a 5% margin for the synthesized 600 weight and hinting.
# Bold metrics bound the regular face too, so every emitted text is covered.
# Every other common sans (SF Pro, Segoe UI, Arial, Helvetica) is narrower for
# these characters, so text_width is a safe upper bound: no line emitted by
# wrap_fit can overflow its tile. Characters outside the set default to the
# widest entry.
CHAR_W = {" ": 0.3656, "%": 1.0521, "(": 0.4799, ")": 0.4799, ",": 0.3989,
          "-": 0.4358, ".": 0.3989, "/": 0.3835, "0": 0.7306, "1": 0.7306,
          "2": 0.7306, "3": 0.7306, "4": 0.7306, "5": 0.7306, "6": 0.7306,
          "7": 0.7306, "8": 0.7306, "9": 0.7306, ":": 0.4199, "|": 0.3835,
          "·": 0.3989, "A": 0.8126, "B": 0.8003, "C": 0.7706, "D": 0.8716,
          "F": 0.7173, "G": 0.8618, "H": 0.8788, "I": 0.3907, "K": 0.8136,
          "L": 0.6691, "M": 1.0449, "N": 0.8788, "O": 0.8926, "P": 0.7696,
          "R": 0.8085, "S": 0.7562, "T": 0.7162, "U": 0.8526, "V": 0.8126,
          "W": 1.1582, "X": 0.8095, "Y": 0.7603, "Z": 0.7614,
          "a": 0.7085, "b": 0.7516, "c": 0.6224, "d": 0.7516, "e": 0.7121,
          "f": 0.4568, "g": 0.7516, "h": 0.7475, "i": 0.3599, "k": 0.6983,
          "l": 0.3599, "m": 1.0941, "n": 0.7475, "o": 0.7214, "p": 0.7516,
          "r": 0.5178, "s": 0.625, "t": 0.5019, "u": 0.7475, "v": 0.6844,
          "w": 0.97, "x": 0.6773, "y": 0.6844, "z": 0.6111}
CHAR_W_MAX = 1.16


def text_width(text, font_px):
    return sum(CHAR_W.get(ch, CHAR_W_MAX) for ch in text) * font_px


def wrap_fit(text, max_w, font_px):
    """Greedy word wrap to max_w. Returns None if a single word cannot fit at all."""
    lines, cur = [], ""
    for word in text.split():
        if text_width(word, font_px) > max_w:
            return None
        cand = f"{cur} {word}".strip()
        if cur and text_width(cand, font_px) > max_w:
            lines.append(cur)
            cur = word
        else:
            cur = cand
    if cur:
        lines.append(cur)
    return lines


def tooltip_text(node):
    basis = node.get("basis", "")
    done = node.get("done")
    total = node.get("total")
    frac = f" ({done}/{total})" if done is not None and total else ""
    return (f"{node['name']}\n{node['pct']:.1f}% complete{frac}\n"
            f"effort weight {node['weight']:g} | source: {basis or 'derived'}\n"
            f"{node.get('evidence', '')}")


def fit_tile_label(name, pct_s, rw, rh):
    """Font sizes for a leaf tile's name and percentage so both fit.

    Returns (lines, name_size, pct_size); lines is None for a
    percentage-only tile, and (None, None, None) when even the percentage
    does not fit (the label is then tooltip-only). The uniform bases are an
    11px name and a 12px percentage; a cramped tile first loses percentage
    size, then name size (which also reduces the number of wrapped lines),
    and finally the name entirely. Fit model: the first baseline sits
    4px+size down, lines advance size+2 px, and text occupies 0.756em
    above and 0.2358em below its baseline (the widest common sans metrics,
    the same model the geometry checker validates against)."""
    def width_ok(text, fs):
        return text_width(text, fs) <= rw - 8

    def name_pct_height(n, ns, ps):
        base = 4 + ns + n * (ns + 2)  # baseline of the pct line
        return base + 0.2358 * ps <= rh + 0.5

    for ns in (11, 10, 9, 8, 7):
        lines = wrap_fit(name, rw - 12, ns)
        if lines is None:
            continue
        for ps in (12, 11, 10, 9, 8, 7, 6):
            if 0.756 * ps + 0.2358 * ns > ns + 2:  # pct would overlap the name
                continue
            if width_ok(pct_s, ps) and name_pct_height(len(lines), ns, ps):
                return lines, ns, ps
    for ps in (12, 11, 10, 9, 8, 7, 6):
        if width_ok(pct_s, ps) and 0.496 * ps <= rh / 2 + 0.5:
            return None, None, ps
    return None, None, None


def group_header(node, w, depth):
    """Group header: the full label wrapped at word boundaries.

    The name is wrapped first; "· pct" joins the last line when it fits and
    otherwise gets its own line. Falls back to the percentage alone when the
    name cannot fit; the full label is always in the hover title."""
    font = 13 if depth == 0 else 11
    line_h = 16 if font == 13 else 14
    lines = wrap_fit(node["name"], w - 10, font)
    if lines is None:
        lines = [fmt_pct(node["pct"])]
    suffix = f"  ·  {fmt_pct(node['pct'])}"
    if text_width(lines[-1] + suffix, font) <= w - 10:
        lines[-1] += suffix
    else:
        lines.append(suffix.strip())
    return lines, 4 + line_h * len(lines), font


def render_treemap(groups, width, height, css_class="tm", interactive=False, sort_mode="weight"):
    """Squarified treemap: categories > components > sub-components as nested tiles.

    interactive=True renders for the dashboard page: tiles carry data-id and
    classes, no native <title> (the page shows a custom tooltip), and every
    tile that can hold text gets its label — CSS hides text at the root level
    and reveals it when a tile is zoomed into. interactive=False is the static
    fallback and the standalone progress.svg: a label only when the tile fits
    one, otherwise just the percentage. sort_mode orders siblings at every
    level: weight (area, default) or progress (largest percent first)."""
    parts = []
    leaves_out = []   # (node, rect)
    groups_out = []   # (node, rect, depth, header lines, head height, font)

    def key(n):
        if sort_mode == "progress":
            return (-n["pct"], -n["weight"])
        return -n["weight"]  # stable: equal weights keep document order

    def place(nodes, x, y, w, h, depth):
        ordered = sorted(nodes, key=key)
        rects = squarify([n["weight"] for n in ordered], x, y, w, h)
        for node, (rx, ry, rw, rh) in zip(ordered, rects):
            if "children" in node:
                lines, head, font = group_header(node, rw, depth)
                pad = 3
                groups_out.append((node, rx, ry, rw, rh, depth, lines, head, font))
                if rw > 2 * pad + 24 and rh > head + 2 * pad + 18:
                    place(node["children"], rx + pad, ry + head, rw - 2 * pad, rh - head - 2 * pad, depth + 1)
                else:
                    leaves_out.append((node, rx, ry, rw, rh))
            else:
                leaves_out.append((node, rx, ry, rw, rh))

    place(groups, 0, 0, width, height, 0)

    parts.append(f'<rect class="{css_class}-bg" x="0" y="0" width="{width}" height="{height}"/>')
    for node, x, y, w, h, depth, lines, head, font in groups_out:
        cls = "grp0" if depth == 0 else "grp1"
        if interactive:
            parts.append(f'<g class="tile grp" data-id="{esc(node["id"])}">')
        else:
            parts.append(f'<g><title>{html.escape(tooltip_text(node))}</title>')
        parts.append(f'<rect class="{css_class}-{cls}" x="{x:.1f}" y="{y:.1f}" width="{max(w, 0):.1f}" height="{max(h, 0):.1f}"/>')
        if lines and w > 90 and h > head + 6:
            line_h = 16 if font == 13 else 14
            for i, ln in enumerate(lines):
                baseline = y + 4 + i * line_h + (11 if font == 13 else 9)
                parts.append(f'<text class="grp-head {css_class}-head" x="{x + 6:.1f}" y="{baseline:.1f}" font-size="{font}">{html.escape(ln)}</text>')
        parts.append('</g>')

    for node, x, y, w, h in leaves_out:
        fill = pct_color(node["pct"])
        ink = text_on(fill)
        inset = 1.0
        rx, ry, rw, rh = x + inset, y + inset, max(w - 2 * inset, 0), max(h - 2 * inset, 0)
        if interactive:
            parts.append(f'<g class="tile leaf" data-id="{esc(node["id"])}">')
        else:
            parts.append(f'<g><title>{html.escape(tooltip_text(node))}</title>')
        parts.append(f'<rect x="{rx:.1f}" y="{ry:.1f}" width="{rw:.1f}" height="{rh:.1f}" rx="3" fill="{fill}"/>')
        if interactive:
            # Every tile shows its percentage. The uniform bases are an 11px
            # name and a 12px percentage; a cramped tile first loses
            # percentage size, then name size, then the name itself (the
            # full label stays in the tooltip and detail panel).
            lines, ns, ps = fit_tile_label(node["name"], fmt_pct(node["pct"]), rw, rh)
            if lines is None:
                if ps is not None:
                    parts.append(f'<text class="leaf-pct" x="{rx + 5:.1f}" y="{ry + rh / 2 + 0.26 * ps:.1f}" font-size="{ps}" font-weight="700" fill="{ink}">{fmt_pct(node["pct"])}</text>')
            else:
                for i, ln in enumerate(lines):
                    parts.append(f'<text class="leaf-label" x="{rx + 6:.1f}" y="{ry + 4 + ns + i * (ns + 2):.1f}" font-size="{ns}" fill="{ink}">{html.escape(ln)}</text>')
                parts.append(f'<text class="leaf-pct" x="{rx + 6:.1f}" y="{ry + 4 + ns + len(lines) * (ns + 2):.1f}" font-size="{ps}" font-weight="700" fill="{ink}">{fmt_pct(node["pct"])}</text>')
        else:
            label_drawn = False
            if rw >= 60 and rh >= 40:
                lines = wrap_fit(node["name"], rw - 12, 11)
                # Room for the wrapped lines plus the large percentage beneath them.
                if lines and 15 + 13 * (len(lines) - 1) + 25 <= rh:
                    pct_s = fmt_pct(node["pct"])
                    ty = ry + 15
                    for ln in lines:
                        parts.append(f'<text x="{rx + 6:.1f}" y="{ty:.1f}" font-size="11" fill="{ink}">{html.escape(ln)}</text>')
                        ty += 13
                    parts.append(f'<text x="{rx + 6:.1f}" y="{ry + rh - 7:.1f}" font-size="15" font-weight="700" fill="{ink}">{pct_s}</text>')
                    label_drawn = True
            if not label_drawn and rw >= 36 and rh >= 22:
                # Too small for the name: keep the percentage; the full label is in the hover title.
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


def add_parents(groups):
    for g in groups:
        for node in walk_spec_nodes(g):
            node["parent"] = None if node is g else g["id"]
    return groups


def walk_spec_nodes(node):
    yield node
    for child in node.get("children", []):
        yield from walk_spec_nodes(child)


def interactive_treemap(groups, width, height):
    """Treemap for the dashboard page: one static camera group holding two
    precomputed layers (sorted by weight and by progress); the page's inline
    script swaps layers, zooms the camera and toggles label visibility.
    No layout math in JS."""
    layer_w = render_treemap(groups, width, height, interactive=True)
    layer_p = render_treemap(groups, width, height, interactive=True, sort_mode="progress")
    return (f'<svg id="treemap" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
            f'width="100%" role="img" aria-label="Treemap of port progress: click a tile to zoom in" '
            f'font-family="system-ui,-apple-system,Segoe UI,sans-serif">'
            f'<style>{TREEMAP_CSS}{TM_SVG_CSS}</style>'
            f'<g class="tm-cam" id="tm-cam">'
            f'<g class="layer layer-weight">{layer_w}</g>'
            f'<g class="layer layer-progress">{layer_p}</g>'
            f'</g></svg>')


TM_SVG_CSS = """
#treemap .tile{cursor:pointer;transition:opacity .25s}
#treemap .tile rect{vector-effect:non-scaling-stroke}
#treemap .leaf-label,#treemap .leaf-pct,#treemap .grp-head{transition:opacity .3s}
#treemap.at-root .leaf-label{opacity:0;pointer-events:none}
#treemap.zoomed .tile{opacity:.22}
#treemap.zoomed .tile.keep{opacity:1}
#treemap.zoomed .tile.active,#treemap.zoomed .tile.active .tile{opacity:1}
#treemap .tm-cam{transition:transform .5s cubic-bezier(.4,0,.2,1)}
#treemap.sort-progress .layer-weight{display:none}
#treemap:not(.sort-progress) .layer-progress{display:none}
""".strip()


TM_PAGE_CSS = """
.tm-bar{display:none;align-items:center;gap:10px;flex-wrap:wrap;margin:12px 0 8px}
html.tmjs .tm-bar{display:flex}
.tm-crumbs{display:flex;align-items:center;gap:2px;flex-wrap:wrap;font-size:.9rem}
.tm-crumbs button{background:none;border:0;padding:3px 8px;font:inherit;font-size:.9rem;color:var(--ink);cursor:pointer;border-radius:5px}
.tm-crumbs button:hover{background:var(--card)}
.tm-crumbs button.on{color:var(--accent);font-weight:700}
.tm-crumbs .sep{color:var(--ink2);padding:0 2px}
.tm-bar .spacer{flex:1}
.tm-bar .btn,.sort button{border:1px solid var(--line);background:var(--card);border-radius:5px;padding:3px 10px;font:inherit;font-size:.85rem;cursor:pointer;color:var(--ink)}
.sort{display:flex;align-items:center;gap:5px;font-size:.85rem;color:var(--ink2)}
.sort button.on{background:var(--accent);border-color:var(--accent);color:#fff}
.tm-wrap{position:relative}
.tm-tip{position:absolute;z-index:6;max-width:340px;background:var(--card);border:1px solid var(--line);border-radius:8px;padding:9px 11px;pointer-events:none;box-shadow:0 6px 18px rgba(0,0,0,.16);font-size:.85rem}
.tm-tip .t-name{font-weight:700}
.tm-tip .t-sub{color:var(--ink2);margin-top:2px}
.tm-tip .t-ev{margin-top:7px;color:var(--ink2);font-size:.8rem}
.tm-detail{margin-top:12px;background:var(--card);border-radius:8px;padding:12px 14px}
.tm-detail h3{margin:0 0 8px;font-size:1.05rem}
.d-stats{display:flex;gap:10px;align-items:center;flex-wrap:wrap;font-size:.9rem;margin-bottom:8px}
.d-stats .swatch{width:10px;height:10px;display:inline-block;border-radius:2px}
.d-stats .d-w,.d-stats .d-basis{color:var(--ink2)}
.d-kids{margin-top:6px;display:grid;gap:2px}
.d-kids button{display:flex;justify-content:space-between;gap:10px;align-items:baseline;background:none;border:0;border-radius:5px;padding:4px 8px;cursor:pointer;font:inherit;font-size:.9rem;color:var(--ink);text-align:left}
.d-kids button:hover{background:var(--surface)}
.d-kids .d-k-w{color:var(--ink2);font-size:.8rem}
""".strip()


TM_PAGE_JS = """
(function () {
  "use strict";
  var svg = document.getElementById("treemap");
  var cam = document.getElementById("tm-cam");
  var tip = document.getElementById("tm-tip");
  var detail = document.getElementById("tm-detail");
  var crumbs = document.getElementById("tm-crumbs");
  var wrap = document.getElementById("tm-wrap");
  if (!svg || !cam || !window.TM_DATA) return;
  document.documentElement.classList.add("tmjs");

  var vb = (svg.getAttribute("viewBox") || "").split(/\\s+/);
  var W = +vb[2], H = +vb[3];
  var byId = {};
  (function idx(nodes) {
    nodes.forEach(function (n) { byId[n.id] = n; if (n.children) idx(n.children); });
  })(window.TM_DATA);
  var BINS = ["#a8102a", "#e0573a", "#f2b705", "#3b9a3f", "#006b3c"];
  function colorFor(p) { return BINS[Math.min(4, Math.max(0, Math.floor(p / 20)))]; }
  function fmtPct(p) { return p < 10 ? p.toFixed(1) + "%" : Math.round(p) + "%"; }

  var sortMode = "weight";
  function layerSel() { return sortMode === "weight" ? ".layer-weight" : ".layer-progress"; }
  function tileEl(id) { return svg.querySelector(layerSel() + ' .tile[data-id="' + id + '"]'); }
  function tiles() { return Array.prototype.slice.call(svg.querySelectorAll(layerSel() + " .tile")); }
  function rectOf(t) {
    var r = t.querySelector("rect");
    return { x: +r.getAttribute("x"), y: +r.getAttribute("y"), w: +r.getAttribute("width"), h: +r.getAttribute("height") };
  }
  function pathOf(id) {
    var out = [], n = byId[id];
    while (n) { out.push(n); n = n.parent ? byId[n.parent] : null; }
    return out.reverse();
  }

  var stack = [];
  function clearMarks() {
    tiles().forEach(function (t) { t.classList.remove("active", "keep"); t.style.opacity = ""; });
  }
  function applyCam(t) { cam.style.transform = t; }
  function fit() {
    stack = [];
    svg.classList.remove("zoomed");
    svg.classList.add("at-root");
    clearMarks();
    renderCrumbs();
    applyCam("translate(0px,0px) scale(1)");
  }
  function zoomTo(id) {
    var el = tileEl(id);
    if (!el) return;
    var r = rectOf(el);
    var m = 0.08;
    var s = Math.min(W * (1 - 2 * m) / r.w, H * (1 - 2 * m) / r.h);
    if (!(s > 1.02)) { fit(); return; }
    var tx = W / 2 - s * (r.x + r.w / 2);
    var ty = H / 2 - s * (r.y + r.h / 2);
    stack = pathOf(id);
    svg.classList.add("zoomed");
    svg.classList.remove("at-root");
    clearMarks();
    el.classList.add("active");
    pathOf(id).slice(0, -1).forEach(function (n) {
      var a = tileEl(n.id);
      if (a) a.classList.add("keep");
    });
    renderCrumbs();
    applyCam("translate(" + tx + "px," + ty + "px) scale(" + s + ")");
  }
  function renderCrumbs() {
    crumbs.innerHTML = "";
    var all = document.createElement("button");
    all.textContent = "All";
    all.className = stack.length ? "" : "on";
    all.onclick = fit;
    crumbs.appendChild(all);
    stack.forEach(function (n, i) {
      var sep = document.createElement("span");
      sep.className = "sep";
      sep.textContent = "›";
      crumbs.appendChild(sep);
      var b = document.createElement("button");
      b.textContent = n.name;
      b.className = i === stack.length - 1 ? "on" : "";
      b.onclick = function () { zoomTo(n.id); };
      crumbs.appendChild(b);
    });
  }

  var hot = null;
  svg.addEventListener("pointerover", function (e) {
    var t = e.target.closest ? e.target.closest(".tile") : null;
    if (!t || t === hot) return;
    hot = t;
    showTip(t);
  });
  svg.addEventListener("pointerout", function (e) {
    var t = e.target.closest ? e.target.closest(".tile") : null;
    if (!t || t !== hot) return;
    var to = e.relatedTarget && e.relatedTarget.closest ? e.relatedTarget.closest(".tile") : null;
    if (to === t) return;
    hot = null;
    tip.hidden = true;
  });
  function showTip(t) {
    var n = byId[t.getAttribute("data-id")];
    if (!n) return;
    var frac = n.done != null && n.total ? " (" + n.done + "/" + n.total + ")" : "";
    var sub = fmtPct(n.pct) + " complete" + frac + " · effort weight " + n.weight;
    sub += n.basis ? " · basis " + n.basis : "";
    tip.innerHTML = "";
    var a = document.createElement("div");
    a.className = "t-name";
    a.textContent = n.name;
    var b = document.createElement("div");
    b.className = "t-sub";
    b.textContent = sub;
    tip.appendChild(a);
    tip.appendChild(b);
    if (n.evidence) {
      var c = document.createElement("div");
      c.className = "t-ev";
      c.textContent = n.evidence;
      tip.appendChild(c);
    }
    tip.hidden = false;
  }
  function moveTip(e) {
    if (!hot) return;
    var wr = wrap.getBoundingClientRect();
    var x = e.clientX - wr.left + 14;
    var y = e.clientY - wr.top + 14;
    if (x + tip.offsetWidth > wr.width - 8) x = e.clientX - wr.left - tip.offsetWidth - 10;
    if (y + tip.offsetHeight > wr.height - 8) y = e.clientY - wr.top - tip.offsetHeight - 10;
    tip.style.left = Math.max(4, x) + "px";
    tip.style.top = Math.max(4, y) + "px";
  }
  svg.addEventListener("pointermove", moveTip);

  function showDetail(n) {
    detail.innerHTML = "";
    var h = document.createElement("h3");
    h.textContent = n.name;
    detail.appendChild(h);
    var s = document.createElement("div");
    s.className = "d-stats";
    var sw = document.createElement("span");
    sw.className = "swatch";
    sw.style.background = colorFor(n.pct);
    var strong = document.createElement("strong");
    strong.textContent = fmtPct(n.pct);
    s.appendChild(sw);
    s.appendChild(strong);
    if (n.done != null && n.total) {
      var f = document.createElement("span");
      f.className = "d-w";
      f.textContent = n.done + " of " + n.total;
      s.appendChild(f);
    }
    var w = document.createElement("span");
    w.className = "d-w";
    w.textContent = "effort weight " + n.weight;
    s.appendChild(w);
    if (n.basis) {
      var bb = document.createElement("span");
      bb.className = "d-basis";
      bb.textContent = "basis: " + n.basis;
      s.appendChild(bb);
    }
    detail.appendChild(s);
    if (n.evidence) {
      var p = document.createElement("p");
      p.textContent = n.evidence;
      detail.appendChild(p);
    }
    if (n.rationale) {
      var r = document.createElement("p");
      r.className = "ev";
      r.textContent = n.rationale;
      detail.appendChild(r);
    }
    if (n.children) {
      var kids = document.createElement("div");
      kids.className = "d-kids";
      n.children.forEach(function (c) {
        var btn = document.createElement("button");
        var name = document.createElement("span");
        name.textContent = c.name;
        var pct = document.createElement("span");
        pct.textContent = fmtPct(c.pct);
        var ww = document.createElement("span");
        ww.className = "d-k-w";
        ww.textContent = "weight " + c.weight;
        btn.appendChild(name);
        btn.appendChild(pct);
        btn.appendChild(ww);
        btn.onclick = function () { zoomTo(c.id); showDetail(c); };
        kids.appendChild(btn);
      });
      detail.appendChild(kids);
    }
    detail.hidden = false;
  }

  svg.addEventListener("click", function (e) {
    var t = e.target.closest ? e.target.closest(".tile") : null;
    if (!t) return;
    var id = t.getAttribute("data-id");
    zoomTo(id);
    showDetail(byId[id]);
  });

  Array.prototype.forEach.call(document.querySelectorAll(".sort button"), function (b) {
    b.onclick = function () {
      sortMode = b.getAttribute("data-sort");
      svg.classList.toggle("sort-progress", sortMode === "progress");
      Array.prototype.forEach.call(document.querySelectorAll(".sort button"), function (x) {
        x.classList.toggle("on", x === b);
      });
      hot = null;
      tip.hidden = true;
      fit();
    };
  });

  window.addEventListener("keydown", function (e) {
    if (e.key !== "Escape") return;
    if (stack.length > 1) zoomTo(stack[stack.length - 2].id);
    else fit();
  });

  var fitBtn = document.getElementById("tm-fit");
  if (fitBtn) fitBtn.onclick = fit;

  fit();
})();
"""


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
    svg = interactive_treemap(res["groups"], TREEMAP_W, TREEMAP_H)
    tm_json = json.dumps(add_parents(res["groups"])).replace("</", "<\\/")
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fortnite 12.60 port progress</title>
<style>{PAGE_CSS}{TM_PAGE_CSS}</style></head>
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
<div class="stat"><div class="lbl">Evidence-based (measured + documented)</div><div class="big">{res["measured_pct"]:.1f}%</div></div>
<div class="stat"><div class="lbl">Manual estimates &middot; {res["manual_weight_pct"]:.1f}% of weight</div><div class="big">{res["estimated_pct"]:.1f}%</div></div>
<div class="stat"><div class="lbl">Components measured</div><div class="big">{len(res["leaves"])}</div></div>
<div class="stat"><div class="lbl">Milestones reached</div><div class="big">{res["milestones_done"]} / {res["milestones_total"]}</div></div>
</div>
<p class="meta">The objective lower bound is {res["measured_pct"]:.1f}%: progress measured from committed sources, local outputs or documented counts. The remaining {res["estimated_pct"]:.1f}% of the overall is manual estimates; if all of that work were complete, the overall would be {res["manual_full_pct"]:.1f}%.</p>

<h2>Milestones toward the Definition</h2>
<ul class="ms">{milestones}</ul>

<h2>Treemap</h2>
<p class="meta">Tile area = estimated effort weight. Tile colour = percent complete (red to green, five bins). The percentage shows on every tile at all times; name labels reveal on zoom and are never cut off. Hover for the summary, click to read the evidence below. Breadcrumbs, <em>Fit all</em> or <em>Esc</em> go back; the sort toggle re-lays out the tiles by progress instead of area.</p>
{legend_html()}
<div class="tm-bar" id="tm-bar">
<nav class="tm-crumbs" id="tm-crumbs" aria-label="Zoom level"></nav>
<span class="spacer"></span>
<button class="btn" id="tm-fit" type="button">Fit all</button>
<div class="sort"><span>Sort tiles by</span>
<button data-sort="weight" class="on" type="button">area (weight)</button>
<button data-sort="progress" type="button">progress</button>
</div>
</div>
<div class="tm-wrap" id="tm-wrap">{svg}<div class="tm-tip" id="tm-tip" hidden></div></div>
<div class="tm-detail" id="tm-detail" hidden></div>

<h2>Category summary</h2>
<div class="stats">{cats}</div>

<h2>Components</h2>
<p class="meta">Basis: <em>git</em> = committed sources at HEAD; <em>local</em> = ignored outputs under porting/generated/ read from disk; <em>manual</em> = estimate in components.json; <em>documented</em> = count taken from the porting docs; <em>carried</em> = a history replay that used current local data. Ceilings cap a measured value where the measure overstates progress.</p>
<div class="tablewrap"><table>
<thead><tr><th>Component</th><th>Weight</th><th>Done / total</th><th>Complete</th><th>Basis</th><th>Evidence and rationale</th></tr></thead>
<tbody>{''.join(rows)}</tbody>
</table></div>

<h2>How this is measured</h2>
<p class="meta">Weights and manual percentages are edited in <code>porting/progress/components.json</code>. The generator is <code>porting/progress/progress.py</code>, which reads committed files via <code>git archive</code> at HEAD and ignored local outputs. It records one history point per commit through <code>.githooks/post-commit</code>. Measures only count what is committed; uncommitted stubs and generated artifacts do not add progress unless they are local outputs named in a measure. The dashboard splits the overall into the evidence-based share (basis <em>git</em>, <em>local</em> or <em>documented</em>) and the manual-estimate share, so the size of the subjective part is always visible next to the figure it affects.</p>
<footer>Generated offline by porting/progress/progress.py. No external scripts or fonts are loaded; the interactivity is a small inline script, and without JavaScript the same treemap renders as a static picture.</footer>
<script>window.TM_DATA = {tm_json};</script>
<script>{TM_PAGE_JS}</script>
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
    meas_pts = []
    dots = []
    for i, e in enumerate(entries):
        x, y = xy(i, e["overall_pct"])
        pts.append(f"{x:.1f},{y:.1f}")
        if "measured_pct" in e:
            mx, my = xy(i, e["measured_pct"])
            meas_pts.append(f"{mx:.1f},{my:.1f}")
        tip = esc(f'{e["short"]} {e["date"][:10]}: {e["overall_pct"]:.2f}% | {e["subject"]}')
        dots.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="4" fill="var(--accent)"><title>{tip}</title></circle>')
    line = f'<polyline points="{" ".join(pts)}" fill="none" stroke="var(--accent)" stroke-width="2"/>' if n > 1 else ""
    meas_line = (f'<polyline points="{" ".join(meas_pts)}" fill="none" stroke="var(--ink2)" '
                 f'stroke-width="1.5" stroke-dasharray="5 4"/>' if len(meas_pts) > 1 else "")
    xlabels = ""
    if n:
        first, last = entries[0], entries[-1]
        xlabels = (f'<text x="{pl}" y="{H - 10}" font-size="11" fill="var(--ink2)">{esc(first["short"])}</text>'
                   f'<text x="{W - pr}" y="{H - 10}" text-anchor="end" font-size="11" fill="var(--ink2)">{esc(last["short"])}</text>')
    chart = (f'<svg viewBox="0 0 {W} {H}" width="100%" role="img" aria-label="Overall progress per commit">'
             f'{"".join(grid)}{meas_line}{line}{"".join(dots)}{xlabels}</svg>')

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
<p class="meta">Solid line: overall including manual estimates. Dashed line: evidence-based progress only (basis <em>git</em>, <em>local</em> or <em>documented</em>), i.e. the objective lower bound. Points marked replay used the current components.json manual estimates and current ignored local outputs, so they are indicative. Hook-recorded points measure HEAD at commit time.</p>
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
        "measured_pct": res["measured_pct"],
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
            "measured_pct": head_res["measured_pct"],
            "estimated_pct": head_res["estimated_pct"],
            "manual_weight_pct": head_res["manual_weight_pct"],
            "manual_full_pct": head_res["manual_full_pct"],
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
