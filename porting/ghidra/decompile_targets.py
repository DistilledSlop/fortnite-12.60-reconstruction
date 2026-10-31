#!/usr/bin/env python3
"""Decompile the ranked Athena gameplay targets from the slim Ghidra project (PyGhidra, read-only).

Input: porting/generated/reflection/decompile_targets.json (from porting/extract_native_funcs.py,
section 10 of porting/binary_analysis.md). Each target has an exec address, flags and net roles.

Opens project fortnite-slim (default /opt/fortnite/ghidra-projects) through PyGhidra, the same way as
porting/ghidra/run_export_*.py: open_project() with the project locator (no nested project location),
then consume_program() for the program. The program is never saved: the run takes a snapshot of the
project files before and after, and reports whether anything on disk changed. Missing functions
(--create-missing) and symbol names (--rename) are applied inside one transaction that is rolled
back (endTransaction(tx, False)) at the end, so nothing is committed.

For each target function, DecompInterface (per-function timeout, default 30 s) produces raw C
pseudocode. Output (git-ignored):
  porting/generated/decomp/<Class>/<Function>.c   header comment + C pseudocode
  porting/generated/decomp/index.json             per-function status, line count, called addresses,
                                                   referenced string literals; "_meta" with run totals
Records are merged by address into an existing index, so tier runs accumulate (use --no-index for
ad-hoc runs, e.g. --only with --out DIR).

Usage (run from the repo root; needs porting/.venv with jpype1 and Ghidra's pypkg):
  porting/.venv/bin/python -I porting/ghidra/decompile_targets.py [--tiers 1,2] [--max-seconds 5400]
      [--timeout 30] [--limit N] [--only 0xADDR[:Class.Function] ...] [--out DIR] [--no-index]
      [--create-missing] [--rename] [--mem 32G]
Environment: GHIDRA_INSTALL_DIR (default /opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC),
FN_GHIDRA_PROJECT_DIR, FN_GHIDRA_PROJECT, FN_GHIDRA_PROGRAM_PATH, GHIDRA_HEADLESS_MAXMEM (heap).
"""
import argparse
import collections
import datetime
import json
import os
import re
import sys
import time

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GENERATED = os.path.join(REPO, "porting", "generated")
TARGETS_JSON = os.path.join(GENERATED, "reflection", "decompile_targets.json")
NATIVE_JSON = os.path.join(GENERATED, "reflection", "native_funcs.json")
DEFAULT_OUT = os.path.join(GENERATED, "decomp")
PROJECT_DIR = os.environ.get("FN_GHIDRA_PROJECT_DIR", "/opt/fortnite/ghidra-projects")
PROJECT = os.environ.get("FN_GHIDRA_PROJECT", "fortnite-slim")
PROGRAM_PATH = os.environ.get("FN_GHIDRA_PROGRAM_PATH", "/FortniteClient-Win64-Shipping.runtime.exe")
INSTALL = os.environ.get("GHIDRA_INSTALL_DIR", "/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC")

# UE 4.25 EFunctionFlags (Object.h), the bits that appear in the target flags.
FUNC_FLAGS = [
    (0x1, "Final"), (0x40, "Net"), (0x80, "NetReliable"), (0x100, "NetRequest"), (0x200, "Exec"),
    (0x400, "Native"), (0x800, "Event"), (0x1000, "NetResponse"), (0x2000, "Static"),
    (0x4000, "NetMulticast"), (0x20000, "Public"), (0x40000, "Private"), (0x80000, "Protected"),
    (0x200000, "NetServer"), (0x1000000, "NetClient"), (0x4000000, "BlueprintCallable"),
    (0x8000000, "BlueprintEvent"), (0x10000000, "BlueprintPure"), (0x40000000, "Const"),
]
TIER_NAMES = {1: "FUNC_Net (RPC / replicated)", 2: "FUNC_Native + FUNC_BlueprintEvent",
              3: "FUNC_BlueprintCallable with exec", 4: "other native with exec",
              5: "no exec address"}
MAX_STRINGS = 80
MAX_CALLED = 400


def _log(msg):
    print(msg, flush=True)


def decode_flags(value):
    v = int(value, 16) if isinstance(value, str) else int(value)
    return [name for bit, name in FUNC_FLAGS if v & bit]


def safe_name(name):
    return re.sub(r"[^A-Za-z0-9_.-]", "_", name)[:180] or "_"


def parse_args(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--tiers", default="1,2,3,4",
                    help="comma list of target tiers to decompile, in this order (default 1,2,3,4)")
    ap.add_argument("--only", action="append", default=[],
                    help="decompile only this address (repeatable), optional ':Class.Function' label; "
                         "addresses not in the target list get class _adhoc")
    ap.add_argument("--limit", type=int, default=0, help="stop after N functions (0 = no limit)")
    ap.add_argument("--max-seconds", type=float, default=5400.0,
                    help="total wall-clock cap for the decompile loop (default 5400 = 90 min)")
    ap.add_argument("--timeout", type=int, default=30, help="per-function decompiler timeout, seconds")
    ap.add_argument("--out", default=DEFAULT_OUT, help="output root (default porting/generated/decomp)")
    ap.add_argument("--no-index", action="store_true", help="do not read or write index.json")
    ap.add_argument("--create-missing", action="store_true",
                    help="create a function at each address that has none (in memory, rolled back)")
    ap.add_argument("--rename", action="store_true",
                    help="rename functions from native_funcs.json before decompiling (in memory, "
                         "rolled back); output is marked as renamed")
    ap.add_argument("--mem", default=os.environ.get("GHIDRA_HEADLESS_MAXMEM", "32G"),
                    help="JVM max heap, e.g. 32G (default GHIDRA_HEADLESS_MAXMEM or 32G)")
    return ap.parse_args(argv)


def load_targets():
    with open(TARGETS_JSON, encoding="utf-8") as fh:
        data = json.load(fh)
    return data["targets"]


def select_targets(args, targets):
    """Return the ordered list of records to decompile (dicts with class/function/tier/address/...)."""
    by_addr = {}
    for t in targets:
        if t.get("exec_addr"):
            by_addr.setdefault(t["exec_addr"].lower(), t)
    records = []
    if args.only:
        for spec in args.only:
            addr, _, label = spec.partition(":")
            addr = addr.lower()
            if not addr.startswith("0x"):
                addr = "0x" + addr
            t = by_addr.get(addr)
            if t is None:
                cls, _, fn = label.partition(".")
                t = {"tier": 0, "class": cls or "_adhoc", "function": fn or addr,
                     "exec_addr": addr, "flags": None, "net_roles": [], "rank": 0}
            records.append(t)
    else:
        tiers = [int(x) for x in args.tiers.split(",") if x.strip()]
        for tier in tiers:
            rows = sorted((t for t in targets if t["tier"] == tier), key=lambda t: t.get("rank", 0))
            records.extend(rows)
    # Tier 5 has no exec address: keep them out of the decompile list, but list them in the index.
    if args.limit:
        records = records[:args.limit]
    return records


def tkey(t):
    """Identity of a target. Class and function are unique; several targets can share one exec address
    (the linker folds identical functions), so the address is not used as a key."""
    return t["class"] + "." + t["function"]


def assign_paths(all_targets, records, out_root):
    """File path per target: <Class>/<Function>.c, with an address suffix when the name repeats."""
    counts = collections.Counter(tkey(t) for t in all_targets)
    paths = {}
    for t in records:
        stem = safe_name(t["function"])
        if counts[tkey(t)] > 1 or t.get("tier") == 0:
            stem = f"{stem}__{(t['exec_addr'] or 'noaddr').replace('0x', '')}"
        paths[tkey(t)] = os.path.join(safe_name(t["class"]), stem + ".c")
    return paths


def header_for(t, rename_note):
    flags = t.get("flags")
    flag_txt = ", ".join(decode_flags(flags)) if flags else "n/a"
    roles = ", ".join(t.get("net_roles") or []) or "none"
    lines = [
        "/*",
        " * Raw Ghidra decompiler output (DecompInterface), derived from the user's supplied client.",
        " * Generated by porting/ghidra/decompile_targets.py. Git-ignored.",
        f" * Class:     {t['class']}",
        f" * Function:  {t['function']}",
        f" * Tier:      {t.get('tier')} ({TIER_NAMES.get(t.get('tier'), 'unknown')})",
        f" * Address:   {t.get('exec_addr')}",
        f" * Flags:     {flags or 'n/a'} ({flag_txt})",
        f" * Net roles: {roles}",
        " * Analysis:  slim project; decompiler parameter ID and stack analysis were off, so",
        " *            parameters, prototypes and calling conventions are unresolved.",
    ]
    if rename_note:
        lines.append(f" * Symbols:   {rename_note}")
    lines.append(" */")
    return "\n".join(lines) + "\n"


def snapshot(paths):
    """(relpath, size, mtime_ns) for every file under the project paths, to detect disk changes."""
    out = {}
    for root_path in paths:
        if os.path.isfile(root_path):
            st = os.stat(root_path)
            out[root_path] = (st.st_size, st.st_mtime_ns)
            continue
        for dirpath, _dirs, files in os.walk(root_path):
            for name in files:
                full = os.path.join(dirpath, name)
                try:
                    st = os.stat(full)
                except OSError:
                    continue
                out[full] = (st.st_size, st.st_mtime_ns)
    return out


def snapshot_diff(before, after):
    changed = [p for p in set(before) | set(after) if before.get(p) != after.get(p)]
    return sorted(changed)


def _printable(b):
    return 0x20 <= b <= 0x7e or b in (0x09, 0x0a, 0x0d)


def read_cstring(memory, addr, max_len=240):
    """Heuristic read of an undefined ASCII literal (NUL-terminated, printable, >= 3 chars)."""
    try:
        block = memory.getBlock(addr)
        if block is None or not block.isInitialized() or block.isExecute():
            return None
        chars = []
        for i in range(max_len):
            b = memory.getByte(addr.add(i)) & 0xFF
            if b == 0:
                break
            if not _printable(b):
                return None
            chars.append(chr(b))
        return "".join(chars) if len(chars) >= 3 else None
    except Exception:
        return None


def read_wstring(memory, addr, max_len=160):
    """Heuristic read of an undefined UTF-16LE literal (ASCII range only, NUL-terminated)."""
    try:
        block = memory.getBlock(addr)
        if block is None or not block.isInitialized() or block.isExecute():
            return None
        chars = []
        for i in range(0, max_len * 2, 2):
            lo = memory.getByte(addr.add(i)) & 0xFF
            hi = memory.getByte(addr.add(i + 1)) & 0xFF
            if lo == 0 and hi == 0:
                break
            if hi != 0 or not _printable(lo):
                return None
            chars.append(chr(lo))
        return "".join(chars) if len(chars) >= 3 else None
    except Exception:
        return None


def string_at(program, addr, StringDataInstance):
    """Literal text at addr: defined string data first, else a heuristic read of raw bytes."""
    listing = program.getListing()
    data = listing.getDataAt(addr)
    if data is not None:
        sdi = StringDataInstance.getStringDataInstance(data)
        if sdi is not None and sdi != StringDataInstance.NULL_INSTANCE:
            val = sdi.getStringValue()
            if val is not None and len(str(val)) > 0:
                return str(val)
    memory = program.getMemory()
    return read_cstring(memory, addr) or read_wstring(memory, addr)


def analyze_body(program, func, StringDataInstance):
    """Called addresses (direct), indirect call count, and string literals referenced from the body."""
    listing = program.getListing()
    fm = program.getFunctionManager()
    called = collections.OrderedDict()
    indirect = 0
    strings = collections.OrderedDict()
    for instr in listing.getInstructions(func.getBody(), True):
        ft = instr.getFlowType()
        if ft.isCall():
            flows = list(instr.getFlows() or [])
            direct = [f for f in flows if f.isMemoryAddress()]
            if not direct:
                indirect += 1
            for f in direct:
                key = hex(int(f.getOffset()))
                if key not in called and len(called) < MAX_CALLED:
                    callee = fm.getFunctionAt(f)
                    called[key] = callee.getName() if callee is not None else None
        for ref in instr.getReferencesFrom():
            if ref.getReferenceType().isFlow():
                continue
            to = ref.getToAddress()
            if to is None or not to.isMemoryAddress():
                continue
            if len(strings) >= MAX_STRINGS:
                break
            text = string_at(program, to, StringDataInstance)
            if text is not None and text not in strings:
                strings[text] = hex(int(to.getOffset()))
    return ([{"address": a, "name": n} for a, n in called.items()], indirect,
            [{"text": t, "address": a} for t, a in strings.items()])


def decompile_one(decomp, func, timeout, monitor):
    res = decomp.decompileFunction(func, timeout, monitor)
    if res is None:
        return "failed", None, "no result from decompiler"
    if res.isTimedOut():
        return "timeout", None, res.getErrorMessage() or "timed out"
    if not res.decompileCompleted():
        return "failed", None, res.getErrorMessage() or "decompile did not complete"
    dec = res.getDecompiledFunction()
    if dec is None:
        return "failed", None, "no decompiled function"
    return "ok", str(dec.getC()), None


def ensure_function(program, addr, flat):
    """Return the function at addr, creating one in memory (rolled back later) if missing."""
    fm = program.getFunctionManager()
    func = fm.getFunctionAt(addr)
    if func is not None:
        return func, False
    if program.getListing().getInstructionAt(addr) is None:
        flat.disassemble(addr)
    func = flat.createFunction(addr, None)
    if func is None:
        func = fm.getFunctionAt(addr)
    return func, func is not None


def apply_renames(program, native_json):
    """Name functions Class_Function from native_funcs.json (in memory). Returns (renamed, dup, miss)."""
    from ghidra.program.model.symbol import SourceType
    from ghidra.util.exception import DuplicateNameException, InvalidInputException

    fm = program.getFunctionManager()
    renamed = dup = miss = 0
    with open(native_json, encoding="utf-8") as fh:
        native = json.load(fh)
    for cls, funcs in native.items():
        for fname, rec in funcs.items():
            addr_s = rec.get("exec_addr")
            if not addr_s:
                continue
            addr = program.getAddressFactory().getDefaultAddressSpace().getAddress(addr_s)
            func = fm.getFunctionAt(addr)
            if func is None:
                miss += 1
                continue
            name = f"{cls}_{fname}"
            try:
                func.setName(name, SourceType.USER_DEFINED)
                renamed += 1
            except DuplicateNameException:
                try:
                    func.setName(f"{name}_{addr_s.replace('0x', '')}", SourceType.USER_DEFINED)
                    dup += 1
                except (DuplicateNameException, InvalidInputException):
                    miss += 1
            except InvalidInputException:
                miss += 1
    return renamed, dup, miss


def load_index(path):
    if not os.path.exists(path):
        return {}
    with open(path, encoding="utf-8") as fh:
        data = json.load(fh)
    return {r["class"] + "." + r["function"]: r for r in data.get("functions", [])}


def write_index(path, records_by_addr, run_meta):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    rows = sorted(records_by_addr.values(), key=lambda r: (r.get("tier", 9), r["class"], r["function"]))
    counts = collections.Counter(r["status"] for r in rows)
    meta = {"generated": datetime.datetime.now().isoformat(timespec="seconds"),
            "status_counts_total": dict(counts), "functions_total": len(rows)}
    meta.update(run_meta)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump({"_meta": meta, "functions": rows}, fh, indent=1, sort_keys=False)
        fh.write("\n")
    os.replace(tmp, path)


def _run_one(t, rec, rel, program, decomp, flat, monitor, args, out_root, rename_note,
             StringDataInstance, counts):
    """Decompile one target, fill its index record, and write its .c file when ok."""
    addr = program.getAddressFactory().getDefaultAddressSpace().getAddress(t["exec_addr"])
    t0 = time.time()
    try:
        if args.create_missing:
            func, created = ensure_function(program, addr, flat)
        else:
            func, created = program.getFunctionManager().getFunctionAt(addr), False
        if func is None:
            rec.update({"status": "failed", "lines": 0, "called": [], "indirect_calls": 0,
                        "strings": [], "error": "no function at address" +
                        ("" if args.create_missing else " (run with --create-missing)")})
            counts["failed"] += 1
            return rec
        status, code, err = decompile_one(decomp, func, args.timeout, monitor)
        called, indirect, strings = analyze_body(program, func, StringDataInstance)
        body = code or ""
        lines = body.count("\n") + (1 if body and not body.endswith("\n") else 0)
        rec.update({"status": status, "lines": lines, "called": called, "indirect_calls": indirect,
                    "strings": strings, "ghidra_name": func.getName(), "created_in_memory": bool(created),
                    "error": err, "seconds": round(time.time() - t0, 2)})
        if status == "ok":
            full = os.path.join(out_root, rel)
            os.makedirs(os.path.dirname(full), exist_ok=True)
            with open(full, "w", encoding="utf-8") as fh:
                fh.write(header_for(t, rename_note))
                fh.write(body)
    except Exception as exc:  # one bad function must not stop the run
        rec.update({"status": "failed", "lines": 0, "called": [], "indirect_calls": 0, "strings": [],
                    "error": f"{type(exc).__name__}: {exc}", "seconds": round(time.time() - t0, 2)})
    counts[rec["status"]] += 1
    return rec


def main(argv=None):
    args = parse_args(argv if argv is not None else sys.argv[1:])
    out_root = os.path.abspath(args.out)
    index_path = os.path.join(out_root, "index.json")
    all_targets = load_targets()
    records = select_targets(args, all_targets)
    paths = assign_paths(all_targets, records, out_root)
    run_started = time.time()
    deadline = run_started + args.max_seconds

    import jpype  # noqa: F401  (imported by pyghidra; kept for the clearer error if missing)
    import pyghidra
    from pyghidra.launcher import HeadlessPyGhidraLauncher

    launcher = HeadlessPyGhidraLauncher(install_dir=INSTALL)
    launcher.add_vmargs(f"-Xmx{args.mem}")
    launcher.start()

    from ghidra.app.decompiler import DecompInterface, DecompileOptions
    from ghidra.program.flatapi import FlatProgramAPI
    from ghidra.program.model.data import StringDataInstance
    from ghidra.util.task import TaskMonitor

    project_root = os.path.join(PROJECT_DIR, PROJECT + ".rep")
    project_gpr = os.path.join(PROJECT_DIR, PROJECT + ".gpr")
    before = snapshot([project_root, project_gpr])

    project = pyghidra.open_project(PROJECT_DIR, PROJECT, create=False)
    program, consumer = pyghidra.consume_program(project, PROGRAM_PATH)
    tx = None
    counts = collections.Counter()
    rename_note = None
    try:
        if args.rename or args.create_missing:
            tx = program.startTransaction("decompile_targets in-memory (rolled back)")
            if args.rename:
                r, d, m = apply_renames(program, NATIVE_JSON)
                rename_note = (f"renamed from native_funcs.json in memory ({r} renamed, "
                               f"{d} with address suffix, {m} not applied)")
                _log(f"rename pass: {r} renamed, {d} suffixed, {m} not applied")
        opts = DecompileOptions()
        opts.grabFromProgram(program)
        decomp = DecompInterface()
        decomp.setOptions(opts)
        decomp.toggleCCode(True)
        decomp.toggleSyntaxTree(True)
        decomp.setSimplificationStyle("decompile")
        if not decomp.openProgram(program):
            raise SystemExit("DecompInterface.openProgram failed: " + str(decomp.getLastMessage()))
        flat = FlatProgramAPI(program)
        monitor = TaskMonitor.DUMMY

        index = {} if args.no_index else load_index(index_path)
        for n, t in enumerate(records, 1):
            key = tkey(t)
            if time.time() > deadline:
                counts["not_run_cap"] += len(records) - n + 1
                _log(f"  wall-clock cap reached after {n - 1} functions; the rest are not run")
                break
            rel = paths[key]
            rec = {"tier": t.get("tier"), "class": t["class"], "function": t["function"],
                   "address": t["exec_addr"], "flags": t.get("flags"),
                   "net_roles": t.get("net_roles") or [], "file": "decomp/" + rel.replace(os.sep, "/")}
            if not t["exec_addr"]:
                rec.update({"status": "skipped", "error": "no exec address", "lines": 0,
                            "called": [], "indirect_calls": 0, "strings": []})
                counts["skipped"] += 1
            else:
                rec = _run_one(t, rec, rel, program, decomp, flat, monitor, args, out_root,
                               rename_note, StringDataInstance, counts)
            index[key] = rec
            if n % 20 == 0:
                _log(f"  {n}/{len(records)} done, ok={counts['ok']} timeout={counts['timeout']} "
                     f"failed={counts['failed']} elapsed={time.time() - run_started:.0f}s")
                if not args.no_index:
                    write_index(index_path, index, {"partial_run": True})
        decomp.dispose()
    finally:
        if tx is not None:
            program.endTransaction(tx, False)  # roll back: nothing is committed
        program.release(consumer)
        project.close()

    after = snapshot([project_root, project_gpr])
    changed = snapshot_diff(before, after)
    run_meta = {"last_run": {"tiers": args.tiers if not args.only else "only",
                             "requested": len(records), "counts": dict(counts),
                             "timeout_s": args.timeout, "max_seconds": args.max_seconds,
                             "create_missing": args.create_missing, "rename": args.rename,
                             "rename_note": rename_note,
                             "elapsed_s": round(time.time() - run_started, 1),
                             "project_files_changed": changed[:20],
                             "project_files_changed_count": len(changed)}}
    if not args.no_index:
        write_index(index_path, index, run_meta)
    summary = {"requested": len(records), "counts": dict(counts), "not_run_cap": counts["not_run_cap"],
               "elapsed_s": round(time.time() - run_started, 1),
               "project_files_changed": len(changed)}
    _log("SUMMARY " + json.dumps(summary))
    if changed:
        _log("WARNING project files changed on disk: " + ", ".join(changed[:10]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
