# Ghidra 12.2 script (PyGhidra; GhidraScript globals), read-only over the program.
# @category Fortnite
#
# For each FClassParams block in the reflection JSON, find the code that references it (the
# Z_Construct_UClass_* function), then the function that references that constructor (the
# compiled-in initializer that passes TEXT("ClassName") and the constructor address to
# GetPrivateStaticClassBody). The wide-string literal in that initializer is the class name.
#
# Args: <reflection.json> <out_name_map.json>
#   out_name_map.json  {"0x<block_va>": "ClassName"}, the input format of extract_reflection.py --name-map
#   <stem>.evidence.json  per-block evidence (construct function, initializer, candidates, ambiguity)
#
# Run (PyGhidra, headless, read-only over the program):
#   GHIDRA_INSTALL_DIR=<install> porting/.venv/bin/python -I porting/ghidra/run_export_class_names.py
# See porting/binary_analysis.md ("Runtime dump") for the project and program it expects.

import json
import os
import re

TARGETS = ["FortWorldSettings", "FortGameStateAthena", "FortPlayerStateAthena",
           "FortPlayerControllerAthena", "FortPlayerPawnAthena", "FortAthenaMapInfo",
           "FortPoiManager", "FortWorldManager"]
# UE class names: prefix letter (A, U, F, I, E, T, S) then an uppercase letter, no path separators.
CLASS_NAME = re.compile(r"^[AUFIETS][A-Z][A-Za-z0-9_]{2,80}$")
MAX_CALLERS = 40
MAX_DEPTH = 2


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


def read_wide(addr, limit=120):
    """ASCII text of a NUL-terminated UTF-16LE literal at addr, or None."""
    mem = currentProgram.getMemory()
    chars = []
    for i in range(limit):
        try:
            code = mem.getShort(addr.add(2 * i)) & 0xFFFF
        except Exception:
            return None
        if code == 0:
            return "".join(chars) if chars else None
        if code >= 128 or code < 32:
            return None
        chars.append(chr(code))
    return None


def wide_literals(func):
    """(address, text) for every wide literal referenced from an instruction in func."""
    found = {}
    listing = currentProgram.getListing()
    for inst in listing.getInstructions(func.getBody(), True):
        for ref in inst.getReferencesFrom():
            to = ref.getToAddress()
            if to is None or not currentProgram.getMemory().contains(to):
                continue
            text = read_wide(to)
            if text and CLASS_NAME.match(text):
                found[str(to)] = text
    return found


def callers_of(func):
    """Functions containing a reference to func's entry (direct callers and address-takers)."""
    fm = currentProgram.getFunctionManager()
    rm = currentProgram.getReferenceManager()
    out = []
    seen = set()
    for ref in rm.getReferencesTo(func.getEntryPoint()):
        caller = fm.getFunctionContaining(ref.getFromAddress())
        if caller is None or caller.getEntryPoint() == func.getEntryPoint():
            continue
        key = str(caller.getEntryPoint())
        if key not in seen:
            seen.add(key)
            out.append(caller)
        if len(out) >= MAX_CALLERS:
            break
    return out


def name_for_block(block_va):
    """Return (name or None, evidence dict) for one FClassParams block."""
    fm = currentProgram.getFunctionManager()
    rm = currentProgram.getReferenceManager()
    block = toAddr(block_va)
    evidence = {"block_va": block_va, "construct_fns": [], "candidates": {}, "status": "unresolved"}
    constructs = {}
    for ref in rm.getReferencesTo(block):
        fn = fm.getFunctionContaining(ref.getFromAddress())
        if fn is not None:
            constructs[str(fn.getEntryPoint())] = fn
    if not constructs:
        evidence["status"] = "no_code_reference"
        return None, evidence
    for construct in constructs.values():
        evidence["construct_fns"].append(str(construct.getEntryPoint()))
        frontier = [construct]
        visited = {str(construct.getEntryPoint())}
        for depth in range(MAX_DEPTH + 1):
            names = {}
            for fn in frontier:
                for addr, text in wide_literals(fn).items():
                    names.setdefault(text, []).append({"fn": str(fn.getEntryPoint()), "literal": addr, "depth": depth})
            if names:
                evidence["candidates"].update({k: v for k, v in names.items()})
                if len(names) == 1:
                    text = list(names.keys())[0]
                    evidence["status"] = "named"
                    return text, evidence
                evidence["status"] = "ambiguous"
                return None, evidence
            nxt = []
            for fn in frontier:
                for caller in callers_of(fn):
                    key = str(caller.getEntryPoint())
                    if key not in visited:
                        visited.add(key)
                        nxt.append(caller)
            frontier = nxt
            if not frontier:
                break
    return None, evidence


def main():
    args = _args()
    if len(args) != 2:
        raise SystemExit("usage: export_class_names.py <reflection.json> <out_name_map.json>")
    ref_path, out_path = args
    with open(ref_path, "r", encoding="utf-8") as fh:
        reflection = json.load(fh)
    blocks = [c["block_va"] for c in reflection["classes_in_groups_with_fort_names"]]
    _log(f"naming {len(blocks)} FClassParams blocks from {os.path.basename(ref_path)}")

    name_map = {}
    evidence_list = []
    for i, va in enumerate(blocks):
        name, ev = name_for_block(va)
        evidence_list.append(ev)
        if name:
            name_map[va.lower()] = name
        if i and i % 500 == 0:
            _log(f"  {i}/{len(blocks)} blocks, {len(name_map)} named")

    target_hits = {name: va for va, name in name_map.items() if name in TARGETS}
    statuses = {}
    for ev in evidence_list:
        statuses[ev["status"]] = statuses.get(ev["status"], 0) + 1
    _log(f"named {len(name_map)} of {len(blocks)}; statuses {statuses}")
    _log(f"target hits: {target_hits}")

    with open(out_path, "w", encoding="utf-8") as fh:
        json.dump(dict(sorted(name_map.items())), fh, indent=1)
    stem = os.path.splitext(out_path)[0]
    with open(stem + ".evidence.json", "w", encoding="utf-8") as fh:
        json.dump({"targets": target_hits, "statuses": statuses, "blocks": evidence_list}, fh, indent=1)


main()
