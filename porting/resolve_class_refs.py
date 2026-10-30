#!/usr/bin/env python3
"""Map class-typed property and parameter references to the class block they name (stdlib only).

The reflection tables store, for an Object/Class/Interface reference, the address of a class function
(UHT's ClassFunc). That function is usually a `jmp rel32` thunk to the class's Z_Construct_UClass_X_NoRegister
(or to Z_Construct_UClass_X itself). This script follows up to four jmp hops in the runtime dump, then matches
the final address against each block's class_noregister_fn and the Z_Construct functions in
class_name_map.evidence.json. Code bytes come from the runtime dump, because the shipping exe is encrypted on disk.

Input:  reflection JSON, class_name_map.evidence.json, the runtime dump (porting/generated/dump/...runtime.exe)
Output: {"resolved": {ref_fn: block_va}, "unresolved": [ref_fn, ...], "summary": {...}} (default
        porting/generated/reflection/class_ref_map.json, git-ignored). Used by promote_native_classes.sh.

Usage: python3 -I porting/resolve_class_refs.py [--reflection JSON] [--evidence JSON] [--image EXE] [--out JSON]
"""
import argparse
import collections
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import extract_reflection as ER  # noqa: E402  (Image reader shared with the extractor)

REFLECTION = os.path.join(HERE, "generated", "reflection", "fortnite_game_reflection.json")
EVIDENCE = os.path.join(HERE, "generated", "reflection", "class_name_map.evidence.json")
IMAGE = os.path.join(HERE, "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")
OUT = os.path.join(HERE, "generated", "reflection", "class_ref_map.json")
CLASS_REF_KEYS = ("class", "meta_class", "interface_class")


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


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--reflection", default=REFLECTION)
    ap.add_argument("--evidence", default=EVIDENCE)
    ap.add_argument("--image", default=IMAGE)
    ap.add_argument("--out", default=OUT)
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

    img = ER.Image(args.image)
    resolved, unresolved = {}, []
    hits = collections.Counter()
    for ref in sorted(refs):
        target = follow_thunks(img, int(ref, 16))
        block = targets.get("0x%x" % target) if target else None
        if block:
            resolved[ref] = block
            hits["noregister_or_construct"] += 1
        else:
            unresolved.append(ref)
    out = {
        "method": "follow up to 4 jmp thunks in the runtime dump; match class_noregister_fn or Z_Construct",
        "resolved": resolved,
        "unresolved": unresolved,
        "summary": {"distinct_class_refs": len(refs), "resolved": len(resolved), "unresolved": len(unresolved)},
    }
    with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(out, fh, indent=1)
        fh.write("\n")
    print(json.dumps(out["summary"]), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
