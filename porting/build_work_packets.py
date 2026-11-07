#!/usr/bin/env python3
"""Build one self-contained porting work packet per distinct native body.

Inputs (all under porting/generated unless noted):
  thunk-table/final2.json      per-function group, body candidates (real_bodies), resolved chains
  thunk-table/table.json       per-function thunk decompile path (src), kind
  native*/0xADDR.{c,asm,json}  body decompile, capture metadata (body_bytes, called, strings)
  reflection/native_funcs.json reflected signatures and flags
  reflection/fortnite_game_reflection.json  per-class property offsets (keyed by block_va)
  reflection/class_name_map.json     block_va -> class name
  reflection/class_parent_map.json   block_va -> {name, chain}
  behavior/MANIFEST.json       ported helper names (porting/behavior)

Outputs: porting/generated/work-packets/0xADDR.md and index.json.

Usage: python3 -I porting/build_work_packets.py [--limit N]
  --limit N  only write the first N bodies (by address); used for test runs.
"""
import glob
import json
import os
import re
import sys
import time
from collections import Counter, defaultdict
from datetime import datetime, timezone

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GEN = os.path.join(ROOT, "porting", "generated")
OUT = os.path.join(GEN, "work-packets")
OUTPUT_CAP = 150 * 1024 * 1024
BIG = 0x800
THUNK_LINES = 60
PACKET_LINES = 250
MAX_FULL_SERVED = 6
MAX_COMPACT_SERVED = 20
MAX_CALLEES = 20
MAX_STRINGS = 10

FLAG_BITS = [(0x1, "final"), (0x400, "native"), (0x2000, "static"), (0x40000000, "const")]
PROP_OUT = 0x100
PROP_RET = 0x400

stats = Counter()


def J(path):
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        return json.load(fh)


def rel(path):
    return os.path.relpath(path, ROOT).replace(os.sep, "/")


def size_class(n):
    if n <= 0x20:
        return "XS"
    if n <= 0x80:
        return "S"
    if n <= 0x200:
        return "M"
    if n <= 0x800:
        return "L"
    return "XL"


def norm(a):
    return a.lower() if isinstance(a, str) and a else None


def hexint(v):
    if isinstance(v, int):
        return v
    try:
        return int(str(v), 16)
    except (TypeError, ValueError):
        return 0


# ---------------------------------------------------------------- inputs
def load_bodies():
    """Return (bodies, unbodied). bodies: addr -> list of served dicts."""
    final = J(os.path.join(GEN, "thunk-table", "final2.json"))
    table = J(os.path.join(GEN, "thunk-table", "table.json"))
    trow = {(r["cls"], r["fn"]): r for r in table}

    bodies = defaultdict(list)
    unbodied = []
    for row in final:
        cls, fn, g = row["cls"], row["fn"], row["group"]
        t = trow.get((cls, fn), {})
        if g == "U":
            unbodied.append({"cls": cls, "fn": fn, "reason": row.get("reason") or row.get("note") or "U"})
            continue
        if g == "A":
            cands = [norm(row["thunk"])]
            info_by_body = {}
        else:
            cands = [norm(a) for a in (row.get("real_bodies") or [])]
            if not cands:
                cands = [norm(a) for a in (row.get("callees") or row.get("targets") or [])]
                stats["fallback_candidates"] += 1
            info_by_body = defaultdict(list)
            for res in row.get("resolved") or []:
                info_by_body[norm(res.get("real"))].append(res)
        cands = [c for c in dict.fromkeys(cands) if c]
        for b in cands:
            bodies[b].append({
                "cls": cls,
                "fn": fn,
                "group": g,
                "thunk": norm(row["thunk"]),
                "src": t.get("src"),
                "kind": t.get("kind"),
                "confidence": row.get("confidence"),
                "n_bodies": len(cands),
                "resolved": info_by_body.get(b, []) if g != "A" else [],
                "virtual_forward": any(x.get("forward_kind") == "virtual_forward"
                                       for x in (info_by_body.get(b, []) if g != "A" else [])),
            })
    return bodies, unbodied, trow


def index_captures():
    """addr -> {'c','asm','json'} paths; first occurrence across sorted native* dirs."""
    idx = {}
    pat = re.compile(r"^(0x[0-9a-fA-F]+)\.(c|asm|json)$")
    for d in sorted(glob.glob(os.path.join(GEN, "native*"))):
        if not os.path.isdir(d):
            continue
        try:
            names = os.listdir(d)
        except OSError:
            continue
        for f in names:
            m = pat.match(f)
            if not m:
                continue
            addr = m.group(1).lower()
            slot = idx.setdefault(addr, {})
            slot.setdefault(m.group(2), os.path.join(d, f))
    stats["capture_addrs"] = len(idx)
    return idx


def load_manifest_names():
    names = {}
    try:
        man = J(os.path.join(ROOT, "porting", "behavior", "MANIFEST.json"))
    except (OSError, ValueError):
        return names
    for e in man:
        if not isinstance(e, dict) or e.get("status") != "ported":
            continue
        addr = norm(e.get("native_body_address") or e.get("address"))
        if addr and e.get("class") and e.get("function"):
            names.setdefault(addr, f"{e['class']}::{e['function']}")
    return names


def build_name_map(bodies, trow, manifest_names):
    """addr -> list of (priority, name). Lower priority number = preferred."""
    nm = defaultdict(list)
    for addr, served in bodies.items():
        for s in served:
            nm[addr].append((0, f"{s['cls']}::{s['fn']}"))
    for (cls, fn), r in trow.items():
        t = norm(r.get("thunk"))
        if t:
            nm[t].append((2, f"{cls}::{fn}"))
    for addr, name in manifest_names.items():
        nm[addr].append((1, name))
    out = {}
    for addr, lst in nm.items():
        seen = []
        for _, n in sorted(lst):
            if n not in seen:
                seen.append(n)
        out[addr] = seen
    return out


# ---------------------------------------------------------------- reflection
class Layout:
    """Field offsets for a class and its parent chain, from the reflection JSON."""

    def __init__(self):
        cnm = J(os.path.join(GEN, "reflection", "class_name_map.json"))
        cpm = J(os.path.join(GEN, "reflection", "class_parent_map.json"))
        self.block_name = {norm(k): v for k, v in cnm.items()}
        self.name_chain = {}
        for k, v in cpm.items():
            if isinstance(v, dict) and v.get("name"):
                self.name_chain[v["name"]] = v.get("chain") or [v["name"]]
        refl = J(os.path.join(GEN, "reflection", "fortnite_game_reflection.json"))
        self.props = {}
        for rec in refl.get("classes_in_groups_with_fort_names", []):
            name = self.block_name.get(norm(rec.get("block_va")))
            if not name:
                continue
            plist = []
            for p in rec.get("properties") or []:
                off = p.get("offset")
                if isinstance(off, int) and p.get("name"):
                    plist.append((off, p["name"], p.get("kind") or "?", p.get("array_dim") or 1))
            self.props[name] = plist
        del refl
        self.cache = {}
        stats["layout_classes_with_props"] = len(self.props)

    def offset_map(self, cls):
        if cls in self.cache:
            return self.cache[cls]
        chain = self.name_chain.get(cls) or [cls]
        omap = defaultdict(list)
        seen = set()
        for owner in chain:
            for off, pname, kind, _dim in self.props.get(owner, []):
                key = (owner, off, pname)
                if key in seen:
                    continue
                seen.add(key)
                omap[off].append((owner, pname, kind))
        self.cache[cls] = (omap, sorted(omap))
        return self.cache[cls]

    def lookup(self, cls, off):
        """Return (status, text). status: 'exact' | 'nearest' | 'none'."""
        omap, keys = self.offset_map(cls)
        if off in omap:
            owner, pname, kind = omap[off][0]
            alt = len(omap[off]) - 1
            text = f"{pname}:{kind} [{owner}]" + (f" (+{alt} alt)" if alt else "")
            return "exact", text
        i = bisect_left(keys, off)
        if i > 0:
            below = keys[i - 1]
            owner, pname, kind = omap[below][0]
            return "none", f"unresolved, nearest below {pname}+0x{off - below:x}"
        return "none", "unresolved"


def bisect_left(a, x):
    lo, hi = 0, len(a)
    while lo < hi:
        mid = (lo + hi) // 2
        if a[mid] < x:
            lo = mid + 1
        else:
            hi = mid
    return lo


FLAG_RE = re.compile(r"\bparam_1\s*\+\s*(0x[0-9a-fA-F]+)\b")
FUN_RE = re.compile(r"\b(?:thunk_)?FUN_([0-9a-fA-F]{6,})\b")


def fmt_sig(nf, cls, fn):
    rec = nf.get(cls, {}).get(fn)
    if not rec:
        return None, None, "signature not in native_funcs.json"
    args, ret = [], None
    for p in rec.get("params") or []:
        pf = hexint(p.get("property_flags", 0))
        kind = p.get("kind", "?")
        dim = p.get("array_dim") or 1
        kind_s = f"{kind}[{dim}]" if isinstance(dim, int) and dim > 1 else kind
        if p.get("name") == "ReturnValue" or pf & PROP_RET:
            ret = kind_s
            continue
        tag = "out " if pf & PROP_OUT else ""
        args.append(f"{tag}{p.get('name', '?')}: {kind_s}")
    fl = hexint(rec.get("flags", 0))
    bits = [n for b, n in FLAG_BITS if fl & b]
    flag_s = f"{rec.get('flags')}" + (f" ({', '.join(bits)})" if bits else "")
    is_static = bool(fl & 0x2000)
    return f"({', '.join(args)}) -> {ret or 'void'}", flag_s, is_static


def replace_fun(line, names):
    def sub(m):
        addr = "0x" + m.group(1).lower()
        lst = names.get(addr)
        if not lst:
            stats["fun_unnamed"] += 1
            return m.group(0)
        stats["fun_named"] += 1
        label = lst[0] + (f" (+{len(lst) - 1})" if len(lst) > 1 else "")
        return f"{label} /*{addr}*/"
    return FUN_RE.sub(sub, line)


def annotate_this(line, cls, layout):
    offs = []
    for m in FLAG_RE.finditer(line):
        off = int(m.group(1), 16)
        if off not in offs:
            offs.append(off)
    if not offs:
        return line
    parts = []
    for off in offs:
        status, text = layout.lookup(cls, off)
        stats[f"field_{status}"] += 1
        parts.append(f"this+0x{off:x} = {text}")
    return f"{line}  // " + "; ".join(parts)


def read_text(path):
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            return fh.read()
    except OSError:
        return None


# ---------------------------------------------------------------- packet
def build_packet(addr, served, cap, names, nf, layout, trow):
    size = None
    cj = None
    if "json" in cap:
        cj = J(cap["json"])
        size = cj.get("body_bytes")
    size = size or 0
    sc = size_class(size)
    groups = sorted({s["group"] for s in served})
    n = len(served)
    ambiguous = any(s["n_bodies"] > 1 for s in served)
    chains = []
    deltas = set()
    vfwd = any(s["virtual_forward"] for s in served)
    for s in served:
        for r in s["resolved"]:
            for c in r.get("chain") or []:
                if c not in chains:
                    chains.append(c)
            if r.get("this_delta"):
                deltas.add(r["this_delta"])
    primary = served[0]
    primary_static = True
    classes = {s["cls"] for s in served}
    annotate_fields = len(classes) == 1
    sig_map = {}
    for s in served:
        sig, flags, is_static = fmt_sig(nf, s["cls"], s["fn"])
        sig_map[(s["cls"], s["fn"])] = (sig, flags, is_static)
    p_sig = sig_map[(primary["cls"], primary["fn"])]
    if isinstance(p_sig[2], bool):
        primary_static = p_sig[2]
    annotate_fields = annotate_fields and not primary_static

    L = []
    L.append(f"# Work packet {addr}")
    L.append("")
    L.append(f"- Body address: `{addr}`")
    L.append(f"- Size: {size} bytes (class {sc})")
    L.append(f"- Group: {', '.join(groups)} ({'thunk is body' if 'A' in groups else 'reached via real_bodies'})")
    conf = sorted({str(s['confidence']) for s in served if s.get('confidence')})
    L.append(f"- Confidence: {', '.join(conf) if conf else 'n/a (group A/B have no confidence field)'}")
    L.append(f"- Served functions: {n}" + (f" | AMBIGUOUS: functions with multiple candidate bodies" if ambiguous else ""))
    if chains:
        L.append(f"- chain (intermediate addresses): {', '.join(chains)}")
    if deltas:
        L.append(f"- this_delta (secondary-base this adjustment): {', '.join(str(d) for d in sorted(deltas))}")
    if annotate_fields:
        L.append(f"- Field annotations: `param_1` = this of `{primary['cls']}` "
                 f"(chain: {' > '.join(layout.name_chain.get(primary['cls'], [primary['cls']]))})")
    else:
        why = "static function" if primary_static else f"body shared by {len(classes)} classes"
        L.append(f"- Field annotations: off ({why}; no single this layout)")
    L.append("")
    L.append(f"## Served functions ({n})")
    for i, s in enumerate(served[:MAX_FULL_SERVED]):
        sig, flags, _ = sig_map[(s["cls"], s["fn"])]
        L.append(f"{i + 1}. `{s['cls']}::{s['fn']}` [group {s['group']}] thunk `{s['thunk']}`"
                 + (f" kind={s['kind']}" if s.get('kind') else "")
                 + (f" AMBIGUOUS ({s['n_bodies']} candidate bodies)" if s['n_bodies'] > 1 else ""))
        if sig is None:
            L.append("   - signature: not in native_funcs.json")
        else:
            L.append(f"   - signature: `{s['fn']}{sig}`")
            L.append(f"   - flags: {flags}")
        for r in s["resolved"]:
            if (r.get("entry") != addr) or r.get("chain") or r.get("this_delta") or r.get("forward_kind"):
                bits = [f"entry `{r.get('entry')}`"]
                if r.get("chain"):
                    bits.append("chain " + ", ".join(r["chain"]))
                if r.get("this_delta"):
                    bits.append(f"this_delta {r['this_delta']}")
                if r.get("forward_kind"):
                    bits.append(f"forward {r['forward_kind']}")
                L.append("   - resolved: " + "; ".join(bits))
    for s in served[MAX_FULL_SERVED:MAX_FULL_SERVED + MAX_COMPACT_SERVED]:
        L.append(f"- `{s['cls']}::{s['fn']}` [group {s['group']}] thunk `{s['thunk']}`"
                 + (f" ({s['n_bodies']} candidates)" if s['n_bodies'] > 1 else ""))
    if n > MAX_FULL_SERVED + MAX_COMPACT_SERVED:
        L.append(f"- ... {n - MAX_FULL_SERVED - MAX_COMPACT_SERVED} more served functions (see index.json first3 / porting/generated/thunk-table/final2.json)")
    L.append("")

    # body decompile (filled later with budget)
    body_src = read_text(cap["c"]) if "c" in cap else None
    body_lines = []
    if body_src is None:
        body_lines = ["(no decompile capture for this address)"]
    else:
        for line in body_src.rstrip().splitlines():
            line = line.rstrip()
            line = replace_fun(line, names)
            if annotate_fields:
                line = annotate_this(line, primary["cls"], layout)
            body_lines.append(line)
    body_total = len(body_lines)

    # thunk section: first served function whose thunk is not the body
    thunk_sec = []
    thunk_pick = next((s for s in served if s["thunk"] != addr), None)
    if thunk_pick is None:
        thunk_sec.append("Thunk: same as body (group A thunk is the body; see body above).")
    else:
        tsrc = thunk_pick.get("src")
        tpath = os.path.join(ROOT, tsrc) if tsrc else None
        ttext = read_text(tpath) if tpath else None
        thunk_sec.append(f"Thunk of `{thunk_pick['cls']}::{thunk_pick['fn']}` (`{thunk_pick['thunk']}`, src `{tsrc}`), "
                         f"truncated to {THUNK_LINES} lines:")
        if ttext is None:
            thunk_sec.append("(thunk decompile not readable)")
        else:
            tl = ttext.rstrip().splitlines()
            tl = [replace_fun(x.rstrip(), names) for x in tl[:THUNK_LINES]]
            if annotate_fields:
                tl = [annotate_this(x, thunk_pick["cls"], layout) if FLAG_RE.search(x) else x for x in tl]
            thunk_sec.append("```c")
            thunk_sec.extend(tl)
            if len(ttext.rstrip().splitlines()) > THUNK_LINES:
                thunk_sec.append(f"// ... truncated ({len(ttext.rstrip().splitlines()) - THUNK_LINES} more lines)")
            thunk_sec.append("```")

    # callees
    callee_sec = ["## Callees"]
    called = (cj or {}).get("called") or []
    if not called:
        callee_sec.append("- none recorded in capture")
    for c in called[:MAX_CALLEES]:
        ca = norm(c.get("address"))
        captured = "captured" if ca in CAPIDX else "NOT captured"
        served_name = names.get(ca, [None])[0] if ca else None
        label = f" -> `{served_name}`" if served_name else ""
        callee_sec.append(f"- `{ca}` `{c.get('name')}`{label} [{captured}]")
    if len(called) > MAX_CALLEES:
        callee_sec.append(f"- ... {len(called) - MAX_CALLEES} more")
    callee_sec.append("")

    # strings
    str_sec = ["## Strings"]
    strings = (cj or {}).get("strings") or []
    if not strings:
        str_sec.append("- none")
    for st in strings[:MAX_STRINGS]:
        txt = st.get("text", "") if isinstance(st, dict) else str(st)
        str_sec.append(f"- `{txt[:120]}`" + (f" @ {st.get('address')}" if isinstance(st, dict) and st.get('address') else ""))
    if len(strings) > MAX_STRINGS:
        str_sec.append(f"- ... {len(strings) - MAX_STRINGS} more")
    str_sec.append("")

    # hints
    hints = ["## Hints"]
    if any(s["group"] == "C" for s in served) or vfwd:
        if vfwd:
            hints.append("- virtual: body is `this->virtual slot` forward (forward_kind=virtual_forward); the real body lives in the override")
        if any(s["group"] == "C" for s in served):
            hints.append("- virtual: vtable target (group C); override-dispatched, real body may differ per subclass")
    if n > 1:
        hints.append(f"- shared stub: serves {n} functions ({len({s['cls'] for s in served})} classes)")
    if size and size <= 0x20 and not called:
        hints.append("- inline getter/setter candidate (<=32 bytes, no calls)")
    if ambiguous:
        hints.append("- ambiguous: at least one served function has multiple candidate bodies")
    if any(s["kind"] == "inline" for s in served):
        hints.append("- thunk kind=inline (thunk is small and inlined)")
    if body_src is None:
        hints.append("- no decompile capture for this body")
    if len(hints) == 1:
        hints.append("- none")
    hints.append("")

    # assemble with line budget
    head = list(L)
    fixed = len(head) + len(thunk_sec) + len(callee_sec) + len(str_sec) + len(hints) + 6
    budget = max(60, PACKET_LINES - fixed)
    truncated = body_total > budget
    shown = body_lines[:budget]
    body_block = [f"## Body decompile (lines 1-{len(shown)} of {body_total})" + (" [TRUNCATED]" if truncated else ""),
                  "```c"] + shown
    if truncated:
        body_block.append(f"// ... truncated: {body_total - len(shown)} more lines; full text: {rel(cap['c'])}")
    body_block.append("```")
    body_block.append("")

    out = head + body_block + thunk_sec + [""] + callee_sec + str_sec + hints
    text = "\n".join(out) + "\n"
    info = {"size": size, "size_class": sc, "groups": groups, "served": n,
            "truncated": truncated, "body_lines": body_total, "lines": len(out)}
    return text, info


CAPIDX = {}


def main(argv):
    limit = None
    if "--limit" in argv:
        limit = int(argv[argv.index("--limit") + 1])
    t0 = time.time()
    global CAPIDX
    bodies, unbodied, trow = load_bodies()
    CAPIDX = index_captures()
    manifest_names = load_manifest_names()
    names = build_name_map(bodies, trow, manifest_names)
    nf = J(os.path.join(GEN, "reflection", "native_funcs.json"))
    layout = Layout()
    print(f"loaded in {time.time() - t0:.1f}s: {len(bodies)} distinct bodies, {len(unbodied)} unbodied", flush=True)

    os.makedirs(OUT, exist_ok=True)
    for old in glob.glob(os.path.join(OUT, "0x*.md")):
        os.remove(old)

    order = sorted(bodies, key=lambda a: int(a, 16))
    if limit:
        order = order[:limit]

    # size first for the output cap
    sizes = {}
    for a in order:
        cap = CAPIDX.get(a, {})
        size = 0
        if "json" in cap:
            try:
                size = J(cap["json"]).get("body_bytes") or 0
            except (OSError, ValueError):
                size = 0
        sizes[a] = size
    order.sort(key=lambda a: (sizes[a], int(a, 16)))

    total = 0
    records = []
    skipped = 0
    for a in order:
        cap = CAPIDX.get(a, {})
        served = sorted(bodies[a], key=lambda s: (s["group"], s["cls"], s["fn"]))
        rec = {
            "address": a,
            "size": sizes[a],
            "size_class": size_class(sizes[a]),
            "groups": sorted({s["group"] for s in served}),
            "served": len(served),
            "first3": [f"{s['cls']}::{s['fn']}" for s in served[:3]],
            "packet": None,
            "skipped": False,
            "reason": None,
        }
        if not cap.get("c") and not cap.get("json"):
            rec["reason"] = "no capture"
        text, info = build_packet(a, served, cap, names, nf, layout, trow)
        if total + len(text.encode("utf-8")) > OUTPUT_CAP and sizes[a] > BIG:
            rec["skipped"] = True
            rec["reason"] = f"output cap {OUTPUT_CAP >> 20} MB reached; body > 0x800 bytes"
            skipped += 1
            records.append(rec)
            continue
        if total + len(text.encode("utf-8")) > OUTPUT_CAP:
            rec["skipped"] = True
            rec["reason"] = f"output cap {OUTPUT_CAP >> 20} MB reached"
            skipped += 1
            records.append(rec)
            continue
        path = os.path.join(OUT, f"{a}.md")
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(text)
        total += len(text.encode("utf-8"))
        rec["packet"] = rel(path)
        rec["truncated"] = info["truncated"]
        records.append(rec)

    # index
    records.sort(key=lambda r: int(r["address"], 16))
    sc_counts = Counter(r["size_class"] for r in records)
    sc_written = Counter(r["size_class"] for r in records if not r["skipped"])
    grp_counts = Counter("+".join(r["groups"]) for r in records)
    primary_grp = Counter(r["groups"][0] for r in records)
    index = {
        "generated_utc": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "source": "porting/generated/thunk-table/final2.json (real_bodies)",
        "distinct_bodies": len(order),
        "bodies_written": sum(1 for r in records if not r["skipped"]),
        "bodies_skipped": skipped,
        "output_bytes": total,
        "output_cap_bytes": OUTPUT_CAP,
        "size_classes": {k: {"bodies": sc_counts.get(k, 0), "written": sc_written.get(k, 0)}
                         for k in ["XS", "S", "M", "L", "XL"]},
        "size_class_bounds": {"XS": "<=0x20", "S": "<=0x80", "M": "<=0x200", "L": "<=0x800", "XL": ">0x800"},
        "groups_by_body_set": dict(grp_counts),
        "groups_by_primary_group": dict(primary_grp),
        "unbodied_functions": unbodied,
        "field_annotations": {"exact": stats["field_exact"], "unresolved": stats["field_none"]},
        "fun_names": {"named": stats["fun_named"], "unnamed": stats["fun_unnamed"]},
        "fallback_candidates": stats["fallback_candidates"],
        "bodies": records,
    }
    with open(os.path.join(OUT, "index.json"), "w", encoding="utf-8") as fh:
        json.dump(index, fh, indent=1)
    dt = time.time() - t0
    print(f"wrote {len(records) - skipped} packets, {skipped} skipped, {total / 1048576:.1f} MB in {dt:.1f}s", flush=True)
    print(json.dumps({"size_classes": index["size_classes"], "groups_by_primary_group": index["groups_by_primary_group"],
                      "field_annotations": index["field_annotations"], "fun_names": index["fun_names"],
                      "fallback_candidates": index["fallback_candidates"]}))


if __name__ == "__main__":
    main(sys.argv[1:])
