#!/usr/bin/env python3
"""Replication conditions and custom net serializers, from the shipped client (read-only Ghidra use).

Three steps, each writing into a scratch directory (never into porting/generated, which is git-ignored):

  inputs  (stdlib, reads the runtime dump): for every class with owned replicated properties, the VA of each
          property name's UTF-16 literal in the dump; for every struct with a custom net serializer trait, its ops
          vtable and the slot 14 (NetSerialize) and slot 16 (NetDeltaSerialize) code addresses (ICppStructOps order,
          porting/binary_analysis.md section 9).
          python3 -I porting/ghidra/net_lifetime_serializers.py inputs --out DIR

  probe   (PyGhidra, slim project, opened without analysis, nothing saved; functions missing from the database are
          created in memory and the transaction is rolled back). Finds the DOREPLIFETIME registrations of each class
          (calls to RegisterReplicatedLifetimeProperty that reference the most of the class's literals) and decompiles
          the struct serializers.
          porting/.venv/bin/python -I porting/ghidra/net_lifetime_serializers.py probe --in DIR/probe_in.json --out DIR/probe

  build   (stdlib): decodes the registration decompiles and writes porting/replication_conditions.json.
          python3 -I porting/ghidra/net_lifetime_serializers.py build --probe DIR/probe --out porting/replication_conditions.json

Decoding (validated on ACharacter and APawn, 11 of 11 conditions equal to the UE 4.25 source): a registration is
RegisterReplicatedLifetimeProperty(descriptor or FProperty*, TArray&, const FDoRepLifetimeParams&). The params block is
Condition (int32 at +0), RepNotifyCondition (int32 at +4), bIsPushBased (bool at +8). The decompiler writes the 8-byte
block at +0 (uStack_NN = value, or CONCAT44 / masked forms), and the call passes &uStack_NN. The name is the last
L"..." literal before the call. A two-argument call is the default DOREPLIFETIME (COND_None). Push flags are not
recovered by name in the stack-cell layout (see decode_registrations); the generator does not emit push-model
registrations (bIsPushBased) because the native bodies that mark properties dirty are not recovered.
"""
import argparse
import collections
import json
import os
import re
import shutil
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORTING = os.path.dirname(HERE)

GHIDRA_INSTALL = os.environ.get("GHIDRA_INSTALL_DIR", "/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC")
PROJECT_DIR = os.environ.get("FN_GHIDRA_PROJECT_DIR", "/opt/fortnite/ghidra-projects")
PROJECT = os.environ.get("FN_GHIDRA_PROJECT", "fortnite-slim")
PROGRAM_PATH = os.environ.get("FN_GHIDRA_PROGRAM_PATH", "/FortniteClient-Win64-Shipping.runtime.exe")
# RegisterReplicatedLifetimeProperty overloads of this exe (identified by their call shape in the decompile of
# UFortHomebaseSet and AFortPlayerControllerAthena): descriptor (FRepPropertyDescriptor*) and FProperty*.
REG_CALLS = ("144678530", "144678610")
# Two-argument registration thunk (DOREPLIFETIME with the default FDoRepLifetimeParams).
DEFAULT_THUNK = "14467b200"
ICPP_SLOT_NET_SERIALIZE = 14
ICPP_SLOT_NET_DELTA_SERIALIZE = 16
SHARED_STUBS = {"1406801e0", "1406801c0", "1406801d0"}   # shared default bodies (see porting/behavior/README.md)
COND_NAMES = {0: "COND_None", 1: "COND_InitialOnly", 2: "COND_OwnerOnly", 3: "COND_SkipOwner", 4: "COND_SimulatedOnly",
              5: "COND_AutonomousOnly", 6: "COND_SimulatedOrPhysics", 7: "COND_InitialOrOwner", 8: "COND_Custom",
              9: "COND_ReplayOrOwner", 10: "COND_ReplayOnly", 11: "COND_SimulatedOnlyNoReplay",
              12: "COND_SimulatedOrPhysicsNoReplay", 13: "COND_SkipReplay", 15: "COND_Never"}
REFL = os.path.join(PORTING, "generated", "reflection")
DUMP = os.path.join(PORTING, "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")


def hexva(v):
    return "0x%x" % v if isinstance(v, int) else v


# ------------------------------------------------------------------------------------------ inputs (stdlib)
def cmd_inputs(args):
    # porting/ is put on the path here only: inside the PyGhidra run, porting/ghidra would shadow the ghidra package.
    sys.path.insert(0, PORTING)
    import class_checksum as CC
    import extract_reflection as ER
    os.makedirs(args.out, exist_ok=True)
    model = CC.Model(CC.REFL, CC.IMAGE, set_path=CC.CLASS_SET, source_root=CC.SOURCE_ROOT)
    img = ER.Image(args.dump)
    mm = img.m
    classes = []
    for va in sorted(model.blocks, key=lambda v: model.names.get(v) or ""):
        props = model.owned_replicated(va)
        if not props or not model.names.get(va):
            continue
        lits = []
        for p in props:
            pat = p["name"].encode("utf-16-le") + b"\x00\x00"
            hits, i = [], mm.find(pat)
            while i >= 0 and len(hits) < 64:
                if i % 2 == 0:
                    for s in img.sections:
                        if s["raw"] <= i < s["raw"] + s["rsize"]:
                            hits.append(hexva(img.imagebase + s["va"] + (i - s["raw"])))
                            break
                i = mm.find(pat, i + 1)
            lits.append({"name": p["name"], "literals": hits})
        classes.append({"class": model.names[va], "block_va": hexva(va), "props": lits})
    structs = []
    for s in model.structs.values():
        ops = s.get("struct_ops") or {}
        traits = ops.get("traits") or {}
        if not any(traits.get(k) is True for k in ("HasNetSerializer", "HasNetDeltaSerializer", "HasNetSharedSerialization")):
            continue
        vt = int(ops["vtable_va"], 16)
        rec = {"name": s["name"], "package": s.get("package"), "block_va": s["block_va"], "vtable": hexva(vt),
               "traits": sorted(k for k, v in traits.items() if v is True and "Net" in k)}
        for slot, key in ((ICPP_SLOT_NET_SERIALIZE, "slot14"), (ICPP_SLOT_NET_DELTA_SERIALIZE, "slot16")):
            o = img.off(vt + 8 * slot)
            rec[key] = hex(struct.unpack_from("<Q", mm, o)[0]) if o is not None else None
        structs.append(rec)
    out = os.path.join(args.out, "probe_in.json")
    json.dump({"structs": structs, "classes": classes}, open(out, "w"), indent=1)
    print("classes %d structs %d -> %s" % (len(classes), len(structs), out))


# ------------------------------------------------------------------------------------------ probe (PyGhidra)
def cmd_probe(args):
    os.makedirs(args.out, exist_ok=True)
    data = json.load(open(args.inp))
    shutil.copy(args.inp, os.path.join(args.out, "probe_in.json"))
    import pyghidra
    from pyghidra.launcher import HeadlessPyGhidraLauncher
    launcher = HeadlessPyGhidraLauncher(install_dir=GHIDRA_INSTALL)
    launcher.add_vmargs("-Xmx%s" % args.mem)
    launcher.start()
    from ghidra.app.decompiler import DecompInterface, DecompileOptions
    from ghidra.program.flatapi import FlatProgramAPI
    from ghidra.util.task import TaskMonitor
    project = pyghidra.open_project(PROJECT_DIR, PROJECT, create=False)
    program, consumer = pyghidra.consume_program(project, PROGRAM_PATH)
    monitor = TaskMonitor.DUMMY
    fm, rm = program.getFunctionManager(), program.getReferenceManager()
    af = program.getAddressFactory().getDefaultAddressSpace()
    flat = FlatProgramAPI(program)
    opts = DecompileOptions()
    opts.grabFromProgram(program)
    decomp = DecompInterface()
    decomp.setOptions(opts)
    decomp.toggleCCode(True)
    decomp.setSimplificationStyle("decompile")
    decomp.openProgram(program)
    tx = program.startTransaction("net_lifetime_serializers probe (rolled back)")

    def addr(v):
        return af.getAddress(int(v, 16))

    def text_of(f):
        res = decomp.decompileFunction(f, 120, monitor)
        if res is None or not res.decompileCompleted():
            return None
        return str(res.getDecompiledFunction().getC())

    result = {"classes": [], "serializers": []}
    try:
        # 1. registration callers, then the function of each class that references the most of its literals
        callers = collections.defaultdict(set)
        for reg in REG_CALLS:
            it = rm.getReferencesTo(addr(reg))
            while it.hasNext():
                ref = it.next()
                if ref.getReferenceType().isCall() or ref.getReferenceType().isFlow():
                    c = fm.getFunctionContaining(ref.getFromAddress())
                    if c is not None:
                        callers[str(c.getEntryPoint())].add(reg)
        for c in data["classes"]:
            matched = collections.defaultdict(set)
            for p in c["props"]:
                for lit in p["literals"]:
                    it = rm.getReferencesTo(addr(lit))
                    while it.hasNext():
                        f = fm.getFunctionContaining(it.next().getFromAddress())
                        if f is not None and str(f.getEntryPoint()) in callers:
                            matched[str(f.getEntryPoint())].add(p["name"])
            ranked = sorted(matched.items(), key=lambda kv: -len(kv[1]))[:4]
            rec = {"class": c["class"], "block_va": c["block_va"], "props": len(c["props"]),
                   "candidates": [[e, len(ns)] for e, ns in ranked]}
            if ranked:
                e, ns = ranked[0]
                f = fm.getFunctionAt(addr(e))
                rec.update({"best": e, "best_matched": len(ns), "regs": sorted(callers[e]),
                            "missing": sorted({p["name"] for p in c["props"]} - ns)})
                text = text_of(f) if f else None
                if text:
                    name = c["class"] + "_reg.c"
                    open(os.path.join(args.out, name), "w").write(text)
                    rec["file"] = name
            result["classes"].append(rec)
        # 2. struct serializers: create missing functions in memory, decompile, roll back
        seen = set()
        for s in data["structs"]:
            for slot in ("slot14", "slot16"):
                v = s.get(slot)
                if not v or v[2:] in SHARED_STUBS or v in seen:
                    continue
                seen.add(v)
                a = addr(v)
                f = fm.getFunctionAt(a)
                created = False
                if f is None:
                    if program.getListing().getInstructionAt(a) is None:
                        flat.disassemble(a)
                    f = flat.createFunction(a, None)
                    created = f is not None
                rec = {"key": "%s_%s" % (s["name"], slot), "addr": v, "created": created}
                if f is not None:
                    rec["callees"] = [[cf.getName(), str(cf.getEntryPoint())] for cf in f.getCalledFunctions(monitor)]
                    text = text_of(f)
                    if text:
                        name = rec["key"] + ".c"
                        os.makedirs(os.path.join(args.out, "serializers"), exist_ok=True)
                        open(os.path.join(args.out, "serializers", name), "w").write(text)
                        rec["file"] = name
                result["serializers"].append(rec)
    finally:
        program.endTransaction(tx, False)
        decomp.dispose()
        program.release(consumer)
    json.dump(result, open(os.path.join(args.out, "probe_out.json"), "w"), indent=1)
    print("classes %d serializers %d" % (len(result["classes"]), len(result["serializers"])))


# ------------------------------------------------------------------------------------------ build (stdlib)
ASSIGN = re.compile(r"^\s*[a-zA-Z]+Stack_([0-9a-f]+)\s*=\s*(.+?);\s*$")
LIT = re.compile(r'L"([^"]*)"')
CALL = re.compile(r"(FUN_14467(?:8530|8610)|thunk_FUN_14467b200)\(\s*([^,]+?)\s*,\s*param_\d+\s*"
                  r"(?:,\s*&[a-zA-Z]+Stack_([0-9a-f]+))?\s*\);")


def _strip_cast(expr):
    return re.sub(r"^\(\s*\w+(?:\s*\*)?\s*\)", "", expr.strip())


def _num(expr):
    e = _strip_cast(expr)
    try:
        return int(e, 16) if e.lower().startswith("0x") else int(e, 10)
    except ValueError:
        return None


def decode_registrations(text):
    """[(name, condition, notify, push, unknown)] of one registration function's decompile, in program order.

    The decompiler names each stack cell by its address, and the names run opposite to the addresses: the params
    block passed as &uStack_B holds Condition at B, RepNotifyCondition at B-4 and bIsPushBased at B-8 (checked on
    AActor::GetLifetimeReplicatedProps: AttachmentReplicationParams{COND_Custom, REPNOTIFY_Always, true} is
    "uStack_38 = 8; uStack_34 = 1; uStack_30 = 1" before &uStack_38; ReplicatedMovementParams is "uStack_28 = 6;
    uStack_24 = 1; uStack_20 = 1" before &uStack_28). A cell written as one 8-byte value (uStack_28 = 0x100000000)
    carries RepNotifyCondition in its high half. A params block that is written once and passed to several
    registrations (SharedParams in AActor) keeps its last decoded value until its cells are stored again."""
    cell = collections.defaultdict(lambda: {"low": 0, "high": 0, "unknown": False})
    fresh = set()          # cell offsets stored since the previous registration
    block = {}             # B -> (condition, notify, push, unknown) of its last registration
    name = None
    regs = []
    for line in text.split("\n"):
        for lit in LIT.findall(line):
            name = lit
        m = ASSIGN.match(line)
        if m:
            off = int(m.group(1), 16)
            c, expr = cell[off], m.group(2).strip()
            fresh.add(off)
            v = _num(expr)
            if v is not None:
                c["low"], c["high"] = v & 0xffffffff, (v >> 32) & 0xffffffff
            elif re.match(r"CONCAT44\(\s*[a-zA-Z]+Stack_[0-9a-f]+\._4_4_\s*,\s*(\w+)\s*\)$", expr):
                c["low"] = _num(re.search(r",\s*(\w+)\)$", expr).group(1)) & 0xffffffff
            elif re.match(r"CONCAT44\(\s*(\w+)\s*,\s*\(undefined4\)[a-zA-Z]+Stack_[0-9a-f]+\s*\)$", expr):
                c["high"] = _num(re.match(r"CONCAT44\(\s*(\w+)", expr).group(1)) & 0xffffffff
            elif re.match(r"[a-zA-Z]+Stack_[0-9a-f]+\s*&\s*0xffffffff00000000$", expr):
                c["low"] = 0
            elif re.match(r"[a-zA-Z]+Stack_[0-9a-f]+\s*&\s*0xffffff00$", expr):
                c["low"] &= 0xff
            else:
                c["unknown"] = True
            continue
        for mm in CALL.finditer(line):
            off = mm.group(3)
            if off is None:
                regs.append((name, 0, 0, 0, False))   # two-argument thunk: default FDoRepLifetimeParams
                continue
            b = int(off, 16)
            if {b, b - 4, b - 8} & fresh or b not in block:
                cond = cell[b]
                notify = cell[b - 4]["low"] if (b - 4) in fresh else cell[b]["high"]
                push = (cell[b - 8]["low"] & 1) if (b - 8) in fresh else (block.get(b, (0, 0, 0, False))[2])
                block[b] = (cond["low"], notify, push, cond["unknown"])
            regs.append((name,) + block[b])
            fresh.clear()
    return regs


def cmd_build(args):
    probe = json.load(open(os.path.join(args.probe, "probe_out.json")))
    props_of = {}
    inp = json.load(open(os.path.join(args.probe, "probe_in.json")))
    for c in inp["classes"]:
        props_of[c["class"]] = [p["name"] for p in c["props"]]
    out = {"method": ("GetLifetimeReplicatedProps of each class: the function whose registrations (calls to "
                      "RegisterReplicatedLifetimeProperty) reference the most of the class's replicated property names, "
                      "in the slim Ghidra project. Condition, RepNotifyCondition and bIsPushBased of each registration are "
                      "read from the stores to its FDoRepLifetimeParams block (see decode_registrations). The generator "
                      "emits Condition and RepNotifyCondition only; bIsPushBased is recorded for reference."),
           "validation": ("ACharacter (8 properties), APawn (3) and AActor (10) decode as in UE 4.25 (Engine Private/"
                          "Character.cpp, Pawn.cpp, ActorReplication.cpp), including AActor's SharedParams push flags and "
                          "its REPNOTIFY_Always entries. Classes outside the native set (engine classes used for this "
                          "check, and others) are listed too; the generator reads only the native set."),
           "classes": {}}
    counts = collections.Counter()
    for rec in probe["classes"]:
        cname = rec["class"]
        names = props_of.get(cname, [])
        entry = {"block_va": rec["block_va"], "function_va": rec.get("best"), "replicated_owned": len(names),
                 "properties": {}}
        if not rec.get("file"):
            entry["confidence"] = "unknown"
            entry["note"] = "no registration function found: every property is COND_None with a TODO"
            for n in names:
                entry["properties"][n] = {"condition": "COND_None", "confidence": "unknown"}
            out["classes"][cname] = entry
            counts["class_unknown"] += 1
            continue
        regs = decode_registrations(open(os.path.join(args.probe, rec["file"])).read())
        by_name = collections.OrderedDict()
        dup = set()
        for n, cond, notify, push, unk in regs:
            if n in by_name:
                dup.add(n)
                continue
            by_name[n] = (cond, notify, push, unk)
        missing = [n for n in names if n not in by_name]
        extra = [n for n in by_name if n not in names]
        conf = "low" if missing else ("medium" if dup or extra or len(regs) != len(names) else "high")
        entry["confidence"] = conf
        notes = []
        if missing:
            notes.append("not registered in the chosen function: " + ", ".join(missing))
        if dup:
            notes.append("registered in two branches: " + ", ".join(sorted(dup)) + "; first registration used")
        if extra:
            notes.append("registered but not in the replicated list: " + ", ".join(extra))
        if notes:
            entry["note"] = "; ".join(notes)
        for n in names:
            if n not in by_name or by_name[n][3]:
                entry["properties"][n] = {"condition": "COND_None", "confidence": "unknown"}
                counts["prop_unknown"] += 1
                continue
            cond, notify, push, _unk = by_name[n]
            if cond not in COND_NAMES:
                entry["properties"][n] = {"condition": "COND_None", "confidence": "unknown", "raw_condition": cond}
                counts["prop_unknown"] += 1
                continue
            pe = {"condition": COND_NAMES[cond], "confidence": "medium" if n in dup else "high"}
            if notify:
                pe["rep_notify_condition"] = "REPNOTIFY_Always" if notify == 1 else "raw_%d" % notify
            # Recorded for reference only: the generator does not emit push-model registrations (see the docstring).
            pe["bIsPushBased"] = bool(push)
            entry["properties"][n] = pe
            counts["prop_" + COND_NAMES[cond]] += 1
        out["classes"][cname] = entry
        counts["class_" + conf] += 1
    out["classes"] = dict(sorted(out["classes"].items()))
    out["summary"] = {"classes": len(out["classes"]),
                      "properties": sum(len(v["properties"]) for v in out["classes"].values()),
                      "properties_by_confidence": {c: sum(1 for v in out["classes"].values() for p in v["properties"].values()
                                                          if p["confidence"] == c) for c in ("high", "medium", "unknown")}}
    with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(out, fh, indent=1)
        fh.write("\n")
    print(json.dumps(out["summary"]))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("inputs")
    p.add_argument("--out", required=True)
    p.add_argument("--dump", default=DUMP)
    p.set_defaults(fn=cmd_inputs)
    p = sub.add_parser("probe")
    p.add_argument("--in", dest="inp", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--mem", default="24G")
    p.set_defaults(fn=cmd_probe)
    p = sub.add_parser("build")
    p.add_argument("--probe", required=True, help="the probe output directory (probe_in.json and probe_out.json)")
    p.add_argument("--out", required=True)
    p.set_defaults(fn=cmd_build)
    args = ap.parse_args(argv)
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())
