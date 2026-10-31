#!/usr/bin/env python3
"""Static checker for porting/behavior/MANIFEST.json and the .inc snippets (stdlib only).

For every manifest entry with status "ported" it checks:
  1. the class header exists under porting/native/Source/FortniteGame/Generated/;
  2. the signature: "declared" entries must appear verbatim in the generated header (followed by ';');
     "proposed" entries must NOT already be declared (the generator has to add them, reported as PENDING_DECL);
  3. the .inc file exists, its CONFIDENCE tag matches the manifest, and it has no TODO/FIXME/placeholder text;
  4. best effort token check: every bare identifier and every `this->X` in the .inc is a parameter, a local,
     a type, a function, or a member declared in the class or a generated ancestor header.
Skipped entries are checked for a non-empty reason.

Usage:  python3 porting/behavior/check_behavior.py [--verbose]
Exit status is 1 if any FAIL is reported. PENDING_DECL is not a failure.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
GEN = os.path.join(ROOT, "porting", "native", "Source", "FortniteGame", "Generated")
MANIFEST = os.path.join(HERE, "MANIFEST.json")

CPP_KEYWORDS = {
    "if", "else", "return", "nullptr", "true", "false", "const", "void", "bool", "int", "float", "double",
    "uint8", "int8", "int16", "uint16", "int32", "uint32", "int64", "uint64", "this", "static_cast",
    "reinterpret_cast", "auto", "sizeof", "while", "for", "do", "break", "continue", "case", "default",
    "switch", "new", "delete", "unsigned", "signed", "char", "long", "short", "TEXT", "NULL",
    "Super", "ENetRole", "TSubclassOf", "TArray", "TMap", "TSet", "TEnumAsByte",
}
PLACEHOLDER_RE = re.compile(r"\b(TODO|FIXME|TBD|XXX|PLACEHOLDER|placeholder|not recovered|stub body)\b|\.\.\.", re.I)

_header_cache = {}
_members_cache = {}


def read_header(cls):
    if cls not in _header_cache:
        path = os.path.join(GEN, cls + ".h")
        _header_cache[cls] = open(path, encoding="utf-8", errors="replace").read() if os.path.exists(path) else None
    return _header_cache[cls]


def parent_of(cls):
    txt = read_header(cls)
    if not txt:
        return None
    m = re.search(r"^class\s+(?:\w+_API\s+)?" + re.escape(cls) + r"\s*:\s*public\s+(\w+)", txt, re.M)
    return m.group(1) if m else None


def class_chain(cls):
    out, c = [], cls
    while c and c not in out and len(out) < 16:
        if read_header(c) is None:
            break
        out.append(c)
        c = parent_of(c)
    return out


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
    return text


def header_identifiers(cls):
    """Member names, function names, and type names declared in one generated header (best effort)."""
    if cls in _members_cache:
        return _members_cache[cls]
    txt = strip_comments(read_header(cls) or "")
    members, funcs, types = set(), set(), set()
    for line in txt.splitlines():
        s = line.strip()
        if not s:
            continue
        m = re.match(r"(?:class|struct|enum(?:\s+class)?)\s+(?:\w+_API\s+)?(\w+)", s)
        if m and not s.endswith(";"):
            types.add(m.group(1))
        m = re.match(r"class\s+(\w+)\s*;", s)
        if m:
            types.add(m.group(1))
        # function declaration: "<ret> Name(...)" (no '=' before the parenthesis)
        mf = re.match(r"(?:static\s+|virtual\s+|FORCEINLINE\s+)*[\w:<>\*&\s]+?[\s\*&](\w+)\s*\(", s)
        if mf and s.endswith(";") and "=" not in s.split("(")[0]:
            funcs.add(mf.group(1))
        # member declaration: "<type> Name;", "<type> Name[N];", "<type> Name : 1;"
        mm = re.match(r"[\w:<>\*&\s,]+?[\s\*&](\w+)\s*(\[[^\]]*\])?\s*(:\s*\d+)?\s*;\s*$", s)
        if mm and "(" not in s:
            members.add(mm.group(1))
    _members_cache[cls] = (members, funcs, types)
    return _members_cache[cls]


def normalize(sig):
    return re.sub(r"\s+", " ", sig).strip().rstrip(";").strip()


def signature_params(sig):
    m = re.search(r"\((.*)\)", sig)
    if not m:
        return set()
    names = set()
    for part in m.group(1).split(","):
        part = part.strip()
        mm = re.search(r"(\w+)\s*(?:\[[^\]]*\])?\s*$", part)
        if mm and mm.group(1) not in ("void",):
            names.add(mm.group(1))
    return names


def local_names(code):
    names = set()
    for m in re.finditer(r"(?:^|[;{}\n])\s*(?:const\s+)?[A-Za-z_][\w:]*\s*\*?\s*\*?\s*(\w+)\s*(?:=|;)", code):
        names.add(m.group(1))
    return names


def check_entry(e, problems, infos):
    cls, fn = e["class"], e["function"]
    tag = "%s::%s" % (cls, fn)
    hdr = read_header(cls)
    if hdr is None:
        problems.append(("FAIL", tag, "generated header %s.h not found" % cls))
        return
    sig = e.get("signature")
    if not sig:
        problems.append(("FAIL", tag, "missing signature"))
        return
    declared_text = normalize(hdr)
    if e.get("signature_status") == "declared":
        if normalize(sig) + ";" not in declared_text and normalize(sig) not in declared_text:
            problems.append(("FAIL", tag, "signature not found in generated header: %s" % sig))
            return
    else:
        fname = re.search(r"(\w+)\s*\(", sig).group(1)
        if re.search(r"\b" + re.escape(fname) + r"\s*\(", strip_comments(hdr)):
            problems.append(("FAIL", tag, "signature marked proposed but %s is already declared in the header" % fname))
            return
        infos.append(("PENDING_DECL", tag, "generator must declare: %s;" % sig))
    inc_rel = e.get("inc_path")
    if not inc_rel:
        problems.append(("FAIL", tag, "ported entry has no inc_path"))
        return
    inc_path = os.path.join(ROOT, inc_rel)
    if not os.path.exists(inc_path):
        problems.append(("FAIL", tag, "inc file missing: %s" % inc_rel))
        return
    raw = open(inc_path, encoding="utf-8").read()
    conf = e.get("confidence")
    if conf not in ("high", "medium", "low"):
        problems.append(("FAIL", tag, "confidence must be high, medium or low"))
    m = re.search(r"CONFIDENCE:\s*(\w+)", raw)
    if not m or m.group(1).lower() != (conf or "").lower():
        problems.append(("FAIL", tag, "CONFIDENCE tag in .inc does not match manifest"))
    if not re.search(r"Original address:\s*0x[0-9a-fA-F]+", raw):
        problems.append(("FAIL", tag, "header comment lacks the original address"))
    if not re.match(r"^0x[0-9a-fA-F]+$", str(e.get("address") or "")):
        problems.append(("FAIL", tag, "address is not 0x-hex"))
    for ph in PLACEHOLDER_RE.finditer(raw):  # raw text: comments included
        problems.append(("FAIL", tag, "placeholder text in .inc: %r" % ph.group(0)))
        break
    code = strip_comments(raw)
    # the body is everything that is not a '//' header line; comments were stripped above
    chain = class_chain(cls)
    members, funcs, types = set(), set(), set()
    for c in chain:
        mem, fn_, ty = header_identifiers(c)
        members |= mem
        funcs |= fn_
        types |= ty
    allowed = set(CPP_KEYWORDS) | signature_params(sig) | local_names(code) | funcs | members
    allowed |= {c for c in chain}
    bad = []
    tokens = list(re.finditer(r"[A-Za-z_]\w*", code))
    for m in tokens:
        name = m.group(0)
        start = m.start()
        before = code[max(0, start - 3):start]
        if start > 0 and (code[start - 1].isdigit() or code[start - 1] == "."):
            continue  # numeric literal suffix such as 0.0f
        if before.endswith("."):
            continue
        if before.endswith("->"):
            obj = re.search(r"(\w+)\s*->\s*$", code[:start])
            if obj and obj.group(1) == "this" and name not in allowed:
                bad.append("this->%s" % name)
            continue
        if before.endswith("::"):
            continue
        if name in allowed or name.startswith("__"):
            continue
        if name in types or os.path.exists(os.path.join(GEN, name + ".h")):
            continue
        if name[0].isupper() and name in types:
            continue
        bad.append(name)
    seen = set()
    for b in bad:
        if b not in seen:
            seen.add(b)
            problems.append(("FAIL", tag, "identifier not found in class chain or signature: %s" % b))


def check_skipped(e, problems):
    if not e.get("notes"):
        problems.append(("FAIL", "%s::%s" % (e["class"], e["function"]), "skipped entry has no reason"))
    if e.get("inc_path"):
        problems.append(("FAIL", "%s::%s" % (e["class"], e["function"]), "skipped entry must not have an inc_path"))


def main():
    verbose = "--verbose" in sys.argv
    manifest = json.load(open(MANIFEST, encoding="utf-8"))
    problems, infos = [], []
    counts = {"ported": 0, "skipped": 0}
    for e in manifest:
        if e.get("status") == "ported":
            counts["ported"] += 1
            check_entry(e, problems, infos)
        elif e.get("status") == "skipped":
            counts["skipped"] += 1
            check_skipped(e, problems)
        else:
            problems.append(("FAIL", "%s::%s" % (e.get("class"), e.get("function")), "unknown status %r" % e.get("status")))
    fails = [p for p in problems if p[0] == "FAIL"]
    pending = [i for i in infos]
    decl_ok = sum(1 for e in manifest if e.get("status") == "ported" and e.get("signature_status") == "declared")
    print("manifest entries: %d (ported %d, skipped %d)" % (len(manifest), counts["ported"], counts["skipped"]))
    print("signatures declared in generated header: %d of %d ported" % (decl_ok, counts["ported"]))
    print("PENDING_DECL (generator must add a declaration): %d" % len(pending))
    print("FAIL: %d" % len(fails))
    for f in fails:
        print("  FAIL %s: %s" % (f[1], f[2]))
    if verbose:
        for p in pending:
            print("  PENDING %s: %s" % (p[1], p[2]))
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
