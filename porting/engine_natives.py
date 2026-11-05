#!/usr/bin/env python3
"""Verify which reflected native functions are provided by the stock UE 4.25 engine source.

For every class in generated/reflection/native_funcs.json whose name is declared in the UE source tree
(Engine/Source and Engine/Plugins headers), check that each reflected function with an exec address is
declared in that class's own body with a matching parameter count. Declaration-level evidence only: the
engine body is compiled from source, but Epic's fork may differ in behaviour, which this does not test.

  python3 -I porting/engine_natives.py --ue /opt/fortnite/UnrealEngine/Engine [--out porting/progress/engine_provided_natives.json]

Result file: {"_meta":{...}, "verified":{"Class":["Fn",...]}, "unverified":{"Class":{"Fn":"reason"}}}
Unverified functions stay pending (they are the real gaps against the stock engine).
"""
import argparse, collections, json, os, re, sys

ROOT = os.path.dirname(os.path.abspath(__file__))
NATIVE = os.path.join(ROOT, "generated", "reflection", "native_funcs.json")

CLASS_RE = re.compile(r"\bclass\s+(?:[A-Z_0-9]+_API\s+)?([AUFI][A-Za-z0-9_]+)\s*(?:final\s*)?(?::([^{;]*))?\{")
SKIP_CALLS = {"UFUNCTION", "UPROPERTY", "UCLASS", "GENERATED_BODY", "GENERATED_UCLASS_BODY", "DECLARE_FUNCTION",
              "if", "for", "while", "switch", "return", "sizeof", "static_assert", "check", "ensure", "checkf",
              "DECLARE_DYNAMIC_MULTICAST_DELEGATE", "DECLARE_DELEGATE", "TEXT", "UE_DEPRECATED", "DEPRECATED",
              "meta", "Category", "WithValidation", "BlueprintCallable", "BlueprintPure"}


def strip_comments(t):
    t = re.sub(r"/\*.*?\*/", " ", t, flags=re.S)
    return re.sub(r"//[^\n]*", " ", t)


def match_brace(t, i):
    depth = 0
    n = len(t)
    while i < n:
        c = t[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def split_params(s):
    s = s.strip()
    if not s or s == "void":
        return 0
    depth = 0
    n = 1
    for c in s:
        if c in "(<[{":
            depth += 1
        elif c in ")>]}":
            depth -= 1
        elif c == "," and depth == 0:
            n += 1
    return n


def class_functions(body):
    """{name: set(param_counts)} for member functions at class-body depth 1 (nested braces blanked)."""
    flat = []
    depth = 0
    for c in body:
        if c == "{":
            depth += 1
            if depth == 1:
                flat.append(c)
            continue
        if c == "}":
            depth -= 1
            if depth == 0:
                flat.append(c)
            continue
        if depth <= 1:
            flat.append(c)
    flat = "".join(flat)
    out = collections.defaultdict(set)
    for m in re.finditer(r"([~A-Za-z_][A-Za-z0-9_]*)\s*\(", flat):
        name = m.group(1)
        if name in SKIP_CALLS:
            continue
        # balanced parameter list
        i = m.end()
        d = 1
        j = i
        while j < len(flat) and d:
            if flat[j] == "(":
                d += 1
            elif flat[j] == ")":
                d -= 1
            j += 1
        out[name].add(split_params(flat[i:j - 1]))
    return out


def build_ue_index(ue):
    idx = {}
    for sub in ("Source", "Plugins"):
        for r, _, fs in os.walk(os.path.join(ue, sub)):
            for f in fs:
                if not f.endswith(".h"):
                    continue
                p = os.path.join(r, f)
                try:
                    t = strip_comments(open(p, encoding="utf-8", errors="ignore").read())
                except OSError:
                    continue
                for m in CLASS_RE.finditer(t):
                    end = match_brace(t, m.end() - 1)
                    if end < 0:
                        continue
                    name = m.group(1)
                    # a class may be declared once; keep the first with UFUNCTION-bearing body
                    if name in idx and "UFUNCTION" not in t[m.end():end]:
                        continue
                    bases = re.findall(r"(?:public|protected|private)\s+([A-Za-z_][A-Za-z0-9_]*)", m.group(2) or "")
                    idx[name] = (os.path.relpath(p, ue), class_functions(t[m.end() - 1:end + 1]), bases)
    return idx


def real_param_count(meta):
    """Real parameters only: CPF_Parm (0x80) set, CPF_ReturnParm (0x400) clear. Nested entries (array element
    types, enum underlying types) carry no CPF_Parm and are not parameters."""
    def _flags(p):
        v = p.get("property_flags") or 0
        return int(v, 16) if isinstance(v, str) else v
    return len([p for p in meta.get("params") or [] if _flags(p) & 0x80 and not _flags(p) & 0x400])


def classify(ue, cls, fn, n):
    """('verified', where) | ('param', detail) | ('absent', detail) against one engine index."""
    if cls not in ue:
        return "absent", "class not in engine"
    header, decls, bases = ue[cls]
    cands = [(cls, header, decls)]
    # UINTERFACE classes: the reflected UFoo's functions are declared on the C++ IFoo class.
    iface = "I" + cls[1:]
    if iface in ue:
        cands.append((iface, ue[iface][0], ue[iface][1]))
    todo, seen = list(bases), set()
    while todo:
        b = todo.pop()
        if b in seen or b not in ue:
            continue
        seen.add(b)
        cands.append((b, ue[b][0], ue[b][1]))
        todo += ue[b][2]
    declared = None
    for name, h, d in cands:
        if fn in d:
            if n in d[fn]:
                return "verified", "%s in %s" % (name, h)
            declared = declared or "param count %d vs declared %s in %s" % (n, sorted(d[fn]), h)
    return ("param", declared) if declared else ("absent", "not declared in %s or its bases" % header)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ue", required=True)
    ap.add_argument("--ref", help="reference engine (e.g. UE 4.26) used only to annotate the gaps")
    ap.add_argument("--out", default=os.path.join(ROOT, "progress", "engine_provided_natives.json"))
    a = ap.parse_args()
    ue = build_ue_index(a.ue)
    ref = build_ue_index(a.ref) if a.ref else None
    nf = json.load(open(NATIVE))
    verified = collections.defaultdict(list)
    unverified = collections.defaultdict(dict)
    stats = collections.Counter()
    for cls, fns in sorted(nf.items()):
        if cls not in ue:
            continue
        for fn, meta in sorted(fns.items()):
            if not meta.get("exec_addr"):
                continue
            n = real_param_count(meta)
            status, detail = classify(ue, cls, fn, n)
            if status == "verified":
                verified[cls].append(fn)
                stats["verified"] += 1
                continue
            rec = {"reason": detail}
            if ref is not None:
                rs, rd = classify(ref, cls, fn, n)
                rec["reference_engine"] = ("declared: " + rd) if rs == "verified" else ("not matched: " + rd)
                stats["gap_declared_in_reference" if rs == "verified" else "gap_not_in_reference"] += 1
            unverified[cls][fn] = rec
            stats["gap_" + status] += 1
    out = {"_meta": {"method": "declaration + parameter-count match in stock UE 4.25 headers, own class, its bases "
                               "and the matching I-class for UINTERFACEs (porting/engine_natives.py)",
                     "reference_engine": a.ref and os.path.basename(os.path.dirname(a.ref.rstrip("/"))) or None,
                     "caveat": "declaration-level only; Epic fork body differences are not tested",
                     "counts": dict(stats)},
           "verified": verified, "unverified": unverified}
    with open(a.out, "w") as fh:
        json.dump(out, fh, indent=1, sort_keys=True)
        fh.write("\n")
    print(dict(stats))


if __name__ == "__main__":
    main()
