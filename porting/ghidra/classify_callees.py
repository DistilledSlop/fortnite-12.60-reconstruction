#!/usr/bin/env python3
"""Tag the probed callees by the evidence in their own code (offline, reads porting/generated/callee_probe/).

Input: probe directories written by probe_callees.py (<addr>.json, <addr>.c, <addr>.asm, index.json), the
blocked-count ranking from porting/behavior/BLOCKING_CALLEES.md, and the identities in name_tables/helpers.json.

For each probed address it writes one row with:
  identified   the helper name from helpers.json (second pass), or null
  blocked      the number of candidate functions it blocks (BLOCKING_CALLEES.md, 0 if not listed)
  tags         evidence tags from the body (see TAG_RULES); a callee can carry several
Output: <out>/classes.json (rows) and a count table on stdout. No Ghidra project is opened.

Usage:  porting/.venv/bin/python -I porting/ghidra/classify_callees.py --probe DIR [--probe DIR ...] --out DIR
"""
import argparse
import collections
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
HELPERS = os.path.join(HERE, "name_tables", "helpers.json")
BLOCKING = os.path.join(REPO, "porting", "behavior", "BLOCKING_CALLEES.md")
GPSC_BODY = "FUN_142d698b0"  # GetPrivateStaticClassBody (identified, see helpers.json)
ISA_RE = re.compile(r"\*\(int \*\)\(lVar\d+ \+ 0x38\) <= \*\(int \*\)\(\*\(longlong \*\)\(\w+ \+ 0x10\) \+ 0x38\)")
LOCK_RE = re.compile(r"\bLOCK\(\)")

TAG_RULES = [
    # (tag, predicate(detail, c_text, asm_text))
    ("gpsc_static_class", lambda d, c, a: GPSC_BODY in [x["name"] for x in d["called"]] or "/Script/" in c),
    ("isa_check", lambda d, c, a: bool(ISA_RE.search(c))),
    ("weak_object_lookup", lambda d, c, a: "0x18" in c and "_DAT_148263a18" in c),
    ("tail_jump_vtable", lambda d, c, a: "JMP qword ptr" in a and "CALL qword ptr" not in a and "Treating indirect jump" in c),
    ("vtable_call", lambda d, c, a: d["indirect_calls"] > 0),
    ("calls_FMemory_Free", lambda d, c, a: any(x["name"] == "FMemory_Free" for x in d["called"])),
    ("refcount_lock", lambda d, c, a: bool(LOCK_RE.search(c))),
    ("string_literal", lambda d, c, a: len(d["strings"]) > 0),
    ("log_or_assert_text", lambda d, c, a: any(re.search(r"Fatal|Log|callstack|File:", s["text"]) for s in d["strings"])),
    ("tiny_no_calls", lambda d, c, a: d["instructions"] <= 12 and len(d["called"]) == 0),
    ("calls_only_helpers", lambda d, c, a: len(d["called"]) > 0 and all(
        (x["name"] or "").split("_")[0] in ("FMemory", "FName", "FWeakObjectPtr", "UObject", "FMsg", "FTimerManager")
        or not (x["name"] or "").startswith("FUN_") for x in d["called"])),
]


def load_probe(dirs):
    rows = {}
    for d in dirs:
        idx = os.path.join(d, "index.json")
        if not os.path.exists(idx):
            continue
        for r in json.load(open(idx, encoding="utf-8"))["functions"]:
            if r.get("status") != "ok":
                continue
            stem = os.path.join(d, r["address"])
            detail = json.load(open(stem + ".json", encoding="utf-8"))
            code = open(stem + ".c", encoding="utf-8").read() if os.path.exists(stem + ".c") else ""
            asm = open(stem + ".asm", encoding="utf-8").read() if os.path.exists(stem + ".asm") else ""
            rows[r["address"]] = (detail, code, asm)
    return rows


def blocked_counts(path):
    out = {}
    if not os.path.exists(path):
        return out
    for line in open(path, encoding="utf-8"):
        m = re.match(r"\|\s*\d+\s*\|\s*(0x[0-9a-f]+)\s*\|\s*(\d+)\s*\|", line)
        if m:
            out[m.group(1)] = int(m.group(2))
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(description="Tag probed callees by their own code (see module doc)")
    ap.add_argument("--probe", action="append", required=True, help="probe output directory (repeatable)")
    ap.add_argument("--out", required=True)
    args = ap.parse_args(argv if argv is not None else sys.argv[1:])
    rows = load_probe([os.path.abspath(p) for p in args.probe])
    helpers = {f["address"].lower(): f["name"] for f in json.load(open(HELPERS, encoding="utf-8"))["functions"]}
    blocked = blocked_counts(BLOCKING)
    out_rows, counts = [], collections.Counter()
    for addr in sorted(rows, key=lambda a: int(a, 16)):
        detail, code, asm = rows[addr]
        tags = [t for t, pred in TAG_RULES if pred(detail, code, asm)]
        ident = helpers.get(addr.lower())
        row = {"address": addr, "name": detail["name"], "identified": ident, "blocked": blocked.get(addr, 0),
               "instructions": detail["instructions"], "callers": len(detail["callers"]), "tags": tags}
        out_rows.append(row)
        counts["identified" if ident else "unidentified"] += 1
        for t in tags:
            counts["tag:" + t] += 1
    os.makedirs(os.path.abspath(args.out), exist_ok=True)
    with open(os.path.join(os.path.abspath(args.out), "classes.json"), "w", encoding="utf-8") as fh:
        json.dump({"_meta": {"rows": len(out_rows), "tags": [t for t, _ in TAG_RULES]}, "rows": out_rows}, fh, indent=1)
    for k, v in sorted(counts.items(), key=lambda kv: (-kv[1], kv[0])):
        print("%5d  %s" % (v, k))
    return 0


if __name__ == "__main__":
    sys.exit(main())
