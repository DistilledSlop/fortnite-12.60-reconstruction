#!/usr/bin/env python3
"""Improve the Athena decompiler input, then decompile the same targets (PyGhidra, read-only).

Opens project fortnite-slim the same way as porting/ghidra/decompile_targets.py. Every edit below runs inside
ONE transaction that is rolled back at the end (endTransaction(tx, False)), so the project is not saved:

  1. Helper identities and data labels (name_tables/helpers.json): FFrame::Step, FFrame::StepExplicitProperty,
     FMemory::Free, RPC_ValidateFailed, GMalloc, GNatives, GLastRPCFailedReason.
  2. FFrame struct (UE 4.25 Stack.h layout; the helper bodies use the same offsets).
  3. Class structs for the 8 Athena targets and their Fortnite ancestors from reflection property offsets
     (name_tables/struct_layouts.json, rebuilt with --build-tables).
  4. Vtable struct for each verified table (AFortPlayerStateAthena 0x145e35fa8, AFortPlayerPawnAthena
     0x145e2cfd0). Slots are named from the tier-1 RPC thunks: a validated RPC calls slot (validate) then
     slot+1 (implementation); an unvalidated one calls the implementation. Written to name_tables/vtable_slots_*.json.
  5. Exec prototypes void Class_Fn(UObject* Context, FFrame* Stack, void* Result) with Context typed as the class
     struct. Each FFrame fetch call inside the body gets an end-of-line comment with the UFunction parameter.
  6. RPC prototypes: X_Validate (bool) and X_Implementation at the vtable entries, from native_funcs.json.

It then decompiles the targets and the RPC _Validate/_Implementation bodies to porting/generated/decomp_improved/
(git-ignored) with index.json, and writes the portability ranking to porting/generated/decomp/portability.json.

Usage (repo root; porting/.venv has pyghidra):
  porting/.venv/bin/python -I porting/ghidra/improve_decompile.py [--build-tables] [--only Class.Function ...]
      [--out DIR] [--timeout 60] [--mem 48G] [--no-decompile]
  porting/.venv/bin/python -I porting/ghidra/improve_decompile.py --rank-only
Environment: GHIDRA_INSTALL_DIR, FN_GHIDRA_PROJECT_DIR, FN_GHIDRA_PROJECT, FN_GHIDRA_PROGRAM_PATH.
"""
import argparse
import collections
import datetime
import json
import os
import re
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)  # decompile_targets.py has no import-time side effects; its helpers are reused.
import decompile_targets as base  # noqa: E402

REPO = os.path.dirname(os.path.dirname(HERE))
GENERATED = os.path.join(REPO, "porting", "generated")
REFL = os.path.join(GENERATED, "reflection")
TABLE_DIR = os.path.join(HERE, "name_tables")
RAW_DIR = os.path.join(GENERATED, "decomp")
IMPROVED_DIR = os.path.join(GENERATED, "decomp_improved")
PROJECT_DIR = os.environ.get("FN_GHIDRA_PROJECT_DIR", "/opt/fortnite/ghidra-projects")
PROJECT = os.environ.get("FN_GHIDRA_PROJECT", "fortnite-slim")
PROGRAM_PATH = os.environ.get("FN_GHIDRA_PROGRAM_PATH", "/FortniteClient-Win64-Shipping.runtime.exe")
INSTALL = os.environ.get("GHIDRA_INSTALL_DIR", "/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC")

ATHENA_CLASSES = ["AFortGameModeAthena", "AFortGameStateAthena", "AFortPlayerStateAthena",
                  "AFortPlayerControllerAthena", "AFortPlayerPawnAthena", "AFortPawn", "AFortPlayerPawn",
                  "AFortWorldSettings"]
# Verified vtable addresses (binary_analysis.md section 10, "Vtables from the constructors").
VERIFIED_TABLES = {"AFortPlayerStateAthena": 0x145E35FA8, "AFortPlayerPawnAthena": 0x145E2CFD0,
                   # Same constructor rule (one primary vptr store): InternalConstructor 0x142a527d0 (GPSC
                   # argument 10 of 0x142a53450) tail-jumps to the constructor 0x1419b7d70, whose only
                   # vptr store is MOV [RDI],RAX after LEA RAX,[0x145e1aac0]. Checked by slot consistency only.
                   "AFortPlayerControllerAthena": 0x145E1AAC0}
MAX_SLOTS = 4096            # upper bound: a table is read until an entry is not executable code (controller: ~1300 slots)
FFRAME_SIZE = 0x98
# UE 4.25 FFrame (CoreUObject/Public/UObject/Stack.h). FOutputDevice base = vptr + two bools.
FFRAME_FIELDS = [
    (0x00, "vftable", "ptr:void", 8), (0x08, "bSuppressEventTag", "uint8", 1),
    (0x09, "bAutoEmitLineTerminator", "uint8", 1), (0x10, "Node", "ptr:UFunction", 8),
    (0x18, "Object", "ptr:UObject", 8), (0x20, "Code", "ptr:uint8", 8), (0x28, "Locals", "ptr:uint8", 8),
    (0x30, "MostRecentProperty", "ptr:void", 8), (0x38, "MostRecentPropertyAddress", "ptr:uint8", 8),
    (0x40, "FlowStack", "undef[48]", 48), (0x70, "PreviousFrame", "ptr:void", 8),
    (0x78, "OutParms", "ptr:void", 8), (0x80, "PropertyChainForCompiledIn", "ptr:void", 8),
    (0x88, "CurrentNativeFunction", "ptr:void", 8), (0x90, "bArrayContextFailed", "uint8", 1),
]
TIER_NAMES = {0: "RPC _Validate / _Implementation (vtable)", 1: "FUNC_Net (RPC / replicated)",
              2: "FUNC_Native + FUNC_BlueprintEvent", 3: "FUNC_BlueprintCallable with exec",
              4: "other native with exec"}
PORTABLE_MIN = 75.0
NEEDS_MIN = 45.0

# Reflection kind -> (type string, size). Type strings are described in name_tables/helpers.json _meta.
KIND_TYPE = {
    "Bool": ("bool", 1), "Byte": ("uint8", 1), "Enum": ("uint8", 1), "Int": ("int32", 4),
    "UInt32": ("uint32", 4), "Int64": ("int64", 8), "UInt64": ("uint64", 8), "Int16": ("int16", 2),
    "UInt16": ("uint16", 2), "Float": ("float", 4), "Name": ("FName", 8), "Str": ("FString", 16),
    "Text": ("FText", 8), "Array": ("TArray", 16), "Object": ("ptr:UObject", 8), "Class": ("ptr:UObject", 8),
    "Interface": ("ptr:UObject", 8), "WeakObject": ("uint64", 8), "Delegate": ("FScriptDelegate", 16),
    "FieldPath": ("ptr:void", 8), "Map": ("ptr:void", 8), "Set": ("ptr:void", 8),
    "SoftObject": ("ptr:void", 8), "SoftClass": ("ptr:void", 8),
}
SCALAR_SIZE = {"int32": 4, "uint32": 4, "int64": 8, "uint64": 8, "int16": 2, "uint16": 2, "float": 4,
               "uint8": 1, "bool": 1}

NOISE_RE = re.compile(r"\b(param_\d+|undefined\d?|uStack\w*|lStack\w*|alStack\w*|plStack\w*|auStack\w*|"
                      r"puStack\w*|local_\w+|[a-z]{1,3}Var\d+|in_\w+|longlong|ulonglong)\b")
CALL_RE = re.compile(r"^(CALL|JMP)\s+qword ptr \[(R[A-Z0-9]+)(?: \+ 0x([0-9a-fA-F]+))?\]$")


def _log(msg):
    print(msg, flush=True)


def load_json(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def write_json(path, data, indent=1):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=indent)
        fh.write("\n")
    os.replace(tmp, path)


# ----------------------------------------------------------------------------------------------------------
# Type strings and parameter lists (no Ghidra)
# ----------------------------------------------------------------------------------------------------------

def struct_index_of(struct_blocks):
    """VA -> (name, size). native_funcs/reflection type_refs hold the struct's construct function VA, so both the
    block VA and each construct function VA are keys."""
    out = {}
    for s in struct_blocks.get("structs", []):
        val = (s.get("name"), s.get("size"))
        out[s["block_va"].lower()] = val
        for c in s.get("construct_fns") or []:
            out.setdefault(c.lower(), val)
    return out


def struct_ref(prop_or_param, struct_index):
    ref = ((prop_or_param.get("type_refs") or {}).get("struct") or "").lower()
    return struct_index.get(ref, (None, None))


def field_type(prop, struct_index):
    """Type string for a class member (layout). Struct members use their own size."""
    kind = prop.get("kind")
    if kind == "Struct":
        name, size = struct_ref(prop, struct_index)
        return ("struct:%s:%d" % (name, size)) if name and size else "undef"
    if kind in ("Map", "Set", "SoftObject", "SoftClass", "InlineMulticastDelegate", "SparseMulticastDelegate"):
        return "undef"
    return KIND_TYPE.get(kind, ("ptr:void", 8))[0]


def param_type(p, struct_index):
    """Type string for a UFunction parameter as a C value or pointer (large and by-reference types are pointers)."""
    kind = p.get("kind")
    if kind == "Struct":
        name, size = struct_ref(p, struct_index)
        if not name or not size:
            return "ptr:void"
        if size in (1, 2, 4, 8):
            return "struct:%s:%d" % (name, size)
        return "ptr:struct:%s:%d" % (name, size)
    t = KIND_TYPE.get(kind, ("ptr:void", 8))[0]
    if t in ("FString", "FText", "TArray", "FScriptDelegate"):
        return "ptr:" + t
    return t


def ufunction_params(rec):
    """(return parameter or None, input parameters) of a native_funcs.json record, without inner entries."""
    ps = [p for p in (rec.get("params") or []) if "array_inner_of" not in p and "enum_underlying_of" not in p]
    ret = next((p for p in ps if p.get("name") == "ReturnValue"), None)
    return ret, [p for p in ps if p is not ret]


def build_tables(struct_index):
    """Write name_tables/struct_layouts.json: the 8 Athena targets with their Fortnite ancestors."""
    nci = {c["name"]: c for c in load_json(os.path.join(REFL, "native_classes_index.json"))["classes"]}
    fg = load_json(os.path.join(REFL, "fortnite_game_reflection.json"))["classes_in_groups_with_fort_names"]
    props_by_block = {c["block_va"].lower(): c.get("properties") or [] for c in fg}
    layouts = {}
    for cls in ATHENA_CLASSES:
        if cls not in nci:
            continue
        chain, n = [], cls
        while n and n in nci:
            chain.append(n)
            n = nci[n].get("parent")
        own_size, bits = None, []
        entries = []
        for c in chain:
            for p in props_by_block.get(nci[c]["block_va"].lower(), []):
                if p.get("offset") is None:
                    if c == cls:
                        bits.append(p["name"])
                        own_size = own_size or p.get("size_of_outer")
                    continue
                entries.append({"offset": p["offset"], "name": p["name"], "owner": c, "kind": p.get("kind"),
                                "ctype": field_type(p, struct_index), "replicated": bool(p.get("replicated"))})
        entries.sort(key=lambda e: (e["offset"], e["name"]))
        end = max([e["offset"] for e in entries] or [0])
        size = own_size or end + 8
        fields = []
        for i, e in enumerate(entries):
            nxt = entries[i + 1]["offset"] if i + 1 < len(entries) else size
            length = nxt - e["offset"]
            fields.append({"offset": e["offset"], "name": e["name"], "owner": e["owner"], "kind": e["kind"],
                           "length": max(0, length), "ctype": e["ctype"], "replicated": e["replicated"]})
        layouts[cls] = {"block_va": nci[cls]["block_va"], "chain": chain, "size": size,
                        "size_source": "size_of_outer of an own bitfield" if own_size else "last offset + 8",
                        "bitfields_not_placed": bits, "fields": fields}
    write_json(os.path.join(TABLE_DIR, "struct_layouts.json"), {
        "_meta": {"source": "porting/generated/reflection: native_classes_index.json, fortnite_game_reflection.json, "
                            "struct_blocks.json",
                  "note": "offsets are absolute in the object, ancestor fields included. Engine classes above the "
                          "first Fortnite ancestor are not recovered: they show as _gap_ fields.",
                  "generated": datetime.datetime.now().isoformat(timespec="seconds")},
        "classes": layouts})
    return layouts


# ----------------------------------------------------------------------------------------------------------
# Portability score (text based; used for the raw and the improved output)
# ----------------------------------------------------------------------------------------------------------

def score_text(body):
    """Metrics and a 0-100 portability score (higher = closer to hand-written C++).

    score = 100 - min(40, lines/4) - min(30, 6*unnamed FUN_ calls) - min(20, 4*unnamed vtable calls)
                - min(15, 3*(goto + LAB_)) - min(15, 5*loops) - 20 if a switch / jump table
                - min(25, 50 * noise tokens / lines)
    Noise tokens are param_N, undefinedN, uStack/alStack/lStack names, local_*, xxxVarN, longlong, ulonglong.
    """
    lines = body.count("\n") + (1 if body and not body.endswith("\n") else 0)
    fun = len(re.findall(r"\bFUN_[0-9a-fA-F]+\b", body))
    vt = len(re.findall(r"\(\*\*\(code \*\*\)|\(\*\(code \*\*\)|\(\*\(code\*\*\)|\bvfn_[0-9a-f]+\b", body))
    gotos = len(re.findall(r"\bgoto\b|\bLAB_[0-9a-fA-F]+\b", body))
    loops = len(re.findall(r"\bwhile\s*\(|\bdo\s*\{|\bfor\s*\(", body))
    switch = bool(re.search(r"\bswitch\s*\(|Could not recover jumptable", body))
    noise = len(NOISE_RE.findall(body))
    score = 100.0 - min(40.0, lines / 4.0) - min(30.0, 6.0 * fun) - min(20.0, 4.0 * vt) \
        - min(15.0, 3.0 * gotos) - min(15.0, 5.0 * loops) - (20.0 if switch else 0.0) \
        - min(25.0, 50.0 * noise / max(1, lines))
    score = round(max(0.0, min(100.0, score)), 1)
    return {"lines": lines, "unnamed_calls": fun, "unresolved_vt_calls": vt, "gotos": gotos, "loops": loops,
            "switch": switch, "noise_tokens": noise, "score": score}


def verdict_of(m):
    """Automatic verdict for the before/after counts. A proxy: the sample verdicts in the notes are manual."""
    if (m["score"] >= PORTABLE_MIN and m["lines"] <= 80 and m["unnamed_calls"] == 0
            and m["unresolved_vt_calls"] == 0):
        return "portable"
    if m["score"] >= NEEDS_MIN:
        return "needs names"
    return "not usable"


# ----------------------------------------------------------------------------------------------------------
# Ghidra stage
# ----------------------------------------------------------------------------------------------------------

class Edits:
    """Ghidra edits for one pass. Created after the JVM starts (the ghidra imports need it)."""

    def __init__(self, program, flat):
        from ghidra.program.model.data import (
            ArrayDataType, BooleanDataType, ByteDataType, CategoryPath, DataTypeConflictHandler, FloatDataType,
            IntegerDataType, LongLongDataType, PointerDataType, ShortDataType, StructureDataType,
            Undefined1DataType, UnsignedIntegerDataType, UnsignedLongLongDataType, UnsignedShortDataType,
            VoidDataType)
        from ghidra.program.model.listing import CodeUnit, Function, ParameterImpl
        from ghidra.program.model.symbol import SourceType
        from java.util import ArrayList
        self.program, self.flat = program, flat
        self.fm = program.getFunctionManager()
        self.listing = program.getListing()
        self.af = program.getAddressFactory().getDefaultAddressSpace()
        self.mem = program.getMemory()
        self.dtm = program.getDataTypeManager()
        self.cat = CategoryPath("/Fortnite/UE425")
        self.conflict = DataTypeConflictHandler.REPLACE_HANDLER
        self.ArrayDataType, self.PointerDataType, self.StructureDataType = ArrayDataType, PointerDataType, StructureDataType
        self.undef1 = Undefined1DataType.dataType
        self.void = VoidDataType.dataType
        self.scalars = {"int32": IntegerDataType.dataType, "uint32": UnsignedIntegerDataType.dataType,
                        "int64": LongLongDataType.dataType, "uint64": UnsignedLongLongDataType.dataType,
                        "int16": ShortDataType.dataType, "uint16": UnsignedShortDataType.dataType,
                        "float": FloatDataType.dataType, "uint8": ByteDataType.dataType,
                        "bool": BooleanDataType.dataType}
        self.ParameterImpl, self.CodeUnit, self.Function, self.SourceType = ParameterImpl, CodeUnit, Function, SourceType
        self.ArrayList = ArrayList
        self.named = {}          # name -> managed DataType (placeholders, FFrame, class structs)
        self.log = []

    # -- addresses and functions
    def addr(self, hexstr):
        return self.af.getAddress(hexstr)

    def func_at(self, hexstr):
        return self.fm.getFunctionAt(self.addr(hexstr))

    # -- data types
    def managed(self, dt):
        return self.dtm.addDataType(dt, self.conflict)

    def struct_of(self, name, size, fields):
        """fields: (offset, name, type string, length, comment). Gaps are filled with _gap_ arrays."""
        s = self.StructureDataType(self.cat, name, 0)
        s.setPackingEnabled(False)
        cur = 0
        for off, fname, ctype, length, comment in sorted(fields, key=lambda f: f[0]):
            if length <= 0 or off < cur:
                continue
            if off > cur:
                s.add(self.ArrayDataType(self.undef1, off - cur, 1), off - cur, "_gap_0x%x" % cur, "")
            dt = ctype if not isinstance(ctype, str) else (self.dt(ctype) if ctype else None)
            if dt is None or dt.getLength() != length:
                dt = self.ArrayDataType(self.undef1, length, 1)
            try:
                s.add(dt, length, fname, comment or "")
            except Exception:  # duplicate member name in the chain: keep the offset in the name
                s.add(dt, length, "%s_0x%x" % (fname, off), comment or "")
            cur = off + length
        if size and size > cur:
            s.add(self.ArrayDataType(self.undef1, size - cur, 1), size - cur, "_tail_0x%x" % cur, "")
        return self.managed(s)

    def placeholder(self, name, size, fields):
        if name not in self.named:
            self.named[name] = self.struct_of(name, size, [(o, n, c, ln, "") for o, n, c, ln in fields])
        return self.named[name]

    def dt(self, ctype):
        """DataType for a type string (see name_tables/helpers.json _meta)."""
        if ctype in self.scalars:
            return self.scalars[ctype]
        if ctype == "ptr:void" or ctype == "void*":
            return self.PointerDataType()
        if ctype.startswith("ptr:"):
            inner = self.dt(ctype[4:]) if ctype[4:] not in ("UFunction",) else None
            if inner is None or ctype[4:] == "UFunction":
                return self.PointerDataType()
            return self.PointerDataType(inner)
        if ctype.startswith("undef["):
            return self.ArrayDataType(self.undef1, int(ctype[6:-1]), 1)
        if ctype == "undef" or not ctype:
            return None
        if ctype == "FName":
            return self.placeholder("FName", 8, [(0, "ComparisonIndex", "int32", 4), (4, "Number", "int32", 4)])
        if ctype == "FString":
            return self.placeholder("FString", 16, [(0, "Data", "ptr:void", 8), (8, "ArrayNum", "int32", 4),
                                                    (12, "ArrayMax", "int32", 4)])
        if ctype == "TArray":
            return self.placeholder("TArray", 16, [(0, "Data", "ptr:void", 8), (8, "ArrayNum", "int32", 4),
                                                   (12, "ArrayMax", "int32", 4)])
        if ctype == "FText":
            return self.placeholder("FText", 8, [(0, "TextData", "ptr:void", 8)])
        if ctype == "FScriptDelegate":
            return self.placeholder("FScriptDelegate", 16, [(0, "Object", "ptr:void", 8),
                                                            (8, "FunctionName", "FName", 8)])
        if ctype == "UObject":
            return self.placeholder("UObject", 40, [(0, "vftable", "ptr:void", 8),
                                                    (8, "ObjectFlags", "uint32", 4),
                                                    (12, "InternalIndex", "int32", 4),
                                                    (16, "ClassPrivate", "ptr:void", 8),
                                                    (24, "NamePrivate", "FName", 8),
                                                    (32, "OuterPrivate", "ptr:void", 8)])
        if ctype.startswith("struct:"):
            _, name, size = ctype.split(":")
            return self.placeholder("Struct_" + re.sub(r"\W", "_", name), int(size), [])
        if ctype in self.named:
            return self.named[ctype]
        return None

    # -- functions
    def signature(self, func, ret_dt, params):
        plist = self.ArrayList()
        for name, dt in params:
            plist.add(self.ParameterImpl(name, dt, self.program))
        try:
            func.setCallingConvention("__fastcall")
        except Exception as exc:  # keep the default convention if this name is not defined
            self.log.append("calling convention: %s" % exc)
        func.setReturnType(ret_dt, self.SourceType.USER_DEFINED)
        func.replaceParameters(plist, self.Function.FunctionUpdateType.DYNAMIC_STORAGE_FORMAL_PARAMS, True,
                               self.SourceType.USER_DEFINED)

    def rename(self, func, name):
        try:
            func.setName(name, self.SourceType.USER_DEFINED)
            return name
        except Exception:
            alt = "%s_%s" % (name, str(func.getEntryPoint()).replace(":", "_"))
            try:
                func.setName(alt, self.SourceType.USER_DEFINED)
                return alt
            except Exception as exc:
                self.log.append("rename %s: %s" % (name, exc))
                return func.getName()

    def label(self, hexstr, name):
        try:
            self.program.getSymbolTable().createLabel(self.addr(hexstr), name, self.SourceType.USER_DEFINED)
        except Exception as exc:
            self.log.append("label %s: %s" % (name, exc))

    def indirect_disps(self, func):
        """Displacements of CALL / JMP qword ptr [Rxx + disp] in address order (vtable calls and tail calls)."""
        out = []
        for ins in self.listing.getInstructions(func.getBody(), True):
            m = CALL_RE.match(ins.toString())
            if m:
                out.append(int(m.group(3) or "0", 16))
        return out

    def read_table(self, table_addr):
        """Entries of a vtable: consecutive values that point into executable memory (upper bound)."""
        out = []
        for i in range(MAX_SLOTS):
            try:
                v = self.mem.getLong(self.af.getAddress(hex(table_addr + 8 * i))) & 0xFFFFFFFFFFFFFFFF
                blk = self.mem.getBlock(self.af.getAddress(hex(v)))
                if blk is None or not blk.isExecute():
                    break
                out.append(v)
            except Exception:
                break
        return out


def create_fframe(ed):
    ffr = ed.struct_of("FFrame", FFRAME_SIZE, [(o, n, c, ln, "UE 4.25 Stack.h") for o, n, c, ln in FFRAME_FIELDS])
    ed.named["FFrame"] = ffr
    ed.named["FFrame*"] = ed.PointerDataType(ffr)


def apply_helpers(ed, helpers, summary):
    report = []
    for h in helpers["functions"]:
        f = ed.func_at(h["address"])
        if f is None:
            ed.flat.createFunction(ed.addr(h["address"]), None)
            f = ed.func_at(h["address"])
        if f is None:
            report.append({"name": h["name"], "address": h["address"], "applied": False})
            continue
        old = f.getName()
        ret = ed.void if h["returns"] == "void" else ed.dt(h["returns"])
        params = [(pn, ed.dt(pt)) for pn, pt in h["params"]]
        ed.signature(f, ret, params)
        new = ed.rename(f, h["name"])
        report.append({"name": new, "was": old, "address": h["address"], "applied": True,
                       "source": h.get("source")})
    for d in helpers["data_labels"]:
        ed.label(d["address"], d["name"])
    summary["helpers"] = report


def compute_slot_names(ed, targets, natives, summary):
    """Slot names of each verified table, from the tier-1 RPC thunks. Writes name_tables/vtable_slots_*.json."""
    result = {}
    for table_cls, table_addr in VERIFIED_TABLES.items():
        chain = table_chain(table_cls)
        names, evidence, conflicts = {}, {}, []
        for t in targets:
            if t["class"] not in chain or t.get("tier") != 1 or not t.get("exec_addr"):
                continue
            f = ed.func_at(t["exec_addr"])
            if f is None:
                continue
            disp = ed.indirect_disps(f)
            if not disp:
                continue
            fn = t["function"]
            if t.get("validate"):
                if len(disp) < 2 or disp[1] != disp[0] + 8:
                    conflicts.append("%s.%s validate pattern %s" % (t["class"], fn, [hex(d) for d in disp[:3]]))
                    continue
                pairs = [(disp[0] // 8, fn + "_Validate"), (disp[1] // 8, fn + "_Implementation")]
            else:
                pairs = [(disp[0] // 8, fn + "_Implementation")]
            for slot, nm in pairs:
                if slot in names and names[slot] != nm:
                    conflicts.append("slot %d: %s vs %s" % (slot, names[slot], nm))
                    continue
                names[slot] = nm
                evidence.setdefault(slot, []).append("%s.%s" % (t["class"], fn))
        entries = ed.read_table(VERIFIED_TABLES[table_cls])
        result[table_cls] = {"names": names, "evidence": evidence, "conflicts": conflicts, "entries": entries}
        write_json(os.path.join(TABLE_DIR, "vtable_slots_%s.json" % table_cls), {
            "class": table_cls, "table_va": hex(table_addr), "entries_read_upper_bound": len(entries),
            "source": "tier-1 RPC thunks: indirect CALL qword ptr [reg+disp] (validate at slot, implementation at "
                      "slot+1; unvalidated: implementation at slot)",
            "slots": {str(k): {"name": v, "from": evidence.get(k, [])} for k, v in sorted(names.items())},
            "conflicts": conflicts})
    return result


def apply_structs(ed, layouts, slot_info, summary):
    class_dts = {}
    for cls, lay in layouts.items():
        vptr = ed.PointerDataType()
        if cls in VERIFIED_TABLES:
            vt = vtable_struct(ed, cls, slot_info[cls])
            vptr = ed.PointerDataType(vt)
        fields = [(0, "vftable", vptr, 8, "vtable of %s" % cls)]
        for f in lay["fields"]:
            if f["offset"] == 0 or f["length"] <= 0:
                continue
            comment = "%s %s%s" % (f["owner"], f["kind"], " replicated" if f["replicated"] else "")
            fields.append((f["offset"], f["name"], f["ctype"], f["length"], comment))
        dt = ed.struct_of(cls, lay["size"], fields)
        class_dts[cls] = dt
        ed.named[cls] = dt
    summary["structs"] = sorted(class_dts)
    return class_dts


def vtable_struct(ed, cls, info):
    """Vtable struct for a verified table: one pointer field per slot (named from the RPC slot names)."""
    entries = info["entries"]
    fields = []
    for i in range(len(entries)):
        nm = info["names"].get(i, "vfn_%03x" % (8 * i))
        fields.append((8 * i, nm, "ptr:void", 8, "slot %d (+0x%x)" % (i, 8 * i)))
    return ed.struct_of(cls + "_VTable", max(8, 8 * len(entries)), fields)


def ensure_function(ed, hexstr):
    """Function at hexstr; creates one (in memory, rolled back) when the exec address has no function."""
    f = ed.func_at(hexstr)
    if f is not None:
        return f, False
    a = ed.addr(hexstr)
    if ed.listing.getInstructionAt(a) is None:
        ed.flat.disassemble(a)
    ed.flat.createFunction(a, None)
    return ed.func_at(hexstr), True


def apply_exec(ed, targets, natives, struct_index, class_dts, ffr_ptr, summary):
    """Exec prototypes: void Class_Fn(UObject* Context, FFrame* Stack, void* Result) with Context typed."""
    done, created = 0, 0
    for t in targets:
        if not t.get("exec_addr"):
            continue
        f, made = ensure_function(ed, t["exec_addr"])
        if f is None:
            continue
        created += 1 if made else 0
        cls, fn = t["class"], t["function"]
        cdt = class_dts.get(cls)
        ctx = ed.PointerDataType(cdt) if cdt is not None else ed.PointerDataType()
        ed.signature(f, ed.void, [("Context", ctx), ("Stack", ffr_ptr), ("Result", ed.PointerDataType())])
        ed.rename(f, "%s_%s" % (cls, fn))
        done += 1
    summary["exec_signatures"] = done
    summary["functions_created_in_memory"] = created


def apply_rpc_prototypes(ed, slot_info, natives, struct_index, class_dts, summary):
    """_Validate (bool) and _Implementation prototypes at the verified table entries."""
    impl_records = []
    report = {}
    for table_cls, info in slot_info.items():
        names, evidence, entries = info["names"], info["evidence"], info["entries"]
        conflicts = list(info["conflicts"])
        applied = 0
        for slot, nm in sorted(names.items()):
            if slot >= len(entries):
                conflicts.append("slot %d beyond table (%d entries)" % (slot, len(entries)))
                continue
            impl = entries[slot]
            if sum(1 for v in entries if v == impl) != 1:
                conflicts.append("slot %d (%s): entry shared with another slot" % (slot, nm))
                continue
            owner, fn = evidence[slot][0].split(".", 1)
            rec = natives.get(owner, {}).get(fn)
            if rec is None:
                continue
            f = ed.fm.getFunctionAt(ed.af.getAddress(hex(impl)))
            if f is None:
                conflicts.append("slot %d: no function at %s" % (slot, hex(impl)))
                continue
            ret, ins = ufunction_params(rec)
            this_dt = ed.PointerDataType(class_dts[table_cls]) if table_cls in class_dts else ed.PointerDataType()
            params = [("this", this_dt)]
            for i, p in enumerate(ins):
                params.append((p.get("name") or "p%d" % i, ed.dt(param_type(p, struct_index)) or ed.PointerDataType()))
            if nm.endswith("_Validate"):
                ret_dt = ed.scalars["bool"]
            elif ret is not None:
                ret_dt = ed.dt(param_type(ret, struct_index)) or ed.void
            else:
                ret_dt = ed.void
            ed.signature(f, ret_dt, params)
            ed.rename(f, "%s_%s" % (owner, nm))
            applied += 1
            kind = "validate" if nm.endswith("_Validate") else "implementation"
            impl_records.append({"class": owner, "function": nm, "kind": kind, "tier": 0, "address": hex(impl),
                                 "flags": None, "net_roles": [], "table": table_cls, "slot": slot})
        report[table_cls] = {"table": hex(VERIFIED_TABLES[table_cls]), "entries_read_upper_bound": len(entries),
                             "named_slots": len(names), "prototypes_applied": applied, "conflicts": conflicts}
    summary["vtables"] = report
    return impl_records


#: Identical bodies that the linker folded (COMDAT folding, /OPT:ICF), found by probe_callees.py. Every vtable
#: slot that holds one of these addresses has exactly this body, so the shared body IS the RPC body:
#: 0x1406801c0 "ret" (empty void), 0x1406801d0 "mov al,1; ret" (validate returning true),
#: 0x1406801e0 "xor al,al; ret" (returns false), 0x140f6b550 "cmp [rdx+8],1; setg al; ret" (FString argument
#: non-empty: ArrayNum > 1). Only a note is added; the verdict stays the proxy verdict.
FOLDED_BODIES = {0x1406801C0: "empty body", 0x1406801D0: "returns true", 0x1406801E0: "returns false",
                 0x140F6B550: "returns FString argument is not empty (ArrayNum > 1)"}


RPC_HEADER_RE = re.compile(r"^(\w+)\s+\w+\(", re.M)  # first top-level header line (FUN_xxx or a helper name)


def rename_rpc_header(code, rec):
    """Header of an RPC body: show the RPC name and its return type (a folded body keeps the shared address name).

    Ghidra names the function after its entry address (FUN_xxx) and types a folded validate as undefined1; the
    RPC is bool X_Validate(...) by its UHT declaration. Only the first header line is changed.
    """
    if rec.get("kind") not in ("validate", "implementation"):
        return code

    def sub(m):
        ret = "bool" if rec["kind"] == "validate" else m.group(1)
        return "%s %s_%s(" % (ret, rec["class"], rec["function"])

    return RPC_HEADER_RE.sub(sub, code, count=1)


def collect_unlisted_rpc(ed, slot_info, have, summary):
    """Records for named RPC slots that apply_rpc_prototypes did not decompile.

    apply_rpc_prototypes skips an entry when its address is shared with another slot, when no function exists at
    it, or when the native record is missing. These records decompile the body at the entry (creating the
    function in memory when needed) and apply NO prototype or rename: a shared entry is one body for several names.
    """
    have_keys = {(r["class"], r["function"]) for r in have}
    out, counts = [], collections.Counter()
    for table_cls, info in slot_info.items():
        entries = info["entries"]
        for slot, nm in info["names"].items():
            if slot < len(entries):
                counts[entries[slot]] += 1
    for table_cls, info in slot_info.items():
        entries = info["entries"]
        for slot, nm in sorted(info["names"].items()):
            if slot >= len(entries) or not info["evidence"].get(slot):
                continue
            owner = info["evidence"][slot][0].split(".", 1)[0]
            if (owner, nm) in have_keys:
                continue
            have_keys.add((owner, nm))
            addr = entries[slot]
            ensure_function(ed, hex(addr))
            folded = counts[addr] > 1
            out.append({"class": owner, "function": nm,
                        "kind": "validate" if nm.endswith("_Validate") else "implementation", "tier": 0,
                        "address": hex(addr), "flags": None, "net_roles": [], "table": table_cls, "slot": slot,
                        "shared_slots": counts[addr], "folded": folded, "prototype": False})
    summary["unlisted_rpc_records"] = len(out)
    summary["unlisted_rpc_folded_records"] = sum(1 for r in out if r["folded"])
    return out


def table_chain(cls):
    nci = {c["name"]: c for c in load_json(os.path.join(REFL, "native_classes_index.json"))["classes"]}
    out, n = [], cls
    while n and n in nci:
        out.append(n)
        n = nci[n].get("parent")
    return out


def run_edits(program, flat, targets, natives, struct_index, layouts, helpers):
    ed = Edits(program, flat)
    summary = {}
    create_fframe(ed)
    apply_helpers(ed, helpers, summary)
    summary["exec_functions_created"] = sum(1 for t in targets if t.get("exec_addr") and
                                            ensure_function(ed, t["exec_addr"])[1])
    slot_info = compute_slot_names(ed, targets, natives, summary)
    class_dts = apply_structs(ed, layouts, slot_info, summary)
    apply_exec(ed, targets, natives, struct_index, class_dts, ed.named["FFrame*"], summary)
    impl = apply_rpc_prototypes(ed, slot_info, natives, struct_index, class_dts, summary)
    impl += collect_unlisted_rpc(ed, slot_info, impl, summary)
    summary["edit_log"] = ed.log[:40]
    return ed, summary, impl, slot_info


def helper_xrefs(ed, helpers):
    """Call-site counts for each helper over the whole program (reference manager)."""
    rm = ed.program.getReferenceManager()
    out = {}
    for h in helpers["functions"]:
        a = ed.addr(h["address"])
        calls = sum(1 for r in rm.getReferencesTo(a) if r.getReferenceType().isCall())
        jumps = sum(1 for r in rm.getReferencesTo(a) if r.getReferenceType().isJump())
        out[h["name"]] = {"address": h["address"], "call_sites": calls, "jump_sites": jumps}
    return out



# Text stage after Ghidra (applied to every improved body). Ghidra keeps the StepCompiledIn branches and the
# vtable calls as raw expressions; these rules rewrite them to the same meaning in C++-like form.
P_FINISH_RE = re.compile(r"\*\(ulonglong \*\)Stack->Code\s*=\s*\(ulonglong\)\(\*\(longlong \*\)Stack->Code != 0\)"
                         r"\s*\+\s*\*\(longlong \*\)Stack->Code;")
FIELD_DEREF_RE = re.compile(r"\*\((?:longlong|ulonglong|undefined8|UObject|undefined|void|uint8)\s*\**\)\s*"
                            r"(\w+)->(Code|Object|Locals|vftable|PropertyChainForCompiledIn|"
                            r"MostRecentPropertyAddress|OutParms|Node)\b")
BLOCK_RE = re.compile(r"if \(Stack->Code == 0\) \{\s*(\w+) = Stack->PropertyChainForCompiledIn;\s*"
                      r"Stack->PropertyChainForCompiledIn = \*\(undefined8 \*\)\(\1 \+ 0x20\);\s*"
                      r"FFrame_StepExplicitProperty\(Stack,([^,]+),\1\);\s*\}\s*else \{\s*"
                      r"FFrame_Step\(Stack,[^,]+,\2\);\s*\}")
VCALL_RE = re.compile(r"\(\*\*\(code \*\*\)\((\w+)->vftable \+ 0x([0-9a-f]+)\)\)\s*\((\1(?:,\s*|\))|\))")
CAST_RE = re.compile(r"\((?:undefined\d*|void|UObject|ushort)\s*\**\)\s*")


CAST_SIZE = {"undefined": 1, "undefined1": 1, "char": 1, "uchar": 1, "byte": 1, "bool": 1, "undefined2": 2,
             "short": 2, "ushort": 2, "undefined4": 4, "int": 4, "uint": 4, "float": 4, "undefined8": 8,
             "longlong": 8, "ulonglong": 8}
RESULT_CAST_RE = re.compile(r"\*\((undefined\d?|char|uchar|byte|bool|short|ushort|int|uint|float|longlong|ulonglong)"
                            r"\s*\*\)Result\b")
FIELD_CAST_RE = re.compile(r"\*\((undefined\d?|char|uchar|byte|bool|short|ushort|int|uint|float|longlong|ulonglong|"
                           r"UObject)(\s*\**)\)\s*(Context|this)->(\w+)")


def normalize_code(code, ins, slot_names, ret_ctype=None, field_sizes=None):
    """Rewrite Ghidra idioms to readable C++-like form. Returns (text, info). ins = UFunction input params.
    ret_ctype: C type of ReturnValue (exec functions write the result through Result). field_sizes: member name ->
    size from the class layout; a scalar cast of the same size is dropped (Context->Field)."""
    info = {"p_finish": 0, "field_derefs": 0, "fetch_blocks": 0, "fetch_params": len(ins), "vcalls": 0,
            "renamed_locals": [], "result_casts": 0, "field_casts": 0}
    text, n = P_FINISH_RE.subn("P_FINISH;", code)
    info["p_finish"] = n
    text, n = FIELD_DEREF_RE.subn(r"\1->\2", text)
    info["field_derefs"] = n
    blocks = list(BLOCK_RE.finditer(text))
    info["fetch_blocks"] = len(blocks)
    pieces, last, idents = [], 0, []
    for i, m in enumerate(blocks):
        dest = CAST_RE.sub("", m.group(2).strip())
        ident = dest[1:] if dest.startswith("&") else dest
        idents.append(ident)
        pieces.append(text[last:m.start()])
        pieces.append("Stack->StepCompiledIn(&%s);  /*@P%d@*/" % (ident, i))
        last = m.end()
    pieces.append(text[last:])
    text = "".join(pieces)
    for i, ident in enumerate(idents):
        if i >= len(ins) or not re.fullmatch(r"\w+", ident):
            continue
        pname = ins[i]["name"]
        if re.search(r"\b%s\b" % re.escape(pname), text):
            continue  # name already used: keep the local name
        text = re.sub(r"\b%s\b" % re.escape(ident), pname, text)
        info["renamed_locals"].append([ident, pname])
    for i, p in enumerate(ins[:len(blocks)]):
        text = text.replace("/*@P%d@*/" % i, "/* P%d %s: %s */" % (i, p["name"], _ctype_text(p)))
    for i in range(len(ins), len(blocks)):
        text = text.replace("/*@P%d@*/" % i, "/* unmatched fetch */")
    if len(blocks) != len(ins):
        info["fetch_mismatch"] = "blocks=%d params=%d" % (len(blocks), len(ins))

    def vcall(m):
        off = int(m.group(2), 16)
        name = slot_names.get(off // 8, "vfn_%03x" % off)
        info["vcalls"] += 1
        return "%s->%s(%s" % (m.group(1), name, "" if "," in m.group(3) else ")")
    text = VCALL_RE.sub(vcall, text)
    text = text.replace("(ushort *)L\"", "L\"")
    if ret_ctype and ret_ctype in CTYPE_C and CTYPE_SIZE.get(ret_ctype):
        want = CTYPE_SIZE[ret_ctype]

        def result_cast(m):
            if CAST_SIZE.get(m.group(1)) != want:
                return m.group(0)
            info["result_casts"] += 1
            return "*(%s *)Result" % CTYPE_C[ret_ctype]
        text = RESULT_CAST_RE.sub(result_cast, text)
    if field_sizes:
        def field_cast(m):
            ctype_cast, stars, this, field = m.group(1), m.group(2), m.group(3), m.group(4)
            size = 8 if "**" in stars else CAST_SIZE.get(ctype_cast)
            if field in field_sizes and size == field_sizes[field] and size is not None:
                info["field_casts"] += 1
                return "%s->%s" % (this, field)
            return m.group(0)
        text = FIELD_CAST_RE.sub(field_cast, text)
    return text, info


CTYPE_C = {"int32": "int32_t", "uint32": "uint32_t", "int64": "int64_t", "uint64": "uint64_t", "int16": "int16_t",
           "uint16": "uint16_t", "float": "float", "uint8": "uint8_t", "bool": "bool"}
CTYPE_SIZE = {"int32": 4, "uint32": 4, "int64": 8, "uint64": 8, "int16": 2, "uint16": 2, "float": 4, "uint8": 1,
              "bool": 1}





def _ret_ctype(natives, rec):
    ret, _ins = ufunction_params(natives.get(rec["class"], {}).get(rec["function"], {}))
    return _ctype_text(ret) if ret is not None else None


def _field_sizes(layouts, cls):
    """member name -> size for same-size casts: the scalar size of a scalar member, else its layout length."""
    lay = layouts.get(cls)
    out = {}
    for f in (lay["fields"] if lay else []):
        ct = f["ctype"]
        if ct in SCALAR_SIZE:
            out[f["name"]] = SCALAR_SIZE[ct]
        elif ct in ("FName", "FString", "TArray", "FText", "FScriptDelegate") or ct.startswith("ptr:"):
            out[f["name"]] = 8 if ct.startswith("ptr:") or ct == "FName" else None
        else:
            out[f["name"]] = f["length"]
    return {k: v for k, v in out.items() if v}


def _ctype_text(p):
    """Readable C type of a parameter for comments: Vector, FString*, uint8, UObject*."""
    t = param_type(p, _STRUCT_INDEX[0]) if _STRUCT_INDEX else p.get("kind")
    ptr = t.startswith("ptr:")
    if ptr:
        t = t[4:]
    if t.startswith("ptr:"):
        t = t[4:]
    m = re.match(r"^struct:(\w+):(\d+)$", t)
    if m:
        t = m.group(1)
    elif t.startswith("undef["):
        t = "bytes%s" % t[6:-1]
    return t + ("*" if ptr else "")


_STRUCT_INDEX = []


def slot_names_for(cls, slot_info):
    """Slot names of the verified table whose class chain contains cls (same slots in derived tables)."""
    for table_cls, info in slot_info.items():
        if cls in table_chain(table_cls):
            return info["names"]
    return {}


def decompile_all(ed, program, records, timeout, slot_info):
    from ghidra.app.decompiler import DecompInterface, DecompileOptions
    from ghidra.util.task import TaskMonitor
    from ghidra.program.model.data import StringDataInstance
    opts = DecompileOptions()
    opts.grabFromProgram(program)
    decomp = DecompInterface()
    decomp.setOptions(opts)
    decomp.toggleCCode(True)
    decomp.toggleSyntaxTree(True)
    decomp.setSimplificationStyle("decompile")
    if not decomp.openProgram(program):
        raise SystemExit("DecompInterface.openProgram failed: " + str(decomp.getLastMessage()))
    monitor = TaskMonitor.DUMMY
    rows = []
    for rec in records:
        t0 = time.time()
        row = {k: rec.get(k) for k in ("tier", "class", "function", "address", "flags", "net_roles", "kind",
                                       "table", "slot", "shared_slots", "folded")}
        row["file"] = "decomp_improved/" + rec["relpath"].replace(os.sep, "/")
        f = ed.fm.getFunctionAt(ed.addr(rec["address"]))
        if f is None:
            row.update({"status": "failed", "error": "no function", "lines": 0})
            rows.append(row)
            continue
        res = decomp.decompileFunction(f, timeout, monitor)
        if res is None or res.isTimedOut() or not res.decompileCompleted():
            row.update({"status": "timeout" if res is not None and res.isTimedOut() else "failed",
                        "error": str(res.getErrorMessage()) if res is not None else "no result", "lines": 0})
            rows.append(row)
            continue
        code = str(res.getDecompiledFunction().getC())
        ins = rec.get("_ins") or []
        code, ninfo = normalize_code(code, ins, slot_names_for(rec["class"], slot_info),
                                     ret_ctype=rec.get("_ret_ctype"), field_sizes=rec.get("_fields"))
        code = rename_rpc_header(code, rec)
        called, indirect, strings = base.analyze_body(program, f, StringDataInstance)
        m = score_text(code)
        row.update({"status": "ok", "lines": m["lines"], "called": called, "indirect_calls": indirect,
                    "strings": strings, "ghidra_name": f.getName(), "seconds": round(time.time() - t0, 2),
                    "metrics": m, "verdict": verdict_of(m), "normalize": ninfo})
        if rec.get("folded"):
            # Identical code folded by the linker: this body is the body of every slot listed in the note.
            body = FOLDED_BODIES.get(int(rec["address"], 16), "identical body")
            row["note"] = "folded body at %s shared by %d vtable slots (%s)" % (rec["address"], rec["shared_slots"], body)
        if "indirect jump" in code:
            row["note"] = (row.get("note") or "") + " tail jump through a vtable slot (thunk, not a body)"
        full = os.path.join(rec["_out"], rec["relpath"])
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "w", encoding="utf-8") as fh:
            fh.write(improved_header(rec))
            fh.write(code)
        rows.append(row)
    decomp.dispose()
    return rows


def improved_header(rec):
    lines = [
        "/*",
        " * Improved Ghidra decompiler output (porting/ghidra/improve_decompile.py), derived from the user's client.",
        " * Git-ignored. Helper names, FFrame, class structs, vtable slots and prototypes applied in a rolled-back",
        " * transaction. Raw output: porting/generated/decomp/.",
        " * Class:     %s" % rec["class"],
        " * Function:  %s" % rec["function"],
        " * Kind:      %s" % rec.get("kind", "exec"),
        " * Tier:      %s (%s)" % (rec.get("tier"), TIER_NAMES.get(rec.get("tier"), "unknown")),
        " * Address:   %s" % rec.get("address"),
        " */",
    ]
    return "\n".join(lines) + "\n"


def rank_records(rows, raw_index):
    """Portability ranking. Value bonus: RPC validate/implementation pairs, getters/setters, small bodies."""
    out = []
    for r in rows:
        if r.get("status") != "ok":
            continue
        name = r["function"]
        bonus = 0.0
        if r.get("kind") in ("validate", "implementation") or name.endswith(("_Validate", "_Implementation")):
            bonus += 3.0
        if re.match(r"^(Get|Set|Is|Has|On)", name.split(".")[-1]) or re.search(r"_(Get|Set|Is|Has)", name):
            bonus += 3.0
        if r.get("flags") and int(r["flags"], 16) & 0x40:
            bonus += 2.0
        raw = raw_index.get(r["class"] + "." + r["function"])
        rec = {"class": r["class"], "function": r["function"], "kind": r.get("kind", "exec"), "tier": r.get("tier"),
               "address": r.get("address"), "file": r["file"], "metrics": r["metrics"], "verdict": r["verdict"],
               "value_bonus": bonus, "rank_key": round(r["metrics"]["score"] + bonus, 1),
               "before": None}
        if raw is not None:
            rec["before"] = {"lines": raw.get("lines"), "status": raw.get("status")}
        rec["verdict"] = verdict_of(r["metrics"])
        out.append(rec)
    out.sort(key=lambda x: (-x["rank_key"], x["metrics"]["lines"], x["class"], x["function"]))
    for i, rec in enumerate(out, 1):
        rec["rank"] = i
    return out


def before_after(rows, raw_index):
    """Verdict counts before (raw Ghidra output) and after (improved) for the exec targets, plus per sample."""
    raw = raw_metrics(raw_index)
    before, after = collections.Counter(), collections.Counter()
    per = {}
    for r in rows:
        if r.get("status") != "ok" or r.get("kind") != "exec":
            continue
        key = r["class"] + "." + r["function"]
        after[verdict_of(r["metrics"])] += 1
        b = raw.get(key)
        if b is not None:
            before[b["verdict"]] += 1
        per[key] = {"before": b and {"score": b["metrics"]["score"], "lines": b["metrics"]["lines"],
                                     "verdict": b["verdict"]},
                    "after": {"score": r["metrics"]["score"], "lines": r["metrics"]["lines"],
                              "verdict": verdict_of(r["metrics"])}}
    return {"exec_targets_before": dict(before), "exec_targets_after": dict(after), "per_function": per}


def top_candidates(ranked):
    """Top 60 with quotas: 25 getters/setters/predicates, 20 RPC functions, 15 others.
    Each group is taken in rank order and only from portable or needs-names records with <= 40 lines."""
    def is_value(r):
        return r["kind"] == "exec" and re.match(r"^(Get|Set|Is|Has|On|Can|Should)", r["function"]) is not None

    impl_keys = {(r["class"], r["function"]) for r in ranked if r["kind"] in ("validate", "implementation")}

    def is_rpc_pair(r):
        if r["kind"] != "exec":
            return False
        return ((r["class"], r["function"] + "_Implementation") in impl_keys
                or (r["class"], r["function"] + "_Validate") in impl_keys)
    pool = [r for r in ranked if r["metrics"]["lines"] <= 40 and r["verdict"] in ("portable", "needs names")]
    picked, seen = [], set()
    for name, pred, quota in (("getter/setter/predicate", is_value, 25), ("RPC (validate pair where present)", is_rpc_pair, 20),
                              ("other small body", lambda r: True, 15)):
        n = 0
        for r in pool:
            key = (r["class"], r["function"], r["kind"])
            if key in seen or not pred(r):
                continue
            seen.add(key)
            picked.append(dict(r, category=name))
            n += 1
            if n >= quota:
                break
    return picked


def write_ranking(rows, raw_index, out_root):
    ranked = rank_records(rows, raw_index)
    top = top_candidates(ranked)
    write_json(os.path.join(RAW_DIR, "portability.json"), {
        "_meta": {"generated": datetime.datetime.now().isoformat(timespec="seconds"),
                  "score": "see porting/ghidra/improve_decompile.py score_text (0-100, higher = more portable)",
                  "verdict": "portable: score >= %g, <= 80 lines, no unnamed FUN_ call, no unnamed vtable call; "
                             "needs names: score >= %g; otherwise not usable" % (PORTABLE_MIN, NEEDS_MIN),
                  "value_bonus": "+3 RPC _Validate/_Implementation, +3 Get/Set/Is/Has/On names, +2 FUNC_Net",
                  "records": len(ranked),
                  "verdict_counts": dict(collections.Counter(r["verdict"] for r in ranked)),
                  "top60_quota": "25 getter/setter/predicate, 20 RPC (validate pair where present), 15 other"},
        "top60": top,
        "ranking": ranked})
    return ranked


def raw_index_map():
    path = os.path.join(RAW_DIR, "index.json")
    if not os.path.exists(path):
        return {}
    data = load_json(path)
    out = {}
    for r in data.get("functions", []):
        out[r["class"] + "." + r["function"]] = r
    return out


def raw_metrics(raw_index):
    """Metrics of the raw (pre-improvement) bodies, same scoring, for before/after counts."""
    out = {}
    for key, r in raw_index.items():
        if r.get("status") != "ok" or not r.get("file"):
            continue
        p = os.path.join(GENERATED, r["file"])
        if not os.path.exists(p):
            continue
        with open(p, encoding="utf-8") as fh:
            text = fh.read()
        m = score_text(text)
        out[key] = {"metrics": m, "verdict": verdict_of(m)}
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(description="Improve Athena decompile inputs and re-decompile (see module doc)")
    ap.add_argument("--build-tables", action="store_true", help="rebuild name_tables/struct_layouts.json first")
    ap.add_argument("--only", action="append", default=[], help="Class.Function (repeatable): restrict decompile")
    ap.add_argument("--out", default=IMPROVED_DIR, help="output root (default porting/generated/decomp_improved)")
    ap.add_argument("--timeout", type=int, default=60, help="per-function decompiler timeout, seconds")
    ap.add_argument("--mem", default=os.environ.get("GHIDRA_HEADLESS_MAXMEM", "48G"), help="JVM max heap")
    ap.add_argument("--no-decompile", action="store_true", help="apply the edits, skip decompilation")
    ap.add_argument("--rank-only", action="store_true", help="rank the existing improved outputs")
    args = ap.parse_args(argv if argv is not None else sys.argv[1:])
    out_root = os.path.abspath(args.out)
    t0 = time.time()

    if args.rank_only:
        idx = load_json(os.path.join(out_root, "index.json"))["functions"]
        ranked = write_ranking(idx, raw_index_map(), out_root)
        ba = before_after(idx, raw_index_map())
        write_json(os.path.join(RAW_DIR, "before_after.json"), ba)
        _log("ranked %d records; exec before %s after %s" % (len(ranked), ba["exec_targets_before"],
                                                              ba["exec_targets_after"]))
        return 0

    targets_all = base.load_targets()
    targets = [t for t in targets_all if t.get("exec_addr")]
    if args.only:
        wanted = set(args.only)
        selected = [t for t in targets if t["class"] + "." + t["function"] in wanted]
    else:
        selected = [t for t in targets if t.get("tier") in (1, 2, 3, 4)]
    natives = load_json(os.path.join(REFL, "native_funcs.json"))
    struct_index = struct_index_of(load_json(os.path.join(REFL, "struct_blocks.json")))
    _STRUCT_INDEX[:] = [struct_index]
    if args.build_tables or not os.path.exists(os.path.join(TABLE_DIR, "struct_layouts.json")):
        build_tables(struct_index)
    layouts = load_json(os.path.join(TABLE_DIR, "struct_layouts.json"))["classes"]
    helpers = load_json(os.path.join(TABLE_DIR, "helpers.json"))

    import pyghidra
    from pyghidra.launcher import HeadlessPyGhidraLauncher
    launcher = HeadlessPyGhidraLauncher(install_dir=INSTALL)
    launcher.add_vmargs("-Xmx%s" % args.mem)
    launcher.start()
    from ghidra.program.flatapi import FlatProgramAPI  # noqa: E402 (needs the JVM started)

    project_root = os.path.join(PROJECT_DIR, PROJECT + ".rep")
    project_gpr = os.path.join(PROJECT_DIR, PROJECT + ".gpr")
    before = base.snapshot([project_root, project_gpr])
    project = pyghidra.open_project(PROJECT_DIR, PROJECT, create=False)
    program, consumer = pyghidra.consume_program(project, PROGRAM_PATH)
    tx = program.startTransaction("improve_decompile in-memory (rolled back)")
    summary, rows, xrefs, slot_info = {}, [], {}, {}
    try:
        flat = FlatProgramAPI(program)
        ed, summary, impl, slot_info = run_edits(program, flat, targets_all, natives, struct_index, layouts,
                                                 helpers)
        xrefs = helper_xrefs(ed, helpers)
        if not args.no_decompile:
            for rec in selected:
                rec.update({"kind": "exec", "address": rec["exec_addr"],
                            "relpath": os.path.join(base.safe_name(rec["class"]),
                                                    base.safe_name(rec["function"]) + ".c"),
                            "_out": out_root,
                            "_ins": ufunction_params(natives.get(rec["class"], {}).get(rec["function"], {}))[1],
                            "_ret_ctype": _ret_ctype(natives, rec),
                            "_fields": _field_sizes(layouts, rec["class"])})
            wanted_impl = [i for i in impl if not args.only or (
                i["class"] + "." + re.sub(r"_(Validate|Implementation)$", "", i["function"])) in set(args.only)]
            for i in wanted_impl:
                i["relpath"] = os.path.join(base.safe_name(i["class"]), base.safe_name(i["function"]) + ".c")
                i["_out"] = out_root
                i["_ins"] = []  # RPC bodies take C parameters: no FFrame fetch to map
                i["_fields"] = _field_sizes(layouts, i["class"])
            rows = decompile_all(ed, program, selected + wanted_impl, args.timeout, slot_info)
            summary["decompiled"] = collections.Counter(r["status"] for r in rows)
            summary["decompiled"] = dict(summary["decompiled"])
            norm = [r.get("normalize") or {} for r in rows]
            summary["normalize_totals"] = {k: sum(n.get(k, 0) for n in norm)
                                           for k in ("p_finish", "field_derefs", "fetch_blocks", "vcalls")}
            summary["normalize_renamed_locals"] = sum(len(n.get("renamed_locals", [])) for n in norm)
            summary["fetch_mismatch"] = ["%s.%s %s" % (r["class"], r["function"], r["normalize"]["fetch_mismatch"])
                                         for r in rows if "fetch_mismatch" in (r.get("normalize") or {})]
    finally:
        program.endTransaction(tx, False)  # roll back: nothing is committed
        program.release(consumer)
        project.close()
    changed = base.snapshot_diff(before, base.snapshot([project_root, project_gpr]))

    if rows:
        write_json(os.path.join(out_root, "index.json"), {
            "_meta": {"generated": datetime.datetime.now().isoformat(timespec="seconds"),
                      "elapsed_s": round(time.time() - t0, 1), "only": args.only, "summary": {
                          k: v for k, v in summary.items() if k not in ("edit_log",)},
                      "helper_xrefs": xrefs, "project_files_changed": len(changed), "rolled_back": True},
            "functions": rows})
        if not args.only:
            write_ranking(rows, raw_index_map(), out_root)
            ba = before_after(rows, raw_index_map())
            write_json(os.path.join(RAW_DIR, "before_after.json"), ba)
            summary["before_after"] = {k: v for k, v in ba.items() if k != "per_function"}
    _log("SUMMARY " + json.dumps({"elapsed_s": round(time.time() - t0, 1), "counts": summary.get("decompiled"),
                                  "exec_signatures": summary.get("exec_signatures"),
                                  "exec_functions_created": summary.get("exec_functions_created"),
                                  "normalize_totals": summary.get("normalize_totals"),
                                  "normalize_renamed_locals": summary.get("normalize_renamed_locals"),
                                  "fetch_mismatch": summary.get("fetch_mismatch"),
                                  "vtables": {k: {kk: vv for kk, vv in v.items() if kk != "conflicts"}
                                              for k, v in summary.get("vtables", {}).items()},
                                  "project_files_changed": len(changed)}, default=str))
    if changed:
        _log("WARNING project files changed on disk: " + ", ".join(changed[:10]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
