#!/usr/bin/env python3
"""Map class-typed property and parameter references to the class block they name.

The reflection tables store, for an Object/Class/Interface reference, the address of a class function
(UHT's ClassFunc). Two kinds of target are handled:

1. Rule `noregister_or_construct`: the function is a `jmp rel32` thunk (up to four hops) to the class's
   Z_Construct_UClass_X_NoRegister (or to Z_Construct_UClass_X itself). The final address is matched against
   each block's class_noregister_fn and the Z_Construct functions in class_name_map.evidence.json.

2. Rule `builder_literal_name_package`: the function is a singleton builder that does not reference the
   FClassParams block at all. It calls GetPrivateStaticClassBody directly with its arguments built inline:
   rcx = the package literal (L"/Script/CommonUI"), rdx = the UObject name literal without the C++ prefix
   (L"CommonTextBlock"), r8 = the static UClass* slot, and the other parameters on the stack. The call is
   matched by (package, name) against the blocks: the block whose package (class_parent_map.json) and
   name (class_name_map.json, prefix letter removed) are equal. A match is accepted only when it is unique.
   Verified offline on the 287 builders of this kind: every name matched one block with the same package,
   no block name is duplicated, and the super-class function stored by each builder equals the parent
   block's accessor (286 of 287; the 287th is UObject, the root, which has no parent).

Refs that match neither rule are reported with a reason. The remaining ones are hand-built CoreUObject
intrinsic singletons (no name literal and no FClassParams block), see `unresolved_reasons`.

Code bytes come from the runtime dump, because the shipping exe is encrypted on disk. The builder rule disassembles
the builder with objdump (binutils), as dump_runtime_image.py does.

Input:  reflection JSON, class_name_map.evidence.json, class_name_map.json, class_parent_map.json, the runtime dump
Refs are collected from the class tables and from the struct, delegate and UFunction parameter tables.
Output: {"resolved": {ref_fn: block_va}, "unresolved": [ref_fn, ...], "resolved_by": {ref_fn: rule},
         "builder_names": {ref_fn: {package, name, block_va}}, "unresolved_reasons": {...}, "summary": {...}}
        (default porting/generated/reflection/class_ref_map.json, git-ignored). Used by promote_native_classes.sh.
        `resolved` keeps its meaning, so gen_native_classes.py reads the new rule without change.

Usage: python3 -I porting/resolve_class_refs.py [--reflection JSON] [--evidence JSON] [--names JSON]
                                               [--parents JSON] [--image EXE] [--out JSON] [--no-builder]
"""
import argparse
import bisect
import collections
import json
import os
import re
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import extract_reflection as ER  # noqa: E402  (Image reader shared with the extractor)
import extract_types as ET  # noqa: E402  (.pdata function ranges)

REFL_DIR = os.path.join(HERE, "generated", "reflection")
REFLECTION = os.path.join(REFL_DIR, "fortnite_game_reflection.json")
EVIDENCE = os.path.join(REFL_DIR, "class_name_map.evidence.json")
NAMES = os.path.join(REFL_DIR, "class_name_map.json")
PARENTS = os.path.join(REFL_DIR, "class_parent_map.json")
IMAGE = os.path.join(HERE, "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")
OUT = os.path.join(REFL_DIR, "class_ref_map.json")
CLASS_REF_KEYS = ("class", "meta_class", "interface_class")
PREFIX_LETTERS = "AUFIETS"
BUILDER_MAX_BYTES = 0x400          # a builder is under 0x200 bytes; the cap bounds a missing .pdata entry
PKG_LITERAL = re.compile(r"^/Script/[A-Za-z0-9_]+$")
NAME_LITERAL = re.compile(r"^[A-Za-z_][A-Za-z0-9_]{1,80}$")
RIP_TARGET = re.compile(r"\[rip\+0x[0-9a-f]+\]\s*#\s*0x([0-9a-f]+)")
REG_WRITE = re.compile(r"^(\w+)\s+(\w+),")
CALL_TARGET = re.compile(r"^call\s+0x([0-9a-f]+)$")


def follow_thunks(img, va, hops=4):
    for _ in range(hops):
        off = img.off(va)
        if off is None:
            return None
        head = img.m[off:off + 6]
        if head[0] == 0xE9:                       # jmp rel32
            va = va + 5 + struct.unpack_from("<i", head, 1)[0]
        elif head[:2] == b"\xff\x25":             # jmp [rip+disp32]
            va = img.u64(va + 6 + struct.unpack_from("<i", head, 2)[0])
            if not va:
                return None
        else:
            break
    return va


def collect_refs(params, out):
    for param in params or []:
        for key, value in (param.get("type_refs") or {}).items():
            if key in CLASS_REF_KEYS and value:
                out.add(value.lower())
        collect_refs(param.get("params"), out)


def read_wide(img, va, limit=160):
    """UTF-16LE NUL-terminated ASCII literal at va, or None."""
    off = img.off(va)
    if off is None:
        return None
    chars = []
    for i in range(limit):
        if off + 2 * i + 2 > len(img.m):
            return None
        code = img.m[off + 2 * i] | (img.m[off + 2 * i + 1] << 8)
        if code == 0:
            return "".join(chars) if chars else None
        if code < 32 or code > 126:
            return None
        chars.append(chr(code))
    return None


def disassemble(img, start, length):
    """objdump lines (normalised, lowercase mnemonic and operands) for [start, start+length)."""
    off = img.off(start)
    if off is None:
        return []
    data = img.m[off:off + length]
    with tempfile.TemporaryDirectory(prefix="resolve_class_refs_") as tmp:
        path = os.path.join(tmp, "fn.bin")
        with open(path, "wb") as fh:
            fh.write(data)
        proc = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386:x86-64", "-M", "intel",
                               "--adjust-vma=0x%x" % start, path],
                              capture_output=True, text=True, check=False)
    lines = []
    for raw in proc.stdout.splitlines():
        m = re.match(r"\s+([0-9a-f]+):\t[0-9a-f ]+\t(.*)$", raw)
        if m:
            lines.append((int(m.group(1), 16), re.sub(r"\s+", " ", m.group(2)).strip()))
    return lines


def builder_registration(img, funcs, va):
    """(package, name, helper_va, call_va) of the GetPrivateStaticClassBody-style call in the builder at va.

    Walks the disassembly of the .pdata function that contains va. rcx and rdx are tracked as the value of the
    last instruction that wrote them (a RIP-relative LEA gives a literal address). The last call that gets a
    "/Script/..." package literal in rcx and a bare name literal in rdx is the registration. Other calls clobber
    rcx and rdx. Returns None when there is no such call.
    """
    start = funcs.start_of(va)
    if start is None:
        start = va
    rva = start - img.imagebase
    i = bisect.bisect_left(funcs.begins, rva)
    end = img.imagebase + funcs.ends[i] if i < len(funcs.begins) and funcs.begins[i] == rva else None
    length = min(BUILDER_MAX_BYTES, (end - start) if end and end > start else BUILDER_MAX_BYTES)
    found = None
    regs = {"rcx": None, "rdx": None}
    for addr, text in disassemble(img, start, length):
        m = CALL_TARGET.match(text)
        if m:
            helper = int(m.group(1), 16)
            pkg = read_wide(img, regs["rcx"]) if regs["rcx"] else None
            name = read_wide(img, regs["rdx"]) if regs["rdx"] else None
            if pkg and name and PKG_LITERAL.match(pkg) and NAME_LITERAL.match(name):
                found = (pkg, name, helper, addr)
            regs = {"rcx": None, "rdx": None}
            continue
        m = REG_WRITE.match(text)
        if not m:
            continue
        op, dst = m.group(1), m.group(2)
        key = {"rcx": "rcx", "ecx": "rcx", "cl": "rcx", "rdx": "rdx", "edx": "rdx", "dl": "rdx"}.get(dst)
        if key is None:
            continue
        target = RIP_TARGET.search(text)
        # Only a 64-bit LEA into rcx/rdx keeps a literal address; any other write clears the register.
        regs[key] = int(target.group(1), 16) if (op == "lea" and target and dst in ("rcx", "rdx")) else None
    return found


def load_block_index(names_path, parents_path):
    """{(package, name_without_prefix): [block_va]} from the name map and the parent map."""
    with open(names_path, "r", encoding="utf-8") as fh:
        names = json.load(fh)
    with open(parents_path, "r", encoding="utf-8") as fh:
        parents = json.load(fh)
    index = collections.defaultdict(list)
    for va, name in names.items():
        package = (parents.get(va) or parents.get(va.lower()) or {}).get("package")
        if not package or not name or name[0] not in PREFIX_LETTERS:
            continue
        index[(package, name[1:])].append(va.lower())
    return index


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--reflection", default=REFLECTION)
    ap.add_argument("--evidence", default=EVIDENCE)
    ap.add_argument("--names", default=NAMES, help="class_name_map.json {block_va: prefixed name}")
    ap.add_argument("--parents", default=PARENTS, help="class_parent_map.json {block_va: {package, ...}}")
    ap.add_argument("--image", default=IMAGE)
    ap.add_argument("--out", default=OUT)
    ap.add_argument("--no-builder", action="store_true", help="thunk rule only (the previous behaviour)")
    args = ap.parse_args(argv)

    with open(args.reflection, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    with open(args.evidence, "r", encoding="utf-8") as fh:
        evidence = json.load(fh)
    blocks = data.get("classes_in_groups_with_fort_names") or data.get("classes") or []

    targets = {}
    for block in blocks:
        if block.get("class_noregister_fn"):
            targets[block["class_noregister_fn"].lower()] = block["block_va"].lower()
    for rec in evidence.get("blocks", []):
        for fn in rec.get("construct_fns") or []:
            fn = str(fn).lower()
            targets.setdefault(fn if fn.startswith("0x") else "0x" + fn, rec["block_va"].lower())

    refs = set()
    for block in blocks:
        collect_refs(block.get("properties"), refs)
        for func in block.get("functions") or []:
            collect_refs(func.get("params"), refs)
            for cand in func.get("candidates") or []:
                collect_refs(cand.get("params"), refs)
    # Class references inside struct members, delegate signatures and UFunction parameter blocks (all tables
    # written by extract_types.py). The checksum walk (class_checksum.py) needs them as well.
    table_dir = os.path.dirname(os.path.abspath(args.evidence))
    for fname, key, field in (("struct_blocks.json", "structs", "properties"),
                              ("delegate_blocks.json", "delegates", "params"),
                              ("function_blocks.json", "functions", "params")):
        path = os.path.join(table_dir, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r", encoding="utf-8") as fh:
            for rec in json.load(fh).get(key, []):
                collect_refs(rec.get(field), refs)

    img = ER.Image(args.image)
    funcs = ET.Functions(img) if not args.no_builder else None
    index = load_block_index(args.names, args.parents) if not args.no_builder else {}

    resolved, unresolved, resolved_by = {}, [], {}
    builder_names, reasons = {}, {}
    helpers = collections.Counter()
    for ref in sorted(refs):
        target = follow_thunks(img, int(ref, 16))
        block = targets.get("0x%x" % target) if target else None
        if block:
            resolved[ref] = block
            resolved_by[ref] = "noregister_or_construct"
            continue
        if args.no_builder:
            unresolved.append(ref)
            continue
        reg = builder_registration(img, funcs, target) if target else None
        if reg is None:
            unresolved.append(ref)
            reasons[ref] = "no_name_literal_or_fclassparams_block"
            continue
        pkg, name, helper, call_va = reg
        helpers["0x%x" % helper] += 1
        matches = index.get((pkg, name), [])
        if len(matches) == 1:
            resolved[ref] = matches[0]
            resolved_by[ref] = "builder_literal_name_package"
            builder_names[ref] = {"package": pkg, "name": name, "block_va": matches[0],
                                  "helper": "0x%x" % helper, "call_site": "0x%x" % call_va}
        else:
            unresolved.append(ref)
            reasons[ref] = ("builder_name_no_block" if not matches else "builder_name_ambiguous") + \
                           " (%s %s)" % (pkg, name)
    by_rule = collections.Counter(resolved_by.values())
    out = {
        "method": ("(1) follow up to 4 jmp thunks in the runtime dump; match class_noregister_fn or Z_Construct. "
                   "(2) for the rest: singleton builder that calls GetPrivateStaticClassBody with inline "
                   "rcx=/Script/<package> and rdx=<name>; match the unique block with that package and name."),
        "resolved": resolved,
        "unresolved": unresolved,
        "resolved_by": resolved_by,
        "builder_names": builder_names,
        "unresolved_reasons": reasons,
        "summary": {"distinct_class_refs": len(refs), "resolved": len(resolved), "unresolved": len(unresolved),
                    "by_rule": dict(by_rule), "builder_helpers": dict(helpers)},
    }
    with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(out, fh, indent=1)
        fh.write("\n")
    print(json.dumps(out["summary"]), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
