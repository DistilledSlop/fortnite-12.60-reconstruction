#!/usr/bin/env python3
"""Static checker for porting/behavior/MANIFEST.json and the .inc snippets (stdlib only).

For every manifest entry with status "ported" it checks:
  1. the class header exists under porting/native/Source/FortniteGame/Generated/;
  2. the signature: "declared" entries must appear verbatim in the generated header (followed by ';');
     "proposed" entries are declared (counted in declared_in_header) once the generator emits them, and are
     reported as PENDING_DECL until then;
  3. the .inc file exists, its CONFIDENCE tag matches the manifest, and it has no TODO/FIXME/placeholder text;
  4. best effort token check: every bare identifier and every `this->X` in the .inc is a parameter, a local,
     a type, a function, or a member declared in the class or a generated ancestor header.
Skipped entries are checked for a non-empty reason.

Usage:  python3 porting/behavior/check_behavior.py [--verbose]
        python3 porting/behavior/check_behavior.py --gen-root <dir>/FortniteGame/Generated [--splice-report <behavior_splice.json>]
The --gen-root mode checks a generated tree instead of native/ (it does not write COUNTS.txt). For each ported entry
it checks the declaration (and UFUNCTION specifier: BlueprintCallable for getter/exec, WithValidation for the RPC of a
validate), the definition with the same signature, and that the body equals the .inc. An entry whose body is not
spliced is a FAIL unless --splice-report lists it as rejected.
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
# Engine types and casts that snippets may use without a generated header (UE 4.25 names).
ENGINE_NAMES = {"UClass", "UObject", "AActor", "FWeakObjectPtr", "TWeakObjectPtr", "FName", "FString", "FText",
                "FVector", "TArray", "static_cast", "Cast"}
PLACEHOLDER_RE = re.compile(r"\b(TODO|FIXME|TBD|XXX|PLACEHOLDER|placeholder|not recovered|stub body)\b|\.\.\.", re.I)

_header_cache = {}
_members_cache = {}
DECLARED = set()   # "Class::Function" of ported entries whose signature is found in the generated header


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


def check_entry(e, problems, infos, waived=frozenset()):
    """waived: tags the splice report rejected (--gen-root): a declaration that is not emitted is then expected."""
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
    if normalize(sig) + ";" in declared_text:
        DECLARED.add(tag)
    elif tag in waived:
        infos.append(("NOT_SPLICED", tag, "rejected by the splice report, declaration not emitted"))
    elif e.get("signature_status") == "declared":
        problems.append(("FAIL", tag, "signature not found in generated header: %s" % sig))
        return
    else:
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
        if name in allowed or name in ENGINE_NAMES or name.startswith("__"):
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


def squash(text):
    return re.sub(r"\s+", "", text or "").rstrip(";")


def snippet_body_lines(inc_rel):
    """The .inc body as the generator emits it inside a function: one tab of indent per non-empty line."""
    raw = open(os.path.join(ROOT, inc_rel), encoding="utf-8").read().strip("\n")
    return [("\t" + line.rstrip()) if line.strip() else "" for line in raw.split("\n")]


def check_generated(e, rejected, problems, counts):
    """--gen-root: declaration, UFUNCTION specifier, definition and body of one ported entry in a generated tree."""
    cls, fn = e["class"], e["function"]
    tag = "%s::%s" % (cls, fn)
    if tag in rejected:
        counts["not_spliced"] += 1
        return
    hdr, cpp = read_header(cls), read_cpp(cls)
    if hdr is None or cpp is None:
        problems.append(("FAIL", tag, "generated %s.h or %s.cpp not found" % (cls, cls)))
        return
    m = re.match(r"^(?P<ret>.+?)\s+(?P<fn>\w+)\((?P<args>.*)\)\s*(?P<const>const)?\s*;?$", normalize(e.get("signature")))
    if not m:
        problems.append(("FAIL", tag, "cannot parse signature %r" % e.get("signature")))
        return
    ret, sfn, args, const = m["ret"], m["fn"], m["args"], " const" if m["const"] else ""
    hl = [squash(line) for line in hdr.splitlines()]
    decl_key = squash("%s %s(%s)%s;" % (ret, sfn, args, const))
    if hl.count(decl_key) != 1:
        problems.append(("FAIL", tag, "declaration %r found %d times in header" % (decl_key, hl.count(decl_key))))
        return
    decl_index = hl.index(decl_key)
    ufunc = hl[decl_index - 1] if decl_index else ""
    if e["kind"] in ("getter", "exec") and "UFUNCTION(BlueprintCallable" not in ufunc:
        problems.append(("FAIL", tag, "UFUNCTION before the declaration lacks BlueprintCallable: %r" % ufunc))
    if e["kind"] == "validate":
        rpc = sfn[:-len("_Validate")]
        rpc_key = squash("void %s(%s);" % (rpc, args))
        if hl.count(rpc_key) != 1:
            problems.append(("FAIL", tag, "RPC %s declaration missing or repeated" % rpc))
        elif "WithValidation" not in hl[hl.index(rpc_key) - 1]:
            problems.append(("FAIL", tag, "RPC %s is not WithValidation" % rpc))
    cl = [line.rstrip() for line in cpp.splitlines()]
    def_key = squash("%s %s::%s(%s)%s" % (ret, cls, sfn, args, const))
    starts = [i for i, line in enumerate(cl) if squash(line) == def_key]
    if len(starts) != 1 or starts[0] + 1 >= len(cl) or cl[starts[0] + 1] != "{":
        problems.append(("FAIL", tag, "definition %r found %d times in cpp (or not followed by '{')" % (def_key, len(starts))))
        return
    i = starts[0]
    end = next((j for j in range(i + 2, len(cl)) if cl[j] == "}"), None)
    body = cl[i + 2:end] if end is not None else None
    if body == snippet_body_lines(e["inc_path"]):
        counts["spliced"] += 1
    elif tag in rejected:
        counts["not_spliced"] += 1
    else:
        problems.append(("FAIL", tag, "body is not the snippet (stub or other text) and the entry is not a rejection"))


def read_cpp(cls):
    path = os.path.join(GEN, cls + ".cpp")
    return open(path, encoding="utf-8", errors="replace").read() if os.path.exists(path) else None


def main():
    global GEN
    argv = sys.argv[1:]
    verbose = "--verbose" in argv
    gen_root = argv[argv.index("--gen-root") + 1] if "--gen-root" in argv else None
    splice_report = argv[argv.index("--splice-report") + 1] if "--splice-report" in argv else None
    if gen_root:
        GEN = os.path.abspath(gen_root)
        _header_cache.clear()
    manifest = json.load(open(MANIFEST, encoding="utf-8"))
    rejected = set()
    if splice_report:
        rejected = {r["entry"] for r in json.load(open(splice_report, encoding="utf-8")).get("rejections", [])}
    problems, infos = [], []
    counts = {"ported": 0, "skipped": 0, "spliced": 0, "not_spliced": 0}
    for e in manifest:
        if e.get("status") == "ported":
            counts["ported"] += 1
            check_entry(e, problems, infos, waived=rejected if gen_root else frozenset())
            if gen_root and not any(p[1] == "%s::%s" % (e["class"], e["function"]) for p in problems):
                check_generated(e, rejected, problems, counts)
        elif e.get("status") == "skipped":
            counts["skipped"] += 1
            check_skipped(e, problems)
        else:
            problems.append(("FAIL", "%s::%s" % (e.get("class"), e.get("function")), "unknown status %r" % e.get("status")))
    fails = [p for p in problems if p[0] == "FAIL"]
    pending = [i for i in infos]
    decl_ok = len(DECLARED)
    if gen_root:
        print("generated tree: %s" % GEN)
        print("ported %d: spliced and verified %d, not spliced (listed as rejected) %d"
              % (counts["ported"], counts["spliced"], counts["not_spliced"]))
        print("FAIL: %d" % len(fails))
        for f in fails:
            print("  FAIL %s: %s" % (f[1], f[2]))
        sys.exit(1 if fails else 0)
    print("manifest entries: %d (ported %d, skipped %d)" % (len(manifest), counts["ported"], counts["skipped"]))
    print("signatures declared in generated header: %d of %d ported" % (decl_ok, counts["ported"]))
    print("PENDING_DECL (generator must add a declaration): %d" % len(pending))
    print("FAIL: %d" % len(fails))
    for f in fails:
        print("  FAIL %s: %s" % (f[1], f[2]))
    if verbose:
        for p in pending:
            print("  PENDING %s: %s" % (p[1], p[2]))
    # Tracked, deterministic counts for the progress dashboard (porting/progress/components.json reads this).
    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "COUNTS.txt"), "w", encoding="utf-8") as fh:
        fh.write("ported: %d\nskipped: %d\ndeclared_in_header: %d\npending_declaration: %d\nfail: %d\n"
                 % (counts["ported"], counts["skipped"], decl_ok, len(pending), len(fails)))
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
