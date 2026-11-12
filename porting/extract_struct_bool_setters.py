#!/usr/bin/env python3
"""Write generated/reflection/struct_bool_setters.json: the bool setter (FBoolPropertyParams::SetBitFunc) of every
bitfield member of a struct, keyed by struct block va and property index.

struct_blocks.json was extracted before set_bit_fn was recorded for struct members, and several generator policies pin
its exact records, so it is left unchanged. Without the setter the byte offset of a struct bitfield is unknown, which
makes class_checksum.py sort struct members wrongly (the real offset decides the order in SortedStructFieldsChecksum).
Re-runs extract_types.py join into a scratch directory and keeps only the set_bit_fn values.
Usage: python3 -I porting/extract_struct_bool_setters.py
"""
import json
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REFL_DIR = os.path.join(HERE, "generated", "reflection")


def main():
    with tempfile.TemporaryDirectory() as tmp:
        subprocess.run([sys.executable, "-I", os.path.join(HERE, "extract_types.py"), "join", "--out-dir", tmp],
                       check=True, stdout=subprocess.DEVNULL)
        with open(os.path.join(tmp, "struct_blocks.json"), encoding="utf-8") as fh:
            fresh = json.load(fh)["structs"]
    with open(os.path.join(REFL_DIR, "struct_blocks.json"), encoding="utf-8") as fh:
        current = {s["block_va"].lower(): s for s in json.load(fh)["structs"]}
    out = {}
    for s in fresh:
        old = current.get(s["block_va"].lower())
        if old is None or len(old["properties"]) != len(s["properties"]):
            raise SystemExit("struct %s differs from struct_blocks.json" % s["name"])
        for a, b in zip(old["properties"], s["properties"]):
            if a.get("name") != b.get("name"):
                raise SystemExit("struct %s property order differs" % s["name"])
            if b.get("set_bit_fn"):
                out.setdefault(s["block_va"].lower(), {})[str(b["index"])] = b["set_bit_fn"]
    path = os.path.join(REFL_DIR, "struct_bool_setters.json")
    with open(path, "w", encoding="utf-8") as fh:
        json.dump({"setters": out}, fh, indent=1, sort_keys=True)
    print("%d structs, %d setters -> %s" % (len(out), sum(len(v) for v in out.values()), path))


if __name__ == "__main__":
    main()
