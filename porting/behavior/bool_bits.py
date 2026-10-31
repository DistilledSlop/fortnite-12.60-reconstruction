#!/usr/bin/env python3
"""Maps every bool property of the reflection to its (byte offset, bit mask) from its SetBitFunc code.

Uses porting/class_checksum.py (read only, imported) to decode each setter in the runtime dump
(bool_setter_offset). The setter is `or byte ptr [rcx+disp], mask; ret`, so mask is the immediate
byte that is OR-ed in. A `mov byte ptr [rcx+disp], imm` setter is recorded with form "mov".

Output: porting/generated/reflection/bool_bits.json (git-ignored):
  {"_meta": {...}, "by_class": {"<ClassName>": [{"name", "offset", "mask", "form", "bitfield"}, ...]}}
Only the class's own properties are listed (the reflection block of each class).

Usage: python3 -I porting/behavior/bool_bits.py
"""
import collections
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORTING = os.path.dirname(HERE)
sys.path.insert(0, PORTING)
import class_checksum as CC  # noqa: E402  (read only)

OUT = os.path.join(PORTING, "generated", "reflection", "bool_bits.json")


def setter_form(img, va):
    off = img.off(va)
    if off is None:
        return None
    return {0x80: "or8", 0x81: "or32", 0x83: "or32", 0xC6: "mov8", 0xC7: "mov32"}.get(img.m[off])


def main():
    model = CC.Model(CC.REFL, CC.IMAGE)
    img = model.image()
    by_class = collections.defaultdict(list)
    stats = collections.Counter()
    for va, block in model.blocks.items():
        name = model.names.get(va)
        if not name:
            continue
        for prop in block.get("properties") or []:
            if prop.get("kind") != "Bool" or not prop.get("set_bit_fn"):
                continue
            stats["bool_total"] += 1
            setter = int(str(prop["set_bit_fn"]), 16)
            found = CC.bool_setter_offset(img, setter)
            if found is None:
                stats["undecoded"] += 1
                continue
            off, mask = found
            form = setter_form(img, setter)
            bitfield = bool(prop.get("bitfield"))
            if bitfield and bin(mask).count("1") != 1:
                stats["bitfield_multibit_mask"] += 1
            stats["decoded"] += 1
            by_class[name].append({"name": prop["name"], "offset": off, "mask": mask, "form": form,
                                   "bitfield": bitfield})
    out = {"_meta": {"source": "porting/class_checksum.py bool_setter_offset (runtime dump)",
                     "stats": dict(stats), "note": "mask is the immediate OR-ed by the setter"},
           "by_class": dict(sorted(by_class.items()))}
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fh:
        json.dump(out, fh, indent=1)
    print("bool properties %d, decoded %d, undecoded %d; classes %d -> %s" % (
        stats["bool_total"], stats["decoded"], stats["undecoded"], len(by_class), OUT))
    if stats["bitfield_multibit_mask"]:
        print("warning: %d bitfields with a multi-bit mask" % stats["bitfield_multibit_mask"])


if __name__ == "__main__":
    main()
