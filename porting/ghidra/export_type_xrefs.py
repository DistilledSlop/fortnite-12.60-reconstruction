# Ghidra 12.2 script (PyGhidra; GhidraScript globals), read-only over the program.
# @category Fortnite
#
# For each candidate UHT parameter block (FStructParams, FEnumParams, FFunctionParams, FClassParams),
# list the instruction addresses that reference it. The construct function that owns a reference is
# found by the caller from the .pdata function table (porting/extract_types.py), so this script only
# needs the reference sites. Used by porting/extract_types.py (phase 2).
#
# Args: <candidates.json> <out_xrefs.json>
#   candidates.json  {"blocks": ["0x<block_va>", ...]}
#   out_xrefs.json   {"0x<block_va>": ["0x<from_va>", ...]}, sorted, unique reference sites
#
# Run through porting/ghidra/run_export_type_xrefs.py (PyGhidra, headless, no analysis run).

import json


def _args():
    try:
        return list(getScriptArgs())  # GhidraScript global
    except NameError:
        import sys
        return sys.argv[1:]


def _log(msg):
    try:
        println(msg)  # GhidraScript global
    except NameError:
        print(msg, flush=True)


def main():
    args = _args()
    if len(args) != 2:
        raise SystemExit("usage: export_type_xrefs.py <candidates.json> <out_xrefs.json>")
    cand_path, out_path = args
    with open(cand_path, "r", encoding="utf-8") as fh:
        blocks = json.load(fh)["blocks"]
    _log("xrefs for %d blocks" % len(blocks))
    rm = currentProgram.getReferenceManager()
    mem = currentProgram.getMemory()
    out = {}
    total = 0
    for i, va in enumerate(blocks):
        addr = toAddr(va)
        sites = set()
        if addr is not None and mem.contains(addr):
            for ref in rm.getReferencesTo(addr):
                sites.add("0x%x" % ref.getFromAddress().getOffset())
        out[va.lower()] = sorted(sites)
        total += len(sites)
        if i and i % 5000 == 0:
            _log("  %d/%d blocks, %d sites" % (i, len(blocks), total))
    _log("done: %d blocks, %d reference sites, %d blocks without sites" % (
        len(blocks), total, sum(1 for v in out.values() if not v)))
    with open(out_path, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(out, fh)
        fh.write("\n")


main()
