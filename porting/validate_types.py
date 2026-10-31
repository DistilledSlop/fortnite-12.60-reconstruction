#!/usr/bin/env python3
"""Validate the recovered struct, enum and delegate tables against the UE 4.25 engine headers (stdlib only).

Reads porting/generated/reflection/{struct,enum,delegate}_blocks.json (porting/extract_types.py join) and the
engine headers under UNREAL_ENGINE/Engine (Source/Runtime, Plugins). For each type that is declared in a header
and present in the image:

  struct    UPROPERTY member names, in declaration order, against the table (offset order)
  enum      enumerator names and values (implicit values count up from the previous one)
  delegate  DECLARE_*DELEGATE* arity and parameter type names

Prints accuracy per kind and the first mismatches. Writes nothing unless --out is given.

Usage: python3 -I porting/validate_types.py [--reflection-dir DIR] [--engine-root DIR] [--out JSON]
"""
import argparse
import collections
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
REFL_DIR = os.path.join(HERE, "generated", "reflection")
ENGINE_ROOT = os.environ.get("FN_ENGINE_ROOT", "/opt/fortnite/UnrealEngine/Engine")
SCAN_DIRS = ("Source/Runtime", "Plugins/Runtime", "Plugins/Developer", "Source/Developer")

STRUCT_HDR = re.compile(r"\bstruct\s+(?:[A-Z][A-Z0-9_]*_API\s+)?(F[A-Za-z0-9_]+)\s*(?::[^{;]*)?\{")
ENUM_HDR = re.compile(r"\benum\s+(?:class\s+)?(E[A-Za-z0-9_]+)\s*(?::\s*\w+\s*)?\{")
DECLARE_HDR = re.compile(
    r"\bDECLARE_(?:DYNAMIC_)?(?:MULTICAST_)?(?:SPARSE_)?DELEGATE(?:_(One|Two|Three|Four|Five|Six|Seven|Eight|Nine)Params?)?"
    r"\(\s*(F[A-Za-z0-9_]+)")
ARITY = {None: 0, "One": 1, "Two": 2, "Three": 3, "Four": 4, "Five": 5, "Six": 6, "Seven": 7, "Eight": 8, "Nine": 9}


def scan_headers(root):
    """Yield (path, text) for each header under the engine scan directories (sorted, Runtime first)."""
    for sub in SCAN_DIRS:
        base = os.path.join(root, sub)
        if not os.path.isdir(base):
            continue
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = sorted(d for d in dirnames if d not in ("Intermediate", "Binaries", "Content"))
            for filename in sorted(filenames):
                if filename.endswith(".h"):
                    path = os.path.join(dirpath, filename)
                    with open(path, "r", encoding="utf-8", errors="replace") as fh:
                        yield path, fh.read()


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def body_after(text, start):
    """Text between the '{' at or after start and its matching '}'."""
    depth = 0
    i = text.index("{", start)
    begin = i + 1
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[begin:i]
        i += 1
    return text[begin:]


def drop_conditional(text):
    """Remove lines inside #if ... #endif blocks: conditional members are not in the shipping table."""
    out, depth = [], 0
    for line in text.splitlines():
        t = line.strip()
        if t.startswith(("#if", "#ifdef", "#ifndef")):
            depth += 1
        elif t.startswith("#endif"):
            depth = max(0, depth - 1)
        elif depth == 0:
            out.append(line)
    return "\n".join(out)


def struct_members(body):
    """UPROPERTY member names directly inside a struct body, in declaration order (conditional blocks dropped)."""
    names = []
    depth = 0
    pending = False
    for line in drop_conditional(body).splitlines():
        s = line.strip()
        if depth == 0 and s.startswith("UPROPERTY"):
            pending = True
        elif depth == 0 and pending and s.endswith(";") and "(" not in s:
            m = re.search(r"(\w+)\s*(?:\[[^\]]*\])?\s*(?::\s*\d+)?\s*;$", s)
            if m:
                names.append(m.group(1))
            pending = False
        depth += s.count("{") - s.count("}")
    return names


def strip_macro(text, macro):
    """Remove MACRO(...) with balanced parentheses (UMETA strings may contain parentheses)."""
    out, i = [], 0
    while True:
        j = text.find(macro + "(", i)
        if j < 0:
            out.append(text[i:])
            break
        out.append(text[i:j])
        k = j + len(macro) + 1
        depth = 1
        while k < len(text) and depth:
            depth += {"(": 1, ")": -1}.get(text[k], 0)
            k += 1
        i = k
    return "".join(out)


def enum_values(body):
    values = []
    nxt = 0
    body = strip_macro(drop_conditional(body), "UMETA")
    for raw in body.split(","):
        item = " ".join(raw.split())
        if not item:
            continue
        m = re.match(r"^(\w+)\s*(?:=\s*(-?\w+))?$", item)
        if not m:
            return None
        if m.group(2) is not None:
            try:
                nxt = int(m.group(2), 0)
            except ValueError:
                return None
        values.append((m.group(1), nxt))
        nxt += 1
    return values


def collect(engine_root):
    """Declarations by type name: {kind: {name: [payload, ...]}}, one payload per header declaration."""
    decls = {"struct": collections.defaultdict(list), "enum": collections.defaultdict(list),
             "delegate": collections.defaultdict(list)}
    for path, raw in scan_headers(engine_root):
        text = strip_comments(raw)
        for m in STRUCT_HDR.finditer(text):
            decls["struct"][m.group(1)[1:]].append(struct_members(body_after(text, m.start())))
        for m in ENUM_HDR.finditer(text):
            decls["enum"][m.group(1)].append(enum_values(body_after(text, m.start())))
        for m in DECLARE_HDR.finditer(text):
            decls["delegate"][m.group(2)].append(ARITY[m.group(1)])
    return decls


def run(args):
    import gen_native_classes as GN  # noqa: F401  (split_properties: the generator's pairing of inner records)
    refl = args.reflection_dir
    structs = json.load(open(os.path.join(refl, "struct_blocks.json")))["structs"]
    enums = json.load(open(os.path.join(refl, "enum_blocks.json")))["enums"]
    delegates = json.load(open(os.path.join(refl, "delegate_blocks.json")))["delegates"]
    decls = collect(args.engine_root)
    stats = collections.Counter()
    mismatches = collections.defaultdict(list)

    for rec in structs:
        name = rec["name"]
        found = decls["struct"].get(name)
        if not found:
            stats["struct_not_in_headers"] += 1
            continue
        if len(found) > 1:
            stats["struct_name_declared_twice_skipped"] += 1
            continue
        hdr = found[0]
        got = [p["name"] for p in reversed(ps(rec))]
        stats["struct_compared"] += 1
        if hdr == got:
            stats["struct_members_exact"] += 1
        elif set(hdr) == set(got):
            stats["struct_members_order_differs"] += 1
            mismatches["struct"].append((name, "order", hdr[:6], got[:6]))
        else:
            stats["struct_members_differ"] += 1
            mismatches["struct"].append((name, "set", sorted(set(hdr) ^ set(got))[:8]))

    for rec in enums:
        name = rec["name"]
        found = decls["enum"].get(name)
        if not found:
            stats["enum_not_in_headers"] += 1
            continue
        if len(found) > 1 or found[0] is None:
            stats["enum_name_declared_twice_or_unparsed_skipped"] += 1
            continue
        got = [(e["name"].split("::")[-1], e["value"]) for e in rec["enumerators"]]
        stats["enum_compared"] += 1
        if found[0] == got:
            stats["enum_values_exact"] += 1
        elif [n for n, _ in found[0]] == [n for n, _ in got]:
            stats["enum_names_same_values_differ"] += 1
            mismatches["enum"].append((name, "values", found[0][:4], got[:4]))
        else:
            stats["enum_names_differ"] += 1
            mismatches["enum"].append((name, "names", [n for n, _ in found[0]][:5], [n for n, _ in got][:5]))

    for rec in delegates:
        name = rec["type_name"]
        found = decls["delegate"].get(name)
        if not found:
            stats["delegate_not_in_headers"] += 1
            continue
        if len(set(found)) > 1:
            stats["delegate_name_declared_twice_skipped"] += 1
            continue
        want_n = len(GN.split_properties([p for p in rec["params"] if "error" not in p])[0])
        stats["delegate_compared"] += 1
        if found[0] == want_n:
            stats["delegate_arity_exact"] += 1
        else:
            stats["delegate_arity_differs"] += 1
            mismatches["delegate"].append((name, "arity", found[0], want_n))

    print("validated against %s (conditional members and names declared more than once are excluded)" % args.engine_root)
    for kind in ("struct", "enum", "delegate"):
        compared = stats["%s_compared" % kind]
        exact = stats.get("%s_members_exact" % kind, 0) + stats.get("%s_values_exact" % kind, 0) + \
            stats.get("%s_arity_exact" % kind, 0)
        if compared:
            print("%s: %d of %d exact (%.1f%%)" % (kind, exact, compared, 100.0 * exact / compared))
        else:
            print("%s: none compared" % kind)
    for k in sorted(stats):
        print("  %s = %d" % (k, stats[k]))
    for kind, items in mismatches.items():
        print("first %s mismatches:" % kind)
        for item in items[:args.show]:
            print("  ", item)
    if args.out:
        with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
            json.dump({"stats": dict(stats), "mismatches": {k: v[:200] for k, v in mismatches.items()}}, fh, indent=1)
            fh.write("\n")
    return 0


def ps(rec):
    """Top-level member records of a struct table, in table order. Inner records of arrays, maps, sets and enums
    are removed with the generator's own pairing (gen_native_classes.split_properties)."""
    import gen_native_classes as GN
    props = [p for p in rec.get("properties", []) if "error" not in p]
    pairs, _notes = GN.split_properties(props)
    return [entry for entry, _children in pairs]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--reflection-dir", default=REFL_DIR)
    ap.add_argument("--engine-root", default=ENGINE_ROOT)
    ap.add_argument("--show", type=int, default=5)
    ap.add_argument("--out", default=None)
    args = ap.parse_args(argv)
    return run(args)


if __name__ == "__main__":
    sys.exit(main())
