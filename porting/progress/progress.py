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
  --quiet suppresses the summary line for scripted callers.
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
# Tracked data the generator reads from each commit (see README: totals and evidence ledger).
TOTALS_REL = "porting/progress/totals.json"      # written by refresh_totals.py
EVIDENCE_REL = "porting/progress/evidence.json"  # written by record_evidence.py
AGE_CACHE = {}

TREEMAP_W, TREEMAP_H = 1000, 640
# Phone layout: a tall, narrow treemap drawn 1:1 at about 360 px wide, shown at 640 px and below.
TREEMAP_NARROW_W, TREEMAP_NARROW_H = 360, 900
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


def parse_json_or_none(text):
    try:
        return json.loads(text)
    except (TypeError, ValueError):
        return None


def commits_between(older, newer):
    """Commits in older..newer when older is an ancestor of newer, else None."""
    anc = subprocess.run(["git", "-C", REPO, "merge-base", "--is-ancestor", older, newer],
                         capture_output=True)
    if anc.returncode != 0:
        return None
    return int(git("rev-list", "--count", f"{older}..{newer}").strip())


def read_rev_file(rev, rel_path):
    proc = subprocess.run(["git", "-C", REPO, "show", f"{rev}:{rel_path}"],
                          capture_output=True, text=True)
    return proc.stdout if proc.returncode == 0 else None


class Ctx:
    """Measurement context: committed snapshot plus ignored local outputs.

    Tracked denominators (totals.json) and the evidence ledger are read from the
    snapshot like any other tracked file. A commit with no totals.json uses HEAD's
    copy (fallback_totals) and says so in the evidence text."""

    def __init__(self, files, replay, census_order, rev=None, fallback_totals=None):
        self.files = files
        self.replay = replay          # True when measuring a historical commit
        self.census_order = census_order
        self.rev = rev                # commit being measured (ledger ages)
        self.fallback_totals = fallback_totals  # HEAD's totals.json entries, or None
        self.used_local = False       # set per leaf
        self.totals_from_head = False # set per leaf
        self._cache = {}
        self._assign = None

    def reset_leaf(self):
        self.used_local = False
        self.totals_from_head = False

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

    # --- tracked denominators and the evidence ledger (committed data)
    def totals_entries(self):
        if "totals" not in self._cache:
            if TOTALS_REL in self.files:
                data = parse_json_or_none(self.files[TOTALS_REL])
                self._cache["totals"] = (data if isinstance(data, dict) else {}, False)
            else:
                self._cache["totals"] = (self.fallback_totals or {}, True)
        return self._cache["totals"]

    def total_value(self, spec):
        """Denominator of a leaf: a literal 'total', or totals.json[total_key][total_sub]."""
        if "total_key" not in spec:
            return int(spec.get("total", 0)), ""
        entries, from_head = self.totals_entries()
        key, sub = spec["total_key"], spec.get("total_sub")
        where = key + (f"[{sub}]" if sub is not None else "")
        ent = entries.get(key)
        value = ent.get("value") if isinstance(ent, dict) else None
        if sub is not None and value is not None:
            value = value.get(sub) if isinstance(value, dict) else None
        if value is None or not isinstance(value, (int, float)):
            return 0, f"totals.json has no {where} (run porting/progress/refresh_totals.py)"
        note = f"total {int(value)} from totals.json {where}"
        if from_head:
            self.totals_from_head = True
            note += ", HEAD copy (no totals.json at this commit)"
        return int(value), note

    def age_of(self, commit):
        """Commits since a ledger entry was recorded, or None when unknown."""
        if not commit or not self.rev:
            return None
        key = (commit, self.rev)
        if key not in AGE_CACHE:
            AGE_CACHE[key] = commits_between(commit, self.rev)
        return AGE_CACHE[key]

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
    total, tnote = ctx.total_value(spec)
    if "done" in spec:
        done = spec["done"]
        ev = f"{done} of {total} ({spec.get('source', 'documented')}; {tnote})"
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
            ev = f"{done} of {total} (local output {os.path.basename(spec['count_file'])}; {tnote})"
    pct = 100.0 * done / total if total else 0.0
    return {"pct": pct, "done": done, "total": total, "evidence": ev}


def m_jsonl_rows(spec, ctx):
    text = ctx.local_text(spec["file"])
    total, tnote = ctx.total_value(spec)
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
    pct = 100.0 * min(done, total) / total if total else 0.0
    return {"pct": pct, "done": min(done, total), "total": total,
            "evidence": f"{done} map header rows parsed of {total} (local census output; {tnote})"}


def m_local_glob(spec, ctx):
    """Files under ignored local roots. Optional per-root filters:
    sources: [{root, suffix, name_regex}] (name_regex is searched in the basename without
    the suffix); distinct: count distinct basenames across all sources, not files."""
    total, tnote = ctx.total_value(spec)
    sources = spec.get("sources") or [{"root": r, "suffix": spec.get("suffix", ".uasset")}
                                      for r in spec["roots"]]
    distinct = bool(spec.get("distinct"))
    count = 0
    stems = set()
    for src in sources:
        base = ctx.local_path(src["root"])
        if not os.path.isdir(base):
            continue
        ctx.used_local = True
        suffix = src.get("suffix", ".uasset")
        rx = src.get("name_regex")
        for _, _, names in os.walk(base):
            for n in names:
                if not n.endswith(suffix):
                    continue
                stem = n[:-len(suffix)]
                if rx and not re.search(rx, stem):
                    continue
                if distinct:
                    stems.add(stem)
                else:
                    count += 1
    if distinct:
        count = len(stems)
    pct = 100.0 * min(count, total) / total if total else 0.0
    what = spec.get("what", "files")
    return {"pct": pct, "done": count, "total": total,
            "evidence": f"{count} local {what} of {total} ({tnote}); ignored outputs under porting/generated or native/Content"}


def tracked_matches(ctx, globs, name_regex=None):
    """Tracked files at this commit matching any glob; name_regex is searched in the file stem."""
    out = []
    for path, _ in ctx.tracked(globs):
        stem = os.path.splitext(os.path.basename(path))[0]
        if name_regex and not re.search(name_regex, stem):
            continue
        out.append(path)
    return sorted(out)


def m_tracked_count(spec, ctx):
    """Done = tracked files matching the globs (and name_regex on the stem), total from totals.json
    or a literal. also_report lists other tracked groups in the evidence text only."""
    globs = spec.get("globs") or [spec["glob"]]
    name_regex = spec.get("name_regex")
    done = len(tracked_matches(ctx, globs, name_regex))
    total, tnote = ctx.total_value(spec)
    desc = "files" if not name_regex else f"files whose name matches /{name_regex}/"
    ev = f"{done} of {total} tracked {desc} ({', '.join(globs)}); {tnote}"
    extra = [f"{rep['label']} {len(tracked_matches(ctx, rep['globs'], rep.get('name_regex')))}"
             for rep in spec.get("also_report", [])]
    if extra:
        ev += "; also tracked, not counted: " + ", ".join(extra)
    if done > total:
        ev += "; numerator above total, capped at 100%"
    pct = 100.0 * min(done, total) / total if total else 0.0
    return {"pct": pct, "done": done, "total": total, "evidence": ev}


def m_tracked_regex(spec, ctx):
    """Done = integer captured by a regex (group, default 1) from one tracked file at this commit."""
    path = spec["file"]
    total, tnote = ctx.total_value(spec)
    text = ctx.files.get(path)
    if text is None:
        done, what = 0, f"{path} is not tracked at this commit"
    else:
        m = re.search(spec["regex"], text, re.M)
        if not m:
            done, what = 0, f"regex not found in {path}"
        else:
            done, what = int(m.group(spec.get("group", 1))), f"captured from {path}"
    pct = 100.0 * min(done, total) / total if total else 0.0
    return {"pct": pct, "done": done, "total": total, "evidence": f"{done} of {total} ({what}; {tnote})"}


def read_ledger(ctx):
    """Latest entry per check id from the tracked evidence ledger (dict or list form)."""
    data = parse_json_or_none(ctx.files.get(EVIDENCE_REL)) or {}
    checks = data.get("checks", {}) if isinstance(data, dict) else {}
    latest = {}
    if isinstance(checks, dict):
        for cid, ent in checks.items():
            if isinstance(ent, dict):
                latest[cid] = ent
    elif isinstance(checks, list):
        for ent in checks:
            if not isinstance(ent, dict) or not ent.get("id"):
                continue
            prev = latest.get(ent["id"])
            if prev is None or str(ent.get("utc", "")) >= str(prev.get("utc", "")):
                latest[ent["id"]] = ent
    return latest


def m_ledger(spec, ctx):
    """Done = listed checks whose latest recorded entry passed. A missing entry is not a pass."""
    ids = spec["checks"]
    latest = read_ledger(ctx)
    passed = 0
    parts = []
    for cid in ids:
        ent = latest.get(cid)
        if ent is None:
            parts.append(f"{cid} missing")
            continue
        ok = ent.get("result") == "pass"
        passed += ok
        age = ctx.age_of(ent.get("commit"))
        age_s = "age unknown" if age is None else f"{age} commit{'' if age == 1 else 's'} ago"
        parts.append(f"{cid} {ent.get('result', '?')} (exit {ent.get('exit_code', '?')}, {age_s})")
    total = len(ids)
    pct = 100.0 * passed / total if total else 0.0
    ev = f"{passed} of {total} checks pass in the evidence ledger: " + "; ".join(parts)
    return {"pct": pct, "done": passed, "total": total, "evidence": ev}


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
    "tracked_count": m_tracked_count,
    "tracked_regex": m_tracked_regex,
    "ledger": m_ledger,
    "manual": m_manual,
}

# Kinds whose numerator is tracked data (replayable per commit). A git-kind leaf has
# no local input unless it names one, so its basis is 'git' alone.
GIT_KINDS = ("checks", "tracked_count", "tracked_regex", "ledger")

BASIS_BY_KIND = {
    "checks": "git",
    "routes": "git+local",
    "census": "git+local",
    "fraction": "local",
    "jsonl_rows": "local",
    "local_glob": "local",
    "tracked_count": "git",
    "tracked_regex": "git",
    "ledger": "git",
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
        if kind in ("routes", "census") or kind in GIT_KINDS:
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
        "rationale": node.get("rationale", ""), "evidence_links": node.get("evidence_links", []),
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


def basis_key(basis):
    """Basis without the replay marker, e.g. 'git+local+carried' -> 'git+local'."""
    return "+".join(p for p in basis.split("+") if p != "carried")


def measure_tree(model, files, replay, rev=None, fallback_totals=None):
    ctx = Ctx(files, replay, census_order_from_spec(model["tree"]), rev=rev,
              fallback_totals=fallback_totals)
    groups = [evaluate(node, ctx) for node in model["tree"]]
    leaves = [leaf for g in groups for leaf in walk_leaves(g)]
    total_w = sum(leaf["weight"] for leaf in leaves)
    overall = sum(leaf["weight"] * leaf["pct"] for leaf in leaves) / total_w if total_w else 0.0
    manual_w = sum(leaf["weight"] for leaf in leaves if leaf["basis"] == "manual")
    manual_c = sum(leaf["weight"] * leaf["pct"] for leaf in leaves if leaf["basis"] == "manual")
    evidence_c = overall * total_w - manual_c if total_w else 0.0
    # Objective: leaves measured from tracked files, ledgers or local outputs only.
    objective_c = sum(leaf["weight"] * leaf["pct"] for leaf in leaves
                      if "manual" not in leaf["basis"] and "documented" not in leaf["basis"])
    by_basis = {}
    for leaf in leaves:
        key = basis_key(leaf["basis"])
        by_basis[key] = by_basis.get(key, 0.0) + leaf["weight"]
    milestones = model.get("milestones", [])
    return {
        "overall_pct": round(overall, 3),
        # Evidence-based (kept name): git, local and documented leaves. Manual estimates excluded.
        "measured_pct": round(evidence_c / total_w, 3) if total_w else 0.0,
        # Objective: the git and local share only (no typed numbers at all).
        "objective_pct": round(objective_c / total_w, 3) if total_w else 0.0,
        "weight_by_basis": {k: round(100.0 * w / total_w, 3) for k, w in sorted(by_basis.items())}
        if total_w else {},
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


# ---------------------------------------------------------------- unit grids
# Leaves whose units can be enumerated from local data render one square per
# unit (done = unit-done blue, not done = unit-todo neutral, both CSS variables
# so they swap with the page theme), grouped by asset type, package or class.
# A tile with more than UNIT_GRID_MAX units, or one whose grid does not fit,
# falls back to the fill-level tile, as does the ~450k-unit package_coverage.
# Units come from ignored local outputs (current state, the same semantics the
# local measures use). Done sets come from the same sources the leaf's measure
# counts, intersected with the unit list, so a grid never shows more done
# units than its tile's numerator attributes.

UNIT_GRID_MAX = 6000
UNIT_PITCH_MAX = 9    # largest square pitch in SVG units (the treemap viewBox is 1000 x 640)
UNIT_PITCH_MIN = 3    # smallest pitch: at 2 the squares would touch and read as one solid block
UNIT_GAP = 1          # gap between squares at every pitch
UNIT_GRID_PAD = 3     # inset of the grid from the tile's sides and bottom

_UNIT_DATA = {}


def _unit_data(key, loader):
    if key not in _UNIT_DATA:
        _UNIT_DATA[key] = loader()
    return _UNIT_DATA[key]


def _local_read(rel):
    path = os.path.join(REPO, rel)
    if not os.path.isfile(path):
        raise OSError(rel)
    with open(path, encoding="utf-8", errors="replace") as fh:
        return fh.read()


def _content_segment(path):
    """First path segment after '/Content/' — the grouping key for asset and map units."""
    i = path.find("/Content/")
    if i < 0:
        return "(root)"
    seg = path[i + len("/Content/"):].split("/", 1)[0]
    return seg or "(root)"


def _load_assets():
    """From generated/assets.json (the 28 indexed paks' file lists): distinct .uasset
    stems with their /Content/ segment, the sound class/mix paths, and pak names."""
    data = json.loads(_local_read("porting/generated/assets.json"))
    uasset = [f for pak in data.get("paks", []) for f in pak.get("files", [])
              if f.endswith(".uasset")]
    stems = {}
    for f in uasset:
        stem = os.path.basename(f)[:-7]
        if stem not in stems:
            stems[stem] = _content_segment(f)
    sound = sorted({f for f in uasset if re.search(r"SoundClass|SoundMix", f)})
    indexed = [p.get("pak") for p in data.get("paks", []) if p.get("pak")]
    skipped = [s.get("pak") for s in data.get("skipped", []) if s.get("pak")]
    return {"stems": stems, "sound": sound, "indexed": indexed, "skipped": skipped}


def _load_dir_stems(rel):
    base = os.path.join(REPO, rel)
    stems = set()
    if os.path.isdir(base):
        for root, _, names in os.walk(base):
            for n in names:
                if n.endswith((".uasset", ".glb")):
                    stems.add(n.rsplit(".", 1)[0])
    return stems


def _load_local_stems():
    """Converted (generated/converted) and native (native/Content) package stems —
    the done signals of the asset-type leaves."""
    return {"local": _load_dir_stems("porting/generated/converted")
            | _load_dir_stems("porting/native/Content"),
            "native": _load_dir_stems("porting/native/Content")}


def _load_inventory_paks():
    data = json.loads(_local_read("porting/generated/inventory.json"))
    return {p.get("name") for p in data.get("paks", []) if p.get("name")}


def _load_map_paths():
    """Map unit lists: every map path in map-scan.jsonl, and the subset that has a
    parsed native-imports row (the map_headers done signal)."""
    def rows(rel):
        out = []
        for line in _local_read(rel).splitlines():
            if not line.startswith("{"):
                continue
            try:
                p = json.loads(line).get("path")
            except ValueError:
                p = None
            if p:
                out.append(p)
        return out
    return rows("porting/generated/map-scan.jsonl"), set(rows("porting/generated/map-native-imports.jsonl"))


def _load_classes():
    """Named classes per package, all block addresses and the named ones —
    from reflection/class_name_map.json over reflection/class_parent_map.json."""
    cnm = json.loads(_local_read("porting/generated/reflection/class_name_map.json"))
    cpm = json.loads(_local_read("porting/generated/reflection/class_parent_map.json"))
    by_pkg = {}
    blocks = []
    named = set()
    for va, meta in cpm.items():
        blocks.append(va)
        name = cnm.get(va)
        if isinstance(name, str) and name:
            named.add(va)
            by_pkg.setdefault((meta or {}).get("package") or "(none)", []).append(name)
    blocks.sort(key=lambda va: int(va, 16))
    return {"blocks": blocks, "named": named, "by_pkg": by_pkg}


def _load_funcs():
    """class::function units with an exec address (the native_behavior denominator)."""
    nf = json.loads(_local_read("porting/generated/reflection/native_funcs.json"))
    units = []
    for cls in sorted(nf):
        for fn, meta in nf[cls].items():
            if meta.get("exec_addr"):
                units.append((cls, fn))
    return units


def _load_manifest_ported():
    data = json.loads(_local_read("porting/behavior/MANIFEST.json"))
    return {(e.get("class"), e.get("function")) for e in data if e.get("status") == "ported"}


def _load_background():
    """The 32 Apollo_Background mesh components, with the 27 matched by (owner, mesh)
    against the converted placements list (the maps.background done signal)."""
    actors = json.loads(_local_read("porting/generated/apollo-background-actors.json"))
    comps = actors.get("staticMeshComponents", [])
    pl = json.loads(_local_read("porting/generated/converted/apollo-background-placements.json"))
    pm = {(p.get("owner"), p.get("mesh")) for p in pl}
    units, done = [], set()
    for c in comps:
        label = f"{c.get('owner', '?')} :: {c.get('name', '?')}"
        units.append(label)
        if (c.get("owner"), c.get("mesh")) in pm:
            done.add(label)
    return units, done


def _load_landscape():
    tiles = json.loads(_local_read("porting/generated/converted/apollo-landscape-grid.json"))
    return sorted(t.get("tile", f"tile{i}") for i, t in enumerate(tiles))


def _grid_done_units(spec, ctx, units):
    """Done units named in the spec's done_units (matched on the unit's stem)."""
    names = spec["measure"].get("done_units", [])
    return {u for u, _ in units if os.path.basename(u).rsplit(".", 1)[0] in names}


def _grid_pak_index(spec, ctx):
    a = _unit_data("assets", _load_assets)
    units = [(n, "indexed") for n in a["indexed"]] + [(n, "skipped") for n in a["skipped"]]
    inv = _unit_data("inventory", _load_inventory_paks)
    return units, {n for n, _ in units if n in inv}


def _grid_map_headers(spec, ctx):
    scan, imp = _unit_data("maps", _load_map_paths)
    units = [(p, _content_segment(p)) for p in scan]
    return units, {p for p, _ in units if p in imp}


def _grid_prefix(spec, ctx, regex, stems_key):
    """Asset-type units: the cooked stems matching the leaf's prefix rule (the same
    naming heuristic the denominator counts), done where the stem is local."""
    a = _unit_data("assets", _load_assets)
    units = [(s, g) for s, g in a["stems"].items() if re.search(regex, s)]
    done = _unit_data("local_stems", _load_local_stems)[stems_key]
    return units, {s for s, _ in units if s in done}


def _grid_sound_classes(spec, ctx):
    a = _unit_data("assets", _load_assets)
    units = [(p, _content_segment(p)) for p in a["sound"]]
    return units, _grid_done_units(spec, ctx, units)


def _grid_enter_play(spec, ctx):
    scan, _imp = _unit_data("maps", _load_map_paths)
    units = [(p, _content_segment(p)) for p in scan]
    return units, _grid_done_units(spec, ctx, units)


def _grid_landscape(spec, ctx):
    tiles = _unit_data("landscape", _load_landscape)
    return [(t, "tiles") for t in tiles], set(tiles)


def _grid_background(spec, ctx):
    units, done = _unit_data("background", _load_background)
    return [(u, "actors") for u in units], done


def _grid_reflection_recovery(spec, ctx):
    c = _unit_data("classes", _load_classes)
    units = [(va, "blocks") for va in c["blocks"]]
    return units, {va for va in c["blocks"] if va in c["named"]}


def _grid_native_behavior(spec, ctx):
    units = _unit_data("funcs", _load_funcs)
    done = _unit_data("manifest", _load_manifest_ported)
    return [(f"{c}::{fn}", c) for c, fn in units], {f"{c}::{fn}" for c, fn in done}


def _grid_census(spec, ctx):
    members = sorted(n for n, owner in ctx.census_assignment().items() if owner == spec["id"])
    declared = ctx.declared_natives()
    return [(n, "classes") for n in members], {n for n in members if is_declared(n, declared)}


def _grid_event_registrations(spec, ctx):
    inventory = parse_json_or_none(ctx.files.get("porting/progress/frontend_event_targets.json")) or {}
    targets = {row["check_id"]: row for row in inventory.get("targets", [])}
    ledger = read_ledger(ctx)
    ids = spec["measure"]["checks"]
    if any(cid not in targets for cid in ids):
        return [], set()
    return ([(targets[cid]["path"], targets[cid]["cpp_class"]) for cid in ids],
            {targets[cid]["path"] for cid in ids if ledger.get(cid, {}).get("result") == "pass"})


CLASS_GRID_PACKAGES = {
    "gameplay.reflection_exact": ["/Script/FortniteGame"],
    "gameplay.weapons": ["/Script/FortniteGame"],
    "gameplay.storm": ["/Script/FortniteGame"],
    "gameplay.battle_bus": ["/Script/FortniteGame"],
    "ui.commonui_umg": ["/Script/CommonUI", "/Script/UMG"],
    "ui.fortniteui_classes": ["/Script/FortniteUI"],
}
CLASS_GRID_GLOBS = {
    "gameplay.reflection_exact": ["porting/native/Source/FortniteGame/Generated/[AU]*.h"],
    "gameplay.weapons": ["porting/native/Source/FortniteGame/Generated/[AU]*.h"],
    "gameplay.storm": ["porting/native/Source/FortniteGame/Generated/[AU]*.h"],
    "gameplay.battle_bus": ["porting/native/Source/FortniteGame/Generated/[AU]*.h"],
    "ui.commonui_umg": ["porting/native/Source/CommonUI/Generated/[AU]*.h",
                        "porting/native/Source/UMG/Generated/[AU]*.h"],
    "ui.fortniteui_classes": ["porting/native/Source/FortniteUI/Generated/[AU]*.h"],
}


def _grid_class_headers(spec, ctx):
    """Class units: the named classes of the leaf's package(s) (and name_regex), done
    where a tracked generated header carries the class name (the same signal the
    leaf's tracked_count measure counts)."""
    c = _unit_data("classes", _load_classes)
    names = set()
    for pkg in CLASS_GRID_PACKAGES[spec["id"]]:
        names.update(c["by_pkg"].get(pkg, []))
    rx = spec["measure"].get("name_regex")
    if rx:
        names = {n for n in names if re.search(rx, n)}
    units = [(n, "classes") for n in sorted(names)]
    done = {os.path.splitext(os.path.basename(p))[0]
            for p in tracked_matches(ctx, CLASS_GRID_GLOBS[spec["id"]], rx)}
    return units, {u for u, _ in units if u in done}


UNIT_PROVIDERS = {
    "assets.pak_index": _grid_pak_index,
    "assets.map_headers": _grid_map_headers,
    "assets.static_meshes": lambda spec, ctx: _grid_prefix(spec, ctx, r"^SM_", "local"),
    "assets.skeletal_anim": lambda spec, ctx: _grid_prefix(spec, ctx, r"^SK_", "native"),
    "visuals.vfx": lambda spec, ctx: _grid_prefix(spec, ctx, r"^NS_", "local"),
    "assets.reflection_recovery": _grid_reflection_recovery,
    "maps.enter_play": _grid_enter_play,
    "maps.apollo_terrain": _grid_landscape,
    "maps.background": _grid_background,
    "gameplay.native_behavior": _grid_native_behavior,
    "gameplay.framework": _grid_census,
    "gameplay.building": _grid_census,
    "gameplay.inventory": _grid_census,
    "gameplay.abilities": _grid_census,
    "gameplay.ai_nav": _grid_census,
    "gameplay.world_volumes": _grid_census,
    "gameplay.other": _grid_census,
    "gameplay.reflection_exact": _grid_class_headers,
    "gameplay.weapons": _grid_class_headers,
    "gameplay.storm": _grid_class_headers,
    "gameplay.battle_bus": _grid_class_headers,
    "ui.commonui_umg": _grid_class_headers,
    "ui.fortniteui_classes": _grid_class_headers,
    "ui.frontend_event_registrations": _grid_event_registrations,
    "audio.sound_classes": _grid_sound_classes,
}


def build_unit_grids(model, files, leaves):
    """Per-unit cell lists (group, label, done) for the decomposable leaves, keyed by
    leaf id. Missing local data leaves a plain tile. Units are sorted by group, done
    first within each group (so a group's colour is one clean run, not scattered
    specks), so groups are contiguous in the grid.

    leaves maps leaf id to the measured leaf (done, total). A grid is kept only when
    its done and total equal the leaf's own numerator and denominator; otherwise the
    leaf gets an empty list, which renders the progress bar driven by the leaf pct.
    So a grid never contradicts its tile's percentage."""
    ctx = Ctx(files, False, census_order_from_spec(model["tree"]))
    grids = {}
    for spec in walk_spec(model["tree"]):
        provider = UNIT_PROVIDERS.get(spec["id"])
        if provider is None:
            continue
        try:
            units, done = provider(spec, ctx)
        except (OSError, ValueError):
            continue
        if not units:
            continue
        cells = [(g, u, u in done)
                 for u, g in sorted(units, key=lambda t: (t[1], t[0] not in done, t[0]))]
        lf = leaves.get(spec["id"], {})
        if (lf.get("done"), lf.get("total")) != (sum(1 for c in cells if c[2]), len(cells)):
            grids[spec["id"]] = []
            continue
        grids[spec["id"]] = cells
    return grids


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


GRP_INSET = 1      # half the 2px gutter between siblings; groups and leaves are inset alike
GRP_PAD = 3        # inner pad between a group's frame and its children (top: below the group label)
GRP_RADIUS = 4     # corner radius of every tile, group or leaf
GRP_FONT = 12      # group label size; the label is plain text on the surface, with no header strip
LEAF_PAD = 5       # text inset from a leaf's left edge
LEAF_NAME_SIZES = (11, 10)
LEAF_PCT_SIZES = (12, 11)
LEAF_MAX_LINES = 4
LEAF_MAX_LINES_SMALL = 7  # 10px names may wrap further so a tall narrow tile keeps its name


def group_header_height(n_lines):
    """Label band above a group's children: 4px, GRP_FONT + 2 per line, 2px."""
    return 4 + n_lines * (GRP_FONT + 2) + 2


def group_header(node, w):
    """Group label as one line where possible.

    Returns (lines, label band height), where lines is a list of (name, suffix)
    pairs drawn as name plus a lighter "· pct" suffix. Order of fallbacks: the
    full name on one line; the name without a trailing parenthetical (for
    example " (/Script/FortniteGame)") on one line; the shortened name wrapped to
    at most 2 lines; the percentage alone; nothing when even that cannot fit.
    The full name always stays in the hover title and detail panel."""
    font = GRP_FONT
    avail = w - 12
    pct_s = fmt_pct(node["pct"])
    suffix = f" · {pct_s}"
    full = node["name"]
    short = re.sub(r"\s*\([^()]*\)\s*$", "", full) or full
    if text_width(full + suffix, font) <= avail:
        lines = [(full, suffix)]
    elif text_width(short + suffix, font) <= avail:
        lines = [(short, suffix)]
    else:
        # Wrap the name and "·pct" as words, so the percentage moves with the last word.
        wrapped = wrap_fit(f"{short} ·{pct_s}", avail, font)
        if wrapped and len(wrapped) <= 2:
            lines = []
            for ln in wrapped:
                idx = ln.rfind("·")
                if idx < 0:
                    lines.append((ln, ""))
                else:
                    name_part = ln[:idx].rstrip()
                    lead = " " if name_part else ""
                    lines.append((name_part, f"{lead}· {ln[idx + 1:].lstrip()}"))
        elif text_width(f"· {pct_s}", font) <= avail:
            lines = [("", f"· {pct_s}")]
        else:
            lines = []
    return lines, group_header_height(max(len(lines), 1))


def unit_grid_layout(group_sizes, rw, rh):
    """Square pitch for a unit grid: the largest integer pitch s, 2 <= s <= UNIT_PITCH_MAX,
    at which every unit fits in rw x rh. Units fill rows left to right, groups are contiguous,
    and a gap row is inserted between two groups only when the second group starts
    at a row boundary (the same rule render_unit_grid emits). A square is s - gap
    wide, with gap 1 when s >= 4 and 0 otherwise, so cols*s - gap <= rw and
    rows*s - gap <= rh. Returns (s, cols, rows) or None when no pitch >= 2 fits or
    the tile holds more than UNIT_GRID_MAX units."""
    n = sum(group_sizes)
    if n == 0 or n > UNIT_GRID_MAX:
        return None

    def rows_for(c):
        rows = 0
        row = 0
        for i, s in enumerate(group_sizes):
            if i > 0 and row == 0:
                rows += 1
            left = s
            while left > 0:
                take = min(c - row, left)
                row += take
                left -= take
                if row == c:
                    rows += 1
                    row = 0
        if row > 0:
            rows += 1
        return rows

    for s in range(min(UNIT_PITCH_MAX, int(min(rw, rh))), UNIT_PITCH_MIN - 1, -1):
        gap = UNIT_GAP
        cols = min(int((rw + gap) // s), n)
        if cols < 1:
            continue
        rows = rows_for(cols)
        if cols * s - gap <= rw and rows * s - gap <= rh:
            return s, cols, rows
    return None


def group_sizes(cells):
    """Run lengths of consecutive units sharing a group (cells are group-contiguous)."""
    sizes = []
    for g, _label, _done in cells:
        if not sizes or sizes[-1][0] != g:
            sizes.append([g, 0])
        sizes[-1][1] += 1
    return [n for _g, n in sizes]


def render_unit_grid(cells, layout, x0, y0):
    """One square per unit at pitch s from unit_grid_layout, positioned relative to
    (x0, y0). Done squares share one path (class unit-done) and not-done squares one
    path (class unit-todo), so the SVG holds two elements per tile, not one per unit.
    A gap row separates groups."""
    s, cols, _rows = layout
    gap = UNIT_GAP
    side = s - gap
    half = gap / 2
    rows_out = []
    row = []
    prev_g = None
    for g, _label, done in cells:
        if g != prev_g:
            if not row and rows_out:
                rows_out.append(None)  # gap row between groups
            prev_g = g
        row.append(done)
        if len(row) == cols:
            rows_out.append(row)
            row = []
    if row:
        rows_out.append(row)
    done_d = []
    todo_d = []
    for ri, r in enumerate(rows_out):
        if r is None:
            continue
        for ci, done in enumerate(r):
            d = f"M{ci * s + half:g} {ri * s + half:g}h{side}v{side}h-{side}z"
            (done_d if done else todo_d).append(d)
    tr = f' transform="translate({x0:.2f} {y0:.2f})"'
    out = []
    if done_d:
        out.append(f'<path class="unit-done"{tr} d="{"".join(done_d)}"/>')
    if todo_d:
        out.append(f'<path class="unit-todo"{tr} d="{"".join(todo_d)}"/>')
    return "\n".join(out)


def label_strip(lab):
    """Height of a top-aligned label block (with a 3px gap below it), 0 when none."""
    lines, ns, ps = lab
    if ps is None:
        return 0.0
    if lines:
        return 4 + ns + len(lines) * (ns + 2) + 0.2358 * ps + 3
    return 4 + ps + 0.2358 * ps + 3


def unit_label(name, pct_s, rw, rh):
    """Label for a unit tile, top-aligned so it can sit above a grid: the same rules as
    a leaf label (leaf_label), so names are at least 10px. A tile that cannot hold the
    name at 10px gets its percentage alone and the rest goes to the grid or bar."""
    return leaf_label(name, pct_s, rw, rh)


def unit_tile_svg(node, cells, rx, ry, rw, rh):
    """Squares and label for a unit tile, in paint order (squares first, so text is never
    under a square). The label sits in a strip at the top and the grid takes the area
    beneath it, over the tile's fill level. Returns None when the tile has no grid or
    its grid does not fit; the caller still draws the fill level and label."""
    if not cells or len(cells) > UNIT_GRID_MAX:
        return None
    lab = unit_label(node["name"], fmt_pct(node["pct"]), rw, rh)
    strip = label_strip(lab)
    gx, gy = rx + UNIT_GRID_PAD, ry + strip
    gw, gh = rw - 2 * UNIT_GRID_PAD, rh - strip - UNIT_GRID_PAD
    layout = unit_grid_layout(group_sizes(cells), gw, gh) if gw >= 2 and gh >= 2 else None
    if layout is None:
        return None
    return [render_unit_grid(cells, layout, gx, gy)] + label_svg(lab, rx, ry, node["pct"])


def label_svg(lab, rx, ry, pct):
    """Name lines and the bold percentage at a tile's top-left, as SVG text. The colour is
    the palette ink, which reads on both the neutral surface and the fill."""
    lines, ns, ps = lab
    if ps is None:
        return []
    text = []
    if lines:
        for i, ln in enumerate(lines):
            text.append(f'<text class="leaf-label" x="{rx + LEAF_PAD:.1f}" y="{ry + 4 + ns + i * (ns + 2):.1f}" font-size="{ns}">{html.escape(ln)}</text>')
        py = ry + 4 + ns + len(lines) * (ns + 2)
    else:
        py = ry + 4 + ps
    text.append(f'<text class="leaf-pct" x="{rx + LEAF_PAD:.1f}" y="{py:.1f}" font-size="{ps}">{fmt_pct(pct)}</text>')
    return text


def fill_svg(rx, ry, rw, rh, pct, cid):
    """Fill level of a plain tile: the done region rises from the bottom edge to pct of the
    tile's height, clipped to the tile's rounded rect (clipPath id cid). A partial fill has
    a 2px edge line along its top; 0% draws nothing and 100% covers the whole tile."""
    if pct <= 0:
        return []
    fh = rh * min(pct, 100.0) / 100.0
    fy = ry + rh - fh
    out = [f'<clipPath id="{cid}"><rect x="{rx:.2f}" y="{ry:.2f}" width="{rw:.2f}" height="{rh:.2f}" rx="{GRP_RADIUS}"/></clipPath>',
           f'<g clip-path="url(#{cid})"><rect class="tile-fill" x="{rx:.2f}" y="{fy:.2f}" width="{rw:.2f}" height="{fh:.2f}"/>']
    if pct < 100:
        out.append(f'<rect class="tile-edge" x="{rx:.2f}" y="{fy - 1:.2f}" width="{rw:.2f}" height="2"/>')
    out.append('</g>')
    return out


def leaf_label(name, pct_s, rw, rh):
    """Label for a plain leaf tile: (lines, name size, pct size).

    The name is 11px, or 10px when 11px needs more than LEAF_MAX_LINES lines (10px may use up to
    LEAF_MAX_LINES_SMALL).
    The percentage is 12px, or 11px when 12px is too wide. Lines advance size+2
    and the first baseline sits 4px+size down. lines is None for a percentage-only
    label; (None, None, None) when even the percentage does not fit, and the name
    then appears only in the tooltip. Names are never truncated."""
    avail = rw - 2 * LEAF_PAD
    bottom = rh - 2
    for ns in LEAF_NAME_SIZES:
        lines = wrap_fit(name, avail, ns)
        limit = LEAF_MAX_LINES if ns > min(LEAF_NAME_SIZES) else LEAF_MAX_LINES_SMALL
        if not lines or len(lines) > limit:
            continue
        for ps in LEAF_PCT_SIZES:
            if text_width(pct_s, ps) > avail:
                continue
            pct_base = 4 + ns + len(lines) * (ns + 2)
            if pct_base + 0.2358 * ps <= bottom:
                return lines, ns, ps
    for ps in LEAF_PCT_SIZES:
        if text_width(pct_s, ps) <= avail and 4 + ps + 0.2358 * ps <= bottom:
            return None, None, ps
    return None, None, None


def render_treemap(groups, width, height, css_class="tm", interactive=False, sort_mode="weight", grids=None, prefix=""):
    """Squarified treemap: categories > components > sub-components as nested tiles.

    interactive=True renders for the dashboard page: tiles carry data-id and
    classes, no native <title> (the page shows a custom tooltip), and every
    tile that can hold text gets its label, visible at all times.
    interactive=False is the static fallback and the standalone progress.svg:
    the same labels as the page, with a native hover title instead of the page tooltip.
    sort_mode orders siblings at every level: weight (area, default) or
    progress (largest percent first). grids maps leaf ids to per-unit cell
    lists; such a tile shows one square per unit below a label strip (see
    unit_tile_svg). Every leaf has a blue fill rising from the bottom edge to
    its percent complete (see fill_svg), including those with squares. prefix keeps
    clipPath ids unique when two layers share one document."""
    parts = []
    leaves_out = []   # (node, x, y, w, h)
    groups_out = []   # (node, x, y, w, h, depth, label lines)

    def key(n):
        if sort_mode == "progress":
            return (-n["pct"], -n["weight"])
        return -n["weight"]  # stable: equal weights keep document order

    def place(nodes, x, y, w, h, depth):
        ordered = sorted(nodes, key=key)
        rects = squarify([n["weight"] for n in ordered], x, y, w, h)
        for node, (rx, ry, rw, rh) in zip(ordered, rects):
            # Every tile, group or leaf, is inset by the same amount so sibling gutters are 2px.
            x0, y0 = rx + GRP_INSET, ry + GRP_INSET
            bw, bh = max(rw - 2 * GRP_INSET, 0), max(rh - 2 * GRP_INSET, 0)
            if "children" in node:
                lines, head = group_header(node, bw)
                groups_out.append((node, x0, y0, bw, bh, depth, lines))
                cw, ch = bw - 2 * GRP_PAD, bh - head - 2 * GRP_PAD
                if cw > 24 and ch > 18:
                    place(node["children"], x0 + GRP_PAD, y0 + head + GRP_PAD, cw, ch, depth + 1)
                else:
                    leaves_out.append((node, x0, y0, bw, bh))
            else:
                leaves_out.append((node, x0, y0, bw, bh))

    place(groups, 0, 0, width, height, 0)

    parts.append(f'<rect class="{css_class}-bg" x="0" y="0" width="{width}" height="{height}"/>')
    for node, x, y, w, h, depth, lines in groups_out:
        cls = "grp0" if depth == 0 else "grp1"
        if interactive:
            parts.append(f'<g class="tile grp" data-id="{esc(node["id"])}">')
        else:
            parts.append(f'<g><title>{html.escape(tooltip_text(node))}</title>')
        # Frame only: no header strip. Top-level groups carry a faint hairline (CSS).
        parts.append(f'<rect class="tile-frame {css_class}-{cls}" x="{x:.1f}" y="{y:.1f}" '
                     f'width="{max(w, 0):.1f}" height="{max(h, 0):.1f}" rx="{GRP_RADIUS}"/>')
        if lines and w > 24:
            for i, (name, sfx) in enumerate(lines):
                baseline = y + 4 + GRP_FONT + i * (GRP_FONT + 2)
                tail = f'<tspan class="grp-pct">{html.escape(sfx)}</tspan>' if sfx else ""
                parts.append(f'<text class="grp-head" x="{x + 6:.1f}" y="{baseline:.1f}" '
                             f'font-size="{GRP_FONT}">{html.escape(name)}{tail}</text>')
        parts.append('</g>')

    for k, (node, rx, ry, rw, rh) in enumerate(leaves_out):
        if interactive:
            parts.append(f'<g class="tile leaf" data-id="{esc(node["id"])}">')
        else:
            parts.append(f'<g><title>{html.escape(tooltip_text(node))}</title>')
        parts.append(f'<rect class="tile-body" x="{rx:.1f}" y="{ry:.1f}" width="{rw:.1f}" height="{rh:.1f}" rx="{GRP_RADIUS}"/>')
        parts.extend(fill_svg(rx, ry, rw, rh, node["pct"], f"{prefix}{css_class}-c{k}"))
        shapes = unit_tile_svg(node, (grids or {}).get(node["id"]), rx, ry, rw, rh)
        if shapes is None:
            shapes = label_svg(leaf_label(node["name"], fmt_pct(node["pct"]), rw, rh), rx, ry, node["pct"])
        parts.extend(shapes)
        # Frame last, so the hover and selection outline paints over the fill.
        parts.append(f'<rect class="tile-frame" x="{rx:.1f}" y="{ry:.1f}" width="{rw:.1f}" height="{rh:.1f}" rx="{GRP_RADIUS}"/>')
        parts.append('</g>')

    return "\n".join(parts)


TREEMAP_PALETTE_CSS = """
:root{--tm-surface:#fcfcfb;--tm-gutter:var(--tm-surface);--tm-neutral:#ecebe8;--tm-done-fill:#b7d3f6;--tm-done-edge:#2a78d6;--tm-unit-done:#2a78d6;--tm-unit-todo:#d6d5d0;--tm-ink:#0b0b0b;--tm-ink-2:#52514e;--tm-group-label:#52514e}
@media (prefers-color-scheme: dark){
  :root:not([data-theme="light"]){--tm-surface:#1a1a19;--tm-gutter:var(--tm-surface);--tm-neutral:#2b2b29;--tm-done-fill:#184f95;--tm-done-edge:#3987e5;--tm-unit-done:#3987e5;--tm-unit-todo:#3d3d3a;--tm-ink:#f0efec;--tm-ink-2:#c3c2b7;--tm-group-label:#c3c2b7}
}
:root[data-theme="dark"]{--tm-surface:#1a1a19;--tm-gutter:var(--tm-surface);--tm-neutral:#2b2b29;--tm-done-fill:#184f95;--tm-done-edge:#3987e5;--tm-unit-done:#3987e5;--tm-unit-todo:#3d3d3a;--tm-ink:#f0efec;--tm-ink-2:#c3c2b7;--tm-group-label:#c3c2b7}
""".strip()


TREEMAP_CSS = TREEMAP_PALETTE_CSS + """
.tm-bg{fill:var(--tm-gutter)}
.tile-body{fill:var(--tm-neutral)}
.tile-fill{fill:var(--tm-done-fill)}
.tile-edge{fill:var(--tm-done-edge)}
.tile-frame{fill:none;stroke:none}
.tm-grp0{fill:none;stroke:var(--tm-neutral);stroke-width:1}
.tm-grp1{fill:none;stroke:none}
.unit-done{fill:var(--tm-unit-done)}
.unit-todo{fill:var(--tm-unit-todo)}
.leaf-label,.leaf-pct,.tm-head{fill:var(--tm-ink);font-family:system-ui,-apple-system,Segoe UI,sans-serif}
.leaf-pct{font-weight:700}
.grp-head{fill:var(--tm-group-label);font-weight:600;font-family:system-ui,-apple-system,Segoe UI,sans-serif}
.grp-pct{fill:var(--tm-group-label);font-weight:400;opacity:.8}
""".strip()


def interactive_treemap(groups, width, height, grids=None, prefix="", svg_class="tm-svg"):
    """Treemap for the dashboard page: two precomputed layers (sorted by weight
    and by progress); the page's inline script swaps the visible layer.
    No layout math in JS. prefix keeps clipPath ids unique across the two sizes."""
    layer_w = render_treemap(groups, width, height, interactive=True, grids=grids, prefix=prefix + "w")
    layer_p = render_treemap(groups, width, height, interactive=True, sort_mode="progress", grids=grids, prefix=prefix + "p")
    return (f'<svg class="{svg_class}" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
            f'width="100%" role="img" aria-label="Treemap of port progress: tap or click a tile to read its details" '
            f'font-family="system-ui,-apple-system,Segoe UI,sans-serif">'
            f'<g class="layer layer-weight">{layer_w}</g>'
            f'<g class="layer layer-progress">{layer_p}</g>'
            f'</svg>')


TM_SVG_CSS = """
.tm-wrap .tile{cursor:pointer}
.tm-wrap .tile rect{vector-effect:non-scaling-stroke}
.tm-wrap .tile.grp>.tile-frame{pointer-events:all}
.tm-wrap .tile:hover>.tile-frame{stroke:var(--tm-ink);stroke-width:2}
.tm-wrap .tile.sel>.tile-frame{stroke:var(--tm-done-edge);stroke-width:2}
.tm-wrap.sort-progress .layer-weight{display:none}
.tm-wrap:not(.sort-progress) .layer-progress{display:none}
""".strip()


TM_PAGE_CSS = """
.tm-bar{display:none;align-items:center;gap:10px;flex-wrap:wrap;margin:12px 0 8px}
html.tmjs .tm-bar{display:flex}
.tm-bar .sort{margin-left:auto;display:flex;align-items:center;gap:5px;font-size:.85rem;color:var(--ink2)}
.sort button{border:1px solid var(--line);background:var(--card);border-radius:5px;padding:3px 10px;font:inherit;font-size:.85rem;cursor:pointer;color:var(--ink)}
.sort button.on{background:var(--accent);border-color:var(--accent);color:#fff}
.tm-wrap{position:relative}
.tm-tip{position:absolute;z-index:6;max-width:340px;background:var(--card);border:1px solid var(--line);border-radius:8px;padding:9px 11px;pointer-events:none;box-shadow:0 6px 18px rgba(0,0,0,.16);font-size:.85rem}
.tm-tip .t-name{font-weight:700}
.tm-tip .t-sub{color:var(--ink2);margin-top:2px}
.tm-tip .t-ev{margin-top:7px;color:var(--ink2);font-size:.8rem}
.tm-detail{margin-top:12px;background:var(--card);border-radius:8px;padding:12px 14px}
.tm-detail h3{margin:0 0 8px;font-size:1.05rem}
.d-stats{display:flex;gap:10px;align-items:center;flex-wrap:wrap;font-size:.9rem;margin-bottom:8px}
.d-stats .d-w,.d-stats .d-basis{color:var(--ink2)}
.d-kids{margin-top:6px;display:grid;gap:2px}
.d-kids button{display:flex;justify-content:space-between;gap:10px;align-items:baseline;background:none;border:0;border-radius:5px;padding:4px 8px;cursor:pointer;font:inherit;font-size:.9rem;color:var(--ink);text-align:left}
.d-kids button:hover{background:var(--surface)}
.d-kids .d-k-w{color:var(--ink2);font-size:.8rem}
""".strip()


TM_PAGE_JS = """
(function () {
  "use strict";
  var svg = document.getElementById("tm-wrap");
  var tip = document.getElementById("tm-tip");
  var detail = document.getElementById("tm-detail");
  var wrap = document.getElementById("tm-wrap");
  if (!svg || !window.TM_DATA) return;
  document.documentElement.classList.add("tmjs");

  var byId = {};
  (function idx(nodes) {
    nodes.forEach(function (n) { byId[n.id] = n; if (n.children) idx(n.children); });
  })(window.TM_DATA);
  function fmtPct(p) { return p < 10 ? p.toFixed(1) + "%" : Math.round(p) + "%"; }

  var sortMode = "weight";

  var hot = null;
  // Touch has no hover: a tap shows the tooltip and it stays until the next tap.
  var touchTip = false;
  svg.addEventListener("pointerdown", function (e) {
    if (e.pointerType === "mouse") return;
    touchTip = true;
    var t = e.target.closest ? e.target.closest(".tile") : null;
    if (!t) { hot = null; tip.hidden = true; return; }
    hot = t;
    showTip(t);
    moveTip(e);
  });
  document.addEventListener("pointerdown", function (e) {
    if (touchTip && !wrap.contains(e.target)) { hot = null; tip.hidden = true; }
  });
  svg.addEventListener("pointerover", function (e) {
    var t = e.target.closest ? e.target.closest(".tile") : null;
    if (!t || t === hot) return;
    hot = t;
    showTip(t);
  });
  svg.addEventListener("pointerout", function (e) {
    if (touchTip) return;
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
    sw.className = "mini";
    sw.style.setProperty("--pct", n.pct + "%");
    sw.appendChild(document.createElement("i"));
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
    if (n.evidence_links && n.evidence_links.length) {
      var links = document.createElement("p");
      n.evidence_links.forEach(function (link) {
        var a = document.createElement("a");
        a.href = link.href;
        a.textContent = link.label;
        links.appendChild(a);
        links.appendChild(document.createTextNode(" "));
      });
      detail.appendChild(links);
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
        btn.onclick = function () { showDetail(c); };
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
    markSelected(id);
    showDetail(byId[id]);
  });
  // Mark the clicked tile in both layers so the outline survives a sort toggle.
  function markSelected(id) {
    Array.prototype.forEach.call(svg.querySelectorAll(".tile"), function (el) {
      el.classList.toggle("sel", el.getAttribute("data-id") === id);
    });
  }

  Array.prototype.forEach.call(document.querySelectorAll(".sort button"), function (b) {
    b.onclick = function () {
      sortMode = b.getAttribute("data-sort");
      svg.classList.toggle("sort-progress", sortMode === "progress");
      Array.prototype.forEach.call(document.querySelectorAll(".sort button"), function (x) {
        x.classList.toggle("on", x === b);
      });
      hot = null;
      tip.hidden = true;
    };
  });
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


def legend_html(weight_by_basis=None):
    """Key for the treemap: an empty-to-full scale (a tile's fill level is its percent
    complete), then the basis shares."""
    out = ('<div class="legend"><span>empty = 0%</span><span class="fillscale" aria-hidden="true"></span>'
           '<span>full = 100%</span><span>A tile&rsquo;s blue fill rises from its bottom edge to its percent '
           'complete. On tiles with squares, each blue square is one finished unit.</span></div>')
    return out + basis_legend_html(weight_by_basis or {})


BASIS_ORDER = ["git", "git+local", "local", "documented", "manual"]


def basis_legend_html(weight_by_basis):
    """Share of the total effort weight per basis, under the treemap legend."""
    if not weight_by_basis:
        return ""
    keys = [k for k in BASIS_ORDER if k in weight_by_basis]
    keys += sorted(k for k in weight_by_basis if k not in BASIS_ORDER)
    cells = " &middot; ".join(f"<strong>{esc(k)}</strong> {weight_by_basis[k]:.1f}%" for k in keys)
    return (f'<p class="meta basis-share">Weight share by basis (effort weight, not progress): {cells}. '
            f'<em>git</em> and <em>local</em> are objective; <em>documented</em> is typed; <em>manual</em> is an estimate.</p>')


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
.legend{display:flex;flex-wrap:wrap;align-items:center;gap:6px 10px;margin:8px 0;font-size:.9rem;color:var(--ink2)}
.fillscale{display:inline-block;width:120px;height:12px;border-radius:4px;border:1px solid var(--line);background:linear-gradient(to right,var(--tm-neutral),var(--tm-done-fill))}
.mini{display:inline-block;width:48px;height:8px;border-radius:4px;background:var(--tm-neutral);overflow:hidden;vertical-align:middle;margin-right:6px}
.mini i{display:block;height:100%;width:var(--pct);background:var(--tm-done-fill)}
.tm-wrap{width:100%;min-width:0;max-width:100%}
.tm-svg{display:block;width:100%;max-width:100%;height:auto}
.tm-svg-n{display:none}
.chart-scroll{width:100%;min-width:0;max-width:100%}
.chart-scroll svg{display:block;width:100%;height:auto}
@media (max-width:640px){
  /* Phones get the tall treemap, drawn about 1:1 so its labels stay legible. */
  .tm-svg-w{display:none}
  .tm-svg-n{display:block}
  /* The history chart scrolls inside its own box; the page does not. */
  .chart-scroll{overflow-x:auto}
  .chart-scroll svg{min-width:640px}
  /* Tap targets of at least 40 px. */
  .sort button,.d-kids button{min-height:40px;padding:8px 12px}
  .stat{min-width:0;flex:1 1 140px}
  main,p,li,td,code{overflow-wrap:anywhere;min-width:0}
  .tm-tip{max-width:calc(100% - 8px)}
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


def index_html(res, head, subject, date, model, when_note, grids=None):
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
            links = " ".join(f'<a href="{esc(link["href"])}">{esc(link["label"])}</a>'
                             for link in node.get("evidence_links", []))
            rows.append(
                f'<tr class="{cls}"><td class="name">{esc(node["name"])}</td>'
                f'<td class="num">{node["weight"]:g}</td>'
                f'<td class="num">{frac}</td>'
                f'<td class="num"><span class="mini" style="--pct:{node["pct"]:.1f}%"><i></i></span> {node["pct"]:.1f}%</td>'
                f'<td class="ev">{esc(node.get("basis", ""))}</td>'
                f'<td class="ev">{esc(node.get("evidence", ""))}<br><em>{esc(node.get("rationale", ""))}</em><br>{links}</td></tr>')
    milestones = "".join(
        f'<li>{"[x]" if m.get("done") else "[ ]"} {esc(m["text"])}<br><span class="ev">{esc(m.get("evidence", ""))}</span></li>'
        for m in res["milestones"])
    cats = "".join(f'<div class="stat"><div class="lbl">{esc(g["name"])}</div><div class="big" style="font-size:1.3rem">{fmt_pct(g["pct"])}</div></div>'
                   for g in res["groups"])
    svg_wide = interactive_treemap(res["groups"], TREEMAP_W, TREEMAP_H, grids, prefix="", svg_class="tm-svg tm-svg-w")
    svg_narrow = interactive_treemap(res["groups"], TREEMAP_NARROW_W, TREEMAP_NARROW_H, grids, prefix="n", svg_class="tm-svg tm-svg-n")
    tm_json = json.dumps(res["groups"]).replace("</", "<\\/")
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fortnite 12.60 port progress</title>
<style>{TREEMAP_PALETTE_CSS}{PAGE_CSS}{TM_PAGE_CSS}</style></head>
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
<div class="stat"><div class="lbl">Objective (git + local basis)</div><div class="big">{res["objective_pct"]:.1f}%</div></div>
<div class="stat"><div class="lbl">Evidence-based (measured + documented)</div><div class="big">{res["measured_pct"]:.1f}%</div></div>
<div class="stat"><div class="lbl">Manual estimates &middot; {res["manual_weight_pct"]:.1f}% of weight</div><div class="big">{res["estimated_pct"]:.1f}%</div></div>
<div class="stat"><div class="lbl">Components measured</div><div class="big">{len(res["leaves"])}</div></div>
<div class="stat"><div class="lbl">Milestones reached</div><div class="big">{res["milestones_done"]} / {res["milestones_total"]}</div></div>
</div>
<p class="meta">The objective figure, {res["objective_pct"]:.1f}%, counts only leaves measured from committed files (<em>git</em>), ignored local outputs (<em>local</em>) or both. Adding documented counts gives the evidence-based {res["measured_pct"]:.1f}%. The remaining {res["estimated_pct"]:.1f}% of the overall is manual estimates; if all of that work were complete, the overall would be {res["manual_full_pct"]:.1f}%.</p>

<h2>Milestones toward the Definition</h2>
<ul class="ms">{milestones}</ul>

<h2>Treemap</h2>
<p class="meta">Tile area = estimated effort weight. Each tile is a neutral surface with a blue fill that rises from its bottom edge to its percent complete, so empty is 0% and full is 100%. Names and percentages are never cut off; a tile too small for its name shows its percentage alone. Decomposable components also show one square per unit under the name, at most 9 px each, packed from the top-left: blue when done, a neutral square when not, grouped by asset type, package or class with a gap row between groups. A tile too small for its squares, too large to draw, or whose units do not reconcile with its own done/total shows the fill level alone. The tile's done/total is in the hover summary. Hover a tile for the summary, click to read the evidence below. The sort toggle re-lays out the tiles by progress instead of area.</p>
{legend_html(res["weight_by_basis"])}
<div class="tm-bar" id="tm-bar">
<div class="sort"><span>Sort tiles by</span>
<button data-sort="weight" class="on" type="button">area (weight)</button>
<button data-sort="progress" type="button">progress</button>
</div>
</div>
<style>{TREEMAP_CSS}{TM_SVG_CSS}</style>
<div class="tm-wrap" id="tm-wrap">{svg_wide}{svg_narrow}<div class="tm-tip" id="tm-tip" hidden></div></div>
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
<p class="meta">Weights and manual percentages are edited in <code>porting/progress/components.json</code>. The generator is <code>porting/progress/progress.py</code>, which reads committed files via <code>git archive</code> at HEAD and ignored local outputs. It records one history point per commit through <code>.githooks/post-commit</code>. Measures only count what is committed; uncommitted stubs and generated artifacts do not add progress unless they are local outputs named in a measure. Denominators come from the tracked <code>porting/progress/totals.json</code> (regenerated by <code>refresh_totals.py</code> from local extraction data), and run checks from the tracked <code>porting/progress/evidence.json</code> (written by <code>record_evidence.py</code>). The dashboard splits the overall into the objective share (<em>git</em> and <em>local</em>), the evidence-based share (adds <em>documented</em>) and the manual-estimate share, so the size of the subjective part is always visible next to the figure it affects.</p>
<footer>Generated offline by porting/progress/progress.py. No external scripts or fonts are loaded; the interactivity is a small inline script, and without JavaScript the same treemap renders as a static picture.</footer>
<script>window.TM_DATA = {tm_json};</script>
<script>{TM_PAGE_JS}</script>
</main></body></html>
"""


def standalone_svg(res, head, subject, date, grids=None):
    width, height = STANDALONE_W, STANDALONE_H
    header = (f'<text x="12" y="28" font-size="20" font-weight="700" class="tm-head">'
              f'Fortnite 12.60 port progress: overall {res["overall_pct"]:.1f}% (effort-weighted)</text>'
              f'<text x="12" y="48" font-size="12" class="tm-head" font-weight="400">'
              f'Commit {esc(head[:7])} | {esc(subject[:90])} | {esc(date)} | milestones {res["milestones_done"]}/{res["milestones_total"]}</text>')
    # Leave 28px at the bottom for the definition footer line.
    body = render_treemap(res["groups"], width, height - 60 - 28, grids=grids, prefix="s")
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" '
            f'role="img" aria-label="Port progress treemap" font-family="system-ui,-apple-system,Segoe UI,sans-serif">'
            f'<style>{TREEMAP_CSS}</style>'
            f'<rect class="tm-bg" x="0" y="0" width="{width}" height="{height}"/>{header}'
            f'<g transform="translate(0,60)">{body}</g>'
            f'<text x="12" y="{height - 10}" font-size="11" class="tm-head" font-weight="400">'
            f'100%: feature-complete rebuilt 12.60 client and server, with all four milestones verified.</text>'
            f'</svg>')


def history_html(history, model):
    entries = history
    n = len(entries)
    W, H = 900, 300
    pl, pr, pt, pb = 52, 16, 16, 34
    ymax = max([10.0] + [math.ceil(v / 5.0) * 5.0 for e in entries
                         for v in (e["overall_pct"], e.get("measured_pct")) if v is not None])
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

    # Embed the original values; the client only changes the visible index range.
    chart_data = json.dumps([
        {"short": e["short"], "date": e["date"][:10], "subject": e["subject"],
         "overall": e["overall_pct"], "measured": e.get("measured_pct")}
        for e in entries
    ], ensure_ascii=True).replace("<", "\\u003c")
    chart_script = r'''<script>
(() => {
  const data = DATA_PLACEHOLDER;
  const svg = document.querySelector('#history-chart svg');
  const status = document.getElementById('history-range');
  const detail = document.getElementById('history-detail');
  const inspector = document.getElementById('history-inspector');
  const selection = document.getElementById('history-selection');
  const buttons = document.querySelectorAll('[data-history-action]');
  document.querySelector('.history-controls').hidden = false;
  inspector.hidden = false;
  const ns = 'http://www.w3.org/2000/svg';
  let start = Math.max(0, data.length - 50), end = data.length - 1;
  let selected = end, marker;
  function element(name, attrs, parent, content) {
    const node = document.createElementNS(ns, name);
    for (const [key, value] of Object.entries(attrs)) node.setAttribute(key, value);
    if (content !== undefined) node.textContent = content;
    parent.appendChild(node);
    return node;
  }
  function render() {
    const width = Math.max(220, Math.round(svg.parentElement.clientWidth));
    const height = 300, left = 52, right = 16, top = 16, bottom = 34;
    const inner = width - left - right, plotHeight = height - top - bottom;
    const count = end - start + 1;
    svg.setAttribute('viewBox', `0 0 ${width} ${height}`);
    svg.replaceChildren();
    if (!data.length) { status.textContent = 'No recorded commits'; buttons.forEach(button => button.disabled = true); selection.disabled = true; detail.textContent = 'No commit to inspect.'; return; }
    const visible = data.slice(start, end + 1);
    let ymin = 0, ymax, step;
    if (count === data.length) {
      ymax = visible.reduce((max, p) => Math.max(max, Math.ceil(p.overall / 5) * 5,
        p.measured == null ? 0 : Math.ceil(p.measured / 5) * 5), 10);
      step = ymax > 10 ? 5 : 2;
    } else {
      const values = visible.flatMap(p => p.measured == null ? [p.overall] : [p.overall, p.measured]);
      const min = values.reduce((a, b) => Math.min(a, b));
      const max = values.reduce((a, b) => Math.max(a, b));
      const padding = Math.max((max - min) * 0.1, 0.001);
      const rawStep = (max - min + 2 * padding) / 5;
      const magnitude = Math.pow(10, Math.floor(Math.log10(rawStep)));
      const fraction = rawStep / magnitude;
      step = (fraction <= 1 ? 1 : fraction <= 2 ? 2 : fraction <= 5 ? 5 : 10) * magnitude;
      ymin = Math.floor((min - padding) / step) * step;
      ymax = Math.ceil((max + padding) / step) * step;
    }
    const x = i => left + (count > 1 ? inner * i / (count - 1) : inner / 2);
    const y = v => top + plotHeight * (ymax - v) / (ymax - ymin);
    const decimals = Math.max(0, Math.min(12, -Math.floor(Math.log10(step))));
    const tickCount = Math.round((ymax - ymin) / step);
    for (let i = 0; i <= tickCount; i++) {
      const tick = ymin + i * step;
      element('line', {x1:left, x2:width-right, y1:y(tick), y2:y(tick), stroke:'var(--line)'}, svg);
      element('text', {x:left-6, y:y(tick)+4, 'text-anchor':'end', 'font-size':11, fill:'var(--ink2)'}, svg, `${(Math.abs(tick) < step / 1000 ? 0 : tick).toFixed(decimals)}%`);
    }
    const line = (field, attrs) => {
      const points = visible.map((p, i) => p[field] == null ? null : `${x(i)},${y(p[field])}`).filter(Boolean);
      if (points.length > 1) element('polyline', {points:points.join(' '), fill:'none', ...attrs}, svg);
    };
    line('measured', {stroke:'var(--ink2)', 'stroke-width':1.5, 'stroke-dasharray':'5 4'});
    line('overall', {stroke:'var(--accent)', 'stroke-width':2});
    // Dense series retain every line vertex; only the decorative dots are hidden.
    if (count === 1 || inner / (count - 1) >= 9) visible.forEach((p, i) => {
      const dot = element('circle', {cx:x(i), cy:y(p.overall), r:3, fill:'var(--accent)'}, svg);
      element('title', {}, dot, `${p.short} ${p.date}: ${p.overall.toFixed(2)}% | ${p.subject}`);
    });
    const labelCount = Math.min(count, Math.max(2, Math.floor(inner / 95) + 1));
    const indexes = new Set();
    for (let i = 0; i < labelCount; i++) indexes.add(labelCount === 1 ? 0 : Math.round(i * (count - 1) / (labelCount - 1)));
    indexes.forEach(i => element('text', {x:x(i), y:height-10, 'text-anchor':i === 0 ? 'start' : i === count-1 ? 'end' : 'middle', 'font-size':11, fill:'var(--ink2)'}, svg, visible[i].short));
    marker = element('line', {y1:top, y2:height-bottom, stroke:'var(--ink2)', 'stroke-width':1, 'stroke-dasharray':'3 3', 'pointer-events':'none'}, svg);
    const overlay = element('rect', {x:left, y:top, width:inner, height:plotHeight, fill:'transparent', 'pointer-events':'all'}, svg);
    overlay.addEventListener('pointermove', event => {
      const rect = svg.getBoundingClientRect();
      const position = (event.clientX - rect.left) * width / rect.width;
      const index = count === 1 ? 0 : Math.max(0, Math.min(count - 1, Math.round((position - left) / inner * (count - 1))));
      inspect(start + index);
    });
    selection.min = start;
    selection.max = end;
    inspect(selected);
    status.textContent = `Showing commits ${start + 1}–${end + 1} of ${data.length} (${count === data.length ? 'zero-baseline' : 'fitted'} scale)`;
    buttons.forEach(button => {
      const action = button.dataset.historyAction;
      button.disabled = action === 'older' ? start === 0 : action === 'newer' ? end === data.length - 1 :
        action === 'in' ? count <= 2 : action === 'out' || action === 'all' ? count === data.length : data.length <= 50;
    });
  }
  function inspect(index) {
    selected = Math.max(start, Math.min(end, index));
    const p = data[selected];
    selection.value = selected;
    const width = Math.max(220, Math.round(svg.parentElement.clientWidth));
    const position = 52 + (end > start ? (width - 68) * (selected - start) / (end - start) : (width - 68) / 2);
    marker.setAttribute('x1', position);
    marker.setAttribute('x2', position);
    selection.setAttribute('aria-valuetext', `Commit ${selected + 1} of ${data.length}: ${p.short}, ${p.date}`);
    const measured = p.measured == null ? '' : `; evidence-based ${p.measured.toFixed(2)}%`;
    detail.textContent = `${p.short} ${p.date}: overall ${p.overall.toFixed(2)}%${measured} | ${p.subject}`;
  }
  selection.addEventListener('input', () => inspect(Number(selection.value)));
  function setRange(first, count) {
    count = Math.max(1, Math.min(data.length, count));
    start = Math.max(0, Math.min(data.length - count, first));
    end = start + count - 1;
    selected = Math.max(start, Math.min(end, selected));
    render();
  }
  buttons.forEach(button => button.addEventListener('click', () => {
    const count = end - start + 1, action = button.dataset.historyAction;
    if (action === 'all') setRange(0, data.length);
    if (action === 'recent') setRange(data.length - 50, 50);
    if (action === 'older') setRange(start - Math.max(1, Math.floor(count / 2)), count);
    if (action === 'newer') setRange(start + Math.max(1, Math.floor(count / 2)), count);
    if (action === 'in' || action === 'out') {
      const next = action === 'in' ? Math.max(2, Math.ceil(count / 2)) : Math.min(data.length, count * 2);
      setRange(Math.round((start + end - next + 1) / 2), next);
    }
  }));
  if (window.ResizeObserver) new ResizeObserver(render).observe(svg.parentElement);
  else window.addEventListener('resize', render);
  render();
})();
</script>'''.replace('DATA_PLACEHOLDER', chart_data)

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
#history-chart svg{{min-width:0}}
.history-controls:not([hidden]){{display:flex;flex-wrap:wrap;align-items:center;gap:6px;margin:.5rem 0}}
.history-controls button{{font:inherit;padding:5px 9px}}
#history-inspector input{{width:min(100%, 35rem);vertical-align:middle}}
#history-detail{{min-height:1.5em}}
</style></head>
<body><main>
<header>
<h1>Port progress history</h1>
<p class="meta">One point per commit: overall effort-weighted progress. <a href="index.html">back to the dashboard</a> &middot; <a href="progress.json">progress.json</a></p>
</header>
<p class="warn">{esc(model["definition"])} Overall values here estimate reconstruction effort; the four milestones require end-to-end evidence.</p>
<h2>Overall progress per commit</h2>
<div class="history-controls" aria-label="History range controls" hidden>
<button type="button" data-history-action="all">All commits</button>
<button type="button" data-history-action="recent">Recent 50</button>
<button type="button" data-history-action="older">← Older</button>
<button type="button" data-history-action="newer">Newer →</button>
<button type="button" data-history-action="in">Zoom in</button>
<button type="button" data-history-action="out">Zoom out</button>
<span id="history-range" class="meta"></span>
</div>
<div class="chart-scroll" id="history-chart">{chart}</div>
<div id="history-inspector" hidden>
<label for="history-selection">Inspect commit in shown range</label>
<input id="history-selection" type="range" step="1">
<p class="meta" id="history-detail" aria-live="polite"></p>
</div>
<p class="meta">The chart opens on the most recent 50 commits when there are more than 50. Use the range controls to browse or show all history, and the slider or pointer to inspect a commit. Selected ranges use a fitted vertical scale; all commits use a zero baseline. Solid line: overall including manual estimates. Dashed line: evidence-based progress only (basis <em>git</em>, <em>local</em> or <em>documented</em>), which includes documented counts. Estimates use the model in effect at each recording; model revisions can cause discontinuities. Points marked replay used the current components.json manual estimates and current ignored local outputs, so they are indicative. Hook-recorded points measure HEAD at commit time.</p>
<h2>Commits</h2>
<div class="tablewrap"><table>
<thead><tr><th>Date (UTC)</th><th>Commit</th><th>Subject</th><th>Overall</th>{head_cells}<th>Source</th></tr></thead>
<tbody>{''.join(rows)}</tbody>
</table></div>
<footer>Generated offline by porting/progress/progress.py.</footer>
{chart_script}
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
        "objective_pct": res["objective_pct"],
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


def head_totals_entries(head_sha):
    """HEAD's totals.json entries, used for commits that predate the file."""
    data = parse_json_or_none(read_rev_file(head_sha, TOTALS_REL))
    return data if isinstance(data, dict) else None


def measure_commit(model, sha, subject, date, head_sha, seeded, fallback_totals=None):
    files = snapshot(sha)
    replay = sha != head_sha
    if fallback_totals is None and replay:
        fallback_totals = head_totals_entries(head_sha)
    res = measure_tree(model, files, replay, rev=sha, fallback_totals=fallback_totals)
    return res, entry_from(res, sha, subject, date, seeded), files


def reconcile_history(history, revisions, head_entry, replay_missing):
    """Keep recorded measurements and repair gaps left by interrupted hooks.

    Git supplies the reachable commit order. Existing historical measurements
    retain their original values; only missing points are replayed.
    """
    known = {entry['commit']: entry for entry in history if 'commit' in entry}
    known[head_entry['commit']] = head_entry
    result = []
    for revision in revisions:
        if revision not in known:
            known[revision] = replay_missing(revision)
        result.append(known[revision])
    return result


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
        head_fb = head_totals_entries(head)

        if args.seed:
            revs = git("rev-list", "--reverse", "HEAD").split()
            history = []
            for sha in revs:
                s_subject = git("log", "-1", "--format=%s", sha).strip()
                s_date = git("log", "-1", "--format=%cI", sha).strip()
                _, entry, _ = measure_commit(model, sha, s_subject, s_date, head, seeded=(sha != head),
                                             fallback_totals=head_fb)
                history.append(entry)
            head_res, _, files = measure_commit(model, head, subject, date, head, seeded=False,
                                                fallback_totals=head_fb)
        else:
            head_res, head_entry, files = measure_commit(model, head, subject, date, head, seeded=False,
                                                         fallback_totals=head_fb)
            if not args.no_record:
                def replay_missing(sha):
                    replay_subject = git("log", "-1", "--format=%s", sha).strip()
                    replay_date = git("log", "-1", "--format=%cI", sha).strip()
                    _, entry, _ = measure_commit(model, sha, replay_subject, replay_date, head,
                                                 seeded=True, fallback_totals=head_fb)
                    return entry
                revisions = git("rev-list", "--reverse", head).split()
                history = reconcile_history(history, revisions, head_entry, replay_missing)

        if not args.no_record or args.seed:
            write_atomic(HISTORY, json.dumps(history, indent=1, sort_keys=False) + "\n")

        # Publish tracked evidence documents from the same commit as the measurements.
        for source, target in (("porting/material_pipeline/README.md", "material-evidence.txt"),
                               ("porting/material_pipeline/UE_BRIDGE.md", "material-bridge-evidence.txt"),
                               ("porting/progress/frontend_event_targets.json", "frontend-event-targets.txt")):
            if source in files:
                write_atomic(os.path.join(OUT, target), files[source])

        # Render from the HEAD measurement. Overall, categories and milestones are the same
        # whether read from history or measured fresh.
        when_note = "Generated by porting/progress/progress.py."
        grids = build_unit_grids(model, files, {l["id"]: l for l in head_res["leaves"]})
        write_atomic(os.path.join(OUT, "index.html"), index_html(head_res, head, subject, date, model, when_note, grids))
        write_atomic(os.path.join(OUT, "progress.svg"), standalone_svg(head_res, head, subject, date, grids))
        progress_doc = {
            "definition": model["definition"],
            "head": head,
            "subject": subject,
            "date": date,
            "overall_pct": head_res["overall_pct"],
            "measured_pct": head_res["measured_pct"],
            "objective_pct": head_res["objective_pct"],
            "weight_by_basis": head_res["weight_by_basis"],
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
