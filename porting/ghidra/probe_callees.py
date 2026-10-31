#!/usr/bin/env python3
"""Probe unidentified callees in the slim Ghidra project (PyGhidra, read-only, rolled back).

Opens project fortnite-slim exactly as improve_decompile.py does and applies the same edits (helper names,
FFrame, class structs, vtable slots, prototypes) inside ONE transaction that is rolled back. For every address in
--input it writes, under --out (git-ignored porting/generated/...):
  <addr>.c      raw decompile after the edits (C pseudocode)
  <addr>.asm    disassembly of the body (one instruction per line, capped)
  <addr>.json   name, size, instruction count, direct callees (with names), indirect call count, string
                literals, callers (call / jump / data references, with the containing function)
  index.json    one summary row per address

Usage (repo root):
  porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input IN.json --out DIR [--timeout 60] [--mem 32G]
IN.json is a list of {"address": "0x...", "label": "..."} (or plain hex strings).
Environment: GHIDRA_INSTALL_DIR, FN_GHIDRA_PROJECT_DIR, FN_GHIDRA_PROJECT, FN_GHIDRA_PROGRAM_PATH.
"""
import argparse
import collections
import datetime
import json
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import decompile_targets as base  # noqa: E402
import improve_decompile as imp  # noqa: E402

MAX_ASM_LINES = 700
MAX_CALLERS = 80


def _log(msg):
    print("[probe] " + msg, file=sys.stderr, flush=True)


def load_input(path):
    with open(path, encoding="utf-8") as fh:
        data = json.load(fh)
    out, seen = [], set()
    for item in data:
        if isinstance(item, str):
            item = {"address": item}
        addr = int(item["address"], 16)
        if addr in seen:
            continue
        seen.add(addr)
        out.append({"address": hex(addr), "label": item.get("label", "")})
    return out


def callers_of(program, fm, entry):
    rm = program.getReferenceManager()
    found = collections.OrderedDict()
    counts = collections.Counter()
    for r in rm.getReferencesTo(entry):
        rt = r.getReferenceType()
        kind = "call" if rt.isCall() else ("jump" if rt.isJump() else ("data" if rt.isData() else "other"))
        counts[kind] += 1
        frm = r.getFromAddress()
        cf = fm.getFunctionContaining(frm)
        if cf is not None:
            key = (hex(int(cf.getEntryPoint().getOffset())), cf.getName(), kind)
        else:
            key = (hex(int(frm.getOffset())), None, kind)
        if key not in found and len(found) < MAX_CALLERS:
            found[key] = True
    rows = [{"address": a, "name": n, "ref": k} for (a, n, k) in found.keys()]
    return rows, dict(counts)


def disasm_lines(program, func):
    listing = program.getListing()
    lines = []
    for ins in listing.getInstructions(func.getBody(), True):
        if len(lines) >= MAX_ASM_LINES:
            lines.append("... (capped at %d)" % MAX_ASM_LINES)
            break
        lines.append("%s  %s" % (ins.getAddress(), ins.toString()))
    return lines


def decompile(decomp, func, timeout):
    from ghidra.util.task import TaskMonitor
    res = decomp.decompileFunction(func, timeout, TaskMonitor.DUMMY)
    if res is None:
        return "failed", "no result"
    if res.isTimedOut():
        return "timeout", str(res.getErrorMessage())
    if not res.decompileCompleted():
        return "failed", str(res.getErrorMessage())
    return "ok", str(res.getDecompiledFunction().getC())


def main(argv=None):
    ap = argparse.ArgumentParser(description="Probe callee functions (see module doc)")
    ap.add_argument("--input", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--timeout", type=int, default=60)
    ap.add_argument("--mem", default=os.environ.get("GHIDRA_HEADLESS_MAXMEM", "32G"))
    args = ap.parse_args(argv if argv is not None else sys.argv[1:])
    out_root = os.path.abspath(args.out)
    os.makedirs(out_root, exist_ok=True)
    items = load_input(args.input)
    t0 = time.time()

    targets_all = base.load_targets()
    natives = imp.load_json(os.path.join(imp.REFL, "native_funcs.json"))
    struct_index = imp.struct_index_of(imp.load_json(os.path.join(imp.REFL, "struct_blocks.json")))
    imp._STRUCT_INDEX[:] = [struct_index]
    layouts = imp.load_json(os.path.join(imp.TABLE_DIR, "struct_layouts.json"))["classes"]
    helpers = imp.load_json(os.path.join(imp.TABLE_DIR, "helpers.json"))

    import pyghidra
    from pyghidra.launcher import HeadlessPyGhidraLauncher
    launcher = HeadlessPyGhidraLauncher(install_dir=imp.INSTALL)
    launcher.add_vmargs("-Xmx%s" % args.mem)
    launcher.start()
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: E402
    from ghidra.app.decompiler import DecompInterface, DecompileOptions
    from ghidra.program.model.data import StringDataInstance

    project_root = os.path.join(imp.PROJECT_DIR, imp.PROJECT + ".rep")
    project_gpr = os.path.join(imp.PROJECT_DIR, imp.PROJECT + ".gpr")
    before = base.snapshot([project_root, project_gpr])
    project = pyghidra.open_project(imp.PROJECT_DIR, imp.PROJECT, create=False)
    program, consumer = pyghidra.consume_program(project, imp.PROGRAM_PATH)
    tx = program.startTransaction("probe_callees in-memory (rolled back)")
    index_rows = []
    try:
        flat = FlatProgramAPI(program)
        ed, summary, impl, slot_info = imp.run_edits(program, flat, targets_all, natives, struct_index, layouts,
                                                     helpers)
        fm = program.getFunctionManager()
        opts = DecompileOptions()
        opts.grabFromProgram(program)
        decomp = DecompInterface()
        decomp.setOptions(opts)
        decomp.toggleCCode(True)
        decomp.setSimplificationStyle("decompile")
        if not decomp.openProgram(program):
            raise SystemExit("DecompInterface.openProgram failed: " + str(decomp.getLastMessage()))
        for it in items:
            addr = it["address"]
            f, made = imp.ensure_function(ed, addr)
            row = {"address": addr, "label": it["label"], "created_in_memory": bool(made)}
            if f is None:
                row.update({"status": "no function"})
                index_rows.append(row)
                continue
            status, code = decompile(decomp, f, args.timeout)
            called, indirect, strings = base.analyze_body(program, f, StringDataInstance)
            callers, ref_counts = callers_of(program, fm, f.getEntryPoint())
            asm = disasm_lines(program, f)
            n_ins = sum(1 for _ in program.getListing().getInstructions(f.getBody(), True))
            detail = {
                "address": addr, "label": it["label"], "name": f.getName(), "status": status,
                "body_bytes": int(f.getBody().getNumAddresses()), "instructions": n_ins,
                "called": called, "indirect_calls": indirect, "strings": strings, "callers": callers,
                "reference_counts": ref_counts, "thunk": bool(f.isThunk()),
                "error": None if status == "ok" else code,
            }
            stem = os.path.join(out_root, addr)
            with open(stem + ".json", "w", encoding="utf-8") as fh:
                json.dump(detail, fh, indent=1)
            if status == "ok":
                with open(stem + ".c", "w", encoding="utf-8") as fh:
                    fh.write(code)
            with open(stem + ".asm", "w", encoding="utf-8") as fh:
                fh.write("\n".join(asm) + "\n")
            row.update({"status": status, "name": f.getName(), "body_bytes": detail["body_bytes"],
                        "instructions": n_ins, "direct_callees": len(called), "indirect_calls": indirect,
                        "callers": len(callers), "ref_counts": ref_counts})
            index_rows.append(row)
        decomp.dispose()
    finally:
        program.endTransaction(tx, False)  # roll back: nothing is committed
        program.release(consumer)
        project.close()
    changed = base.snapshot_diff(before, base.snapshot([project_root, project_gpr]))
    with open(os.path.join(out_root, "index.json"), "w", encoding="utf-8") as fh:
        json.dump({"_meta": {"generated": datetime.datetime.now().isoformat(timespec="seconds"),
                             "elapsed_s": round(time.time() - t0, 1), "count": len(index_rows),
                             "rolled_back": True, "project_files_changed": changed},
                   "functions": index_rows}, fh, indent=1)
    _log("probed %d addresses in %.1f s; project files changed: %s" % (len(index_rows), time.time() - t0, changed))
    return 0


if __name__ == "__main__":
    sys.exit(main())
