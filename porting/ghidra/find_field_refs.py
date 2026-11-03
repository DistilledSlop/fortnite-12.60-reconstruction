#!/usr/bin/env python3
"""Find instructions that store to [reg+DISP] near a given code address (PyGhidra, read-only).

Usage (repo root): porting/.venv/bin/python -I porting/ghidra/find_field_refs.py DISP@NEAR [DISP@NEAR ...]
                   [--window 0x40000] [--out FILE] [--all-access]
  DISP@NEAR  e.g. 0x290@0x144e0c9c0 : scan [NEAR-window, NEAR+window] for memory operands with that
             displacement. Default reports stores (destination operand is the memory operand);
             --all-access also reports reads and LEAs.
Output: JSON {"DISP@NEAR": [{"addr", "function", "text"}]} (function = containing function entry).
The program is never saved. Same project/program settings as decompile_targets.py.
"""
import argparse
import json
import os
import re
import sys

PROJECT_DIR = os.environ.get("FN_GHIDRA_PROJECT_DIR", "/opt/fortnite/ghidra-projects")
PROJECT = os.environ.get("FN_GHIDRA_PROJECT", "fortnite-slim")
PROGRAM_PATH = os.environ.get("FN_GHIDRA_PROGRAM_PATH", "/FortniteClient-Win64-Shipping.runtime.exe")
INSTALL = os.environ.get("GHIDRA_INSTALL_DIR", "/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC")
MEM = re.compile(r"\[[^\]]*?\+\s*0x([0-9a-fA-F]+)\]")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("targets", nargs="+")
    ap.add_argument("--window", default="0x40000")
    ap.add_argument("--out", default="")
    ap.add_argument("--all-access", action="store_true")
    ap.add_argument("--mem", default="8G")
    ap.add_argument("--call-prefix", default="", help="only report hits followed by a CALL to an address starting with this hex prefix (e.g. 0x142ee)")
    ap.add_argument("--call-within", type=int, default=6)
    args = ap.parse_args()
    import pyghidra
    from pyghidra.launcher import HeadlessPyGhidraLauncher
    launcher = HeadlessPyGhidraLauncher(install_dir=INSTALL)
    launcher.add_vmargs("-Xmx" + args.mem)
    launcher.start()
    project = pyghidra.open_project(PROJECT_DIR, PROJECT, create=False)
    program, consumer = pyghidra.consume_program(project, PROGRAM_PATH)
    result = {}
    try:
        space = program.getAddressFactory().getDefaultAddressSpace()
        listing = program.getListing()
        fm = program.getFunctionManager()
        window = int(args.window, 16)
        for target in args.targets:
            disp_s, near_s = target.split("@")
            disp = int(disp_s, 16)
            near = int(near_s, 16)
            lo, hi = max(0, near - window), near + window
            rows = []
            it = listing.getInstructions(space.getAddress(lo), True)
            for ins in it:
                a = ins.getAddress().getOffset()
                if a > hi:
                    break
                text = str(ins)
                for m in MEM.finditer(text):
                    if int(m.group(1), 16) != disp:
                        continue
                    first = text.split(",")[0]
                    is_store = "[" in first and m.start() < len(first) + 1 and text.find(m.group(0)) <= len(first)
                    if args.call_prefix:
                        nxt, hit, n = ins.getNext(), False, 0
                        while nxt is not None and n < args.call_within:
                            if str(nxt).startswith("CALL") and ("0x" + format(nxt.getFlows()[0].getOffset(), "x") if nxt.getFlows() else "").startswith(args.call_prefix):
                                hit = True
                                break
                            nxt, n = nxt.getNext(), n + 1
                        if not hit:
                            break
                    if args.all_access or is_store:
                        fn = fm.getFunctionContaining(ins.getAddress())
                        rows.append({"addr": hex(a), "function": hex(fn.getEntryPoint().getOffset()) if fn else None,
                                     "text": text})
                    break
            result[target] = rows
            print(target, len(rows), file=sys.stderr)
    finally:
        program.release(consumer)
        project.close()
    out = json.dumps(result, indent=1)
    if args.out:
        with open(args.out, "w") as fh:
            fh.write(out + "\n")
    else:
        print(out)


if __name__ == "__main__":
    main()
