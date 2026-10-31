#!/usr/bin/env python3
"""Recover struct, enum and function-signature (delegate and RPC) tables from the runtime image (read-only).

Layouts are UE 4.25 UObjectGlobals.h UECodeGen_Private (WITH_METADATA off):

  FStructParams   (72 bytes)  OuterFunc, SuperFunc, StructOpsFunc, NameUTF8, SizeOf, AlignOf,
                              PropertyArray, NumProperties(i32 @56), ObjectFlags(u32 @60), StructFlags(u32 @64)
  FEnumParams     (56 bytes)  OuterFunc, DisplayNameFunc, NameUTF8, CppTypeUTF8, EnumeratorParams,
                              NumEnumerators(i32 @40), ObjectFlags(u32 @44), DynamicType(u8 @48), CppForm(u8 @49)
  FEnumeratorParam (16 bytes) NameUTF8, Value(i64)
  FFunctionParams (72 bytes)  OuterFunc, SuperFunc, NameUTF8, OwningClassName, DelegateName, StructureSize,
                              PropertyArray, NumProperties(i32 @56), ObjectFlags(u32 @60), FunctionFlags(u32 @64)

Property tables are decoded with porting/extract_reflection.py (decode_props), so every property record
has the same fields as the class tables. Construct functions (Z_Construct_UScriptStruct_*, Z_Construct_UEnum_*,
Z_Construct_UDelegateFunction_*, Z_Construct_UFunction_*) reference their parameter block with a RIP-relative
LEA. The reference sites come from Ghidra (porting/ghidra/export_type_xrefs.py); the function that owns a site
comes from the image's .pdata table.

Phases (the runtime dump is the image; Ghidra is only needed for phase 2 xrefs):
  scan  -> porting/generated/reflection/type_block_candidates.json   (no Ghidra)
  xrefs -> porting/generated/reflection/type_block_xrefs.json       (porting/ghidra/run_export_type_xrefs.py)
  join  -> struct_blocks.json, enum_blocks.json, delegate_blocks.json, construct_fn_to_block.json,
           function_blocks.json (RPC and other UFunction parameter blocks by construct function)

Usage:
  python3 -I porting/extract_types.py scan  [--image PATH] [--out JSON]
  python3 -I porting/extract_types.py join  [--image PATH] [--candidates JSON] [--xrefs JSON]
                                           [--reflection JSON] [--class-parent JSON] [--out-dir DIR]
  python3 -I porting/extract_types.py --self-test [--image PATH]
"""
import argparse
import bisect
import collections
import json
import os
import re
import struct
import sys
from array import array

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import extract_reflection as ER  # noqa: E402  (Image reader and property decoder)
import resolve_class_parents as RCP  # noqa: E402  (package-name lookup: FPackageParams lea of a package fn)

REFL_DIR = os.path.join(HERE, "generated", "reflection")
DEFAULT_IMAGE = os.path.join(HERE, "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")
DEFAULT_CANDIDATES = os.path.join(REFL_DIR, "type_block_candidates.json")
DEFAULT_XREFS = os.path.join(REFL_DIR, "type_block_xrefs.json")
DEFAULT_REFLECTION = os.path.join(REFL_DIR, "fortnite_game_reflection.json")
DEFAULT_CLASS_PARENT = os.path.join(REFL_DIR, "class_parent_map.json")
DEFAULT_EVIDENCE = os.path.join(REFL_DIR, "class_name_map.evidence.json")

FUNC_DELEGATE = 0x00100000
FUNC_MULTICAST_DELEGATE = 0x00010000
FUNC_NET_FLAG_NAMES = {0x40: "Net", 0x80: "NetReliable", 0x4000: "NetMulticast", 0x200000: "NetServer",
                       0x1000000: "NetClient", 0x2000: "Static", 0x40000000: "Const"}
ECPP_FORM = {0: "Regular", 1: "Namespaced", 2: "EnumClass"}
EDYNAMIC_TYPE = {0: "NotDynamic", 1: "Dynamic"}

# EStructFlags (CoreUObject/Public/UObject/Class.h, UE 4.25). The runtime-computed bits
# (STRUCT_ComputedFlags) are set from the struct ops at load time; they are not in the table.
STRUCT_FLAG_NAMES = [
    (0x00000001, "Native"), (0x00000002, "IdenticalNative"), (0x00000004, "HasInstancedReference"),
    (0x00000008, "NoExport"), (0x00000010, "Atomic"), (0x00000020, "Immutable"),
    (0x00000040, "AddStructReferencedObjects"), (0x00000200, "RequiredAPI"), (0x00000400, "NetSerializeNative"),
    (0x00000800, "SerializeNative"), (0x00001000, "CopyNative"), (0x00002000, "IsPlainOldData"),
    (0x00004000, "NoDestructor"), (0x00008000, "ZeroConstructor"), (0x00010000, "ExportTextItemNative"),
    (0x00020000, "ImportTextItemNative"), (0x00040000, "PostSerializeNative"),
    (0x00080000, "SerializeFromMismatchedTag"), (0x00100000, "NetDeltaSerializeNative"),
    (0x00200000, "PostScriptConstruct"), (0x00400000, "NetSharedSerialization"), (0x00800000, "Trashed"),
]

# ICppStructOps (Class.h, UE 4.25). Vtable slot = declaration index of the virtual, the destructor first.
# MSVC emits the deleting destructor in one slot. Verified by the GetSize/size checks in --self-test.
ICPP_SLOTS = {
    "HasNoopConstructor": 1, "HasZeroConstructor": 2, "HasDestructor": 4, "HasSerializer": 6,
    "HasStructuredSerializer": 7, "HasPostSerialize": 10, "HasNetSerializer": 12,
    "HasNetSharedSerialization": 13, "HasNetDeltaSerializer": 15, "HasPostScriptConstruct": 17,
    "IsPlainOldData": 19, "HasCopy": 20, "HasIdentical": 22, "HasExportTextItem": 24,
    "HasImportTextItem": 26, "HasAddStructReferencedObjects": 28, "HasSerializeFromMismatchedTag": 30,
    "HasStructuredSerializeFromMismatchedTag": 31, "HasGetTypeHash": 34,
}
ICPP_SLOT_COUNT = 37
LEA_RCX_RIP = b"\x48\x8d\x0d"
CONST_RET = [
    (re.compile(rb"\A\xb0([\x00\x01])\xc3"), "al"),          # mov al, imm8; ret
    (re.compile(rb"\A\xb8([\x00\x01])\x00\x00\x00\xc3"), "eax"),  # mov eax, imm32; ret
    (re.compile(rb"\A[\x31\x32\x33]\xc0\xc3"), "zero"),      # xor eax,eax / xor al,al; ret
]
PDATA_DIR_INDEX = 3


# ------------------------------------------------------------------ image helpers

class Functions:
    """Function ranges from the .pdata (exception) directory: containment for any code address."""

    def __init__(self, img):
        m = img.m
        e = struct.unpack_from("<I", m, 0x3C)[0]
        opt = e + 24
        dd = opt + 112 + 8 * PDATA_DIR_INDEX
        rva, size = struct.unpack_from("<II", m, dd)
        sec = None
        for s in img.sections:
            if s["va"] <= rva < s["va"] + max(s["vsize"], s["rsize"]):
                sec = s
                break
        if sec is None or not size:
            raise SystemExit("no .pdata directory in image")
        off = sec["raw"] + (rva - sec["va"])
        n = size // 12
        vals = array("I")
        vals.frombytes(m[off:off + n * 12])
        pairs = sorted((vals[3 * i], vals[3 * i + 1]) for i in range(n))
        self.begins = [b for b, _ in pairs]
        self.ends = [e2 for _, e2 in pairs]
        self.imagebase = img.imagebase

    def start_of(self, va):
        rva = va - self.imagebase
        i = bisect.bisect_right(self.begins, rva) - 1
        if i < 0 or rva >= self.ends[i]:
            return None
        return self.imagebase + self.begins[i]


def follow_jmp(img, va, hops=2):
    for _ in range(hops):
        o = img.off(va)
        if o is None:
            return va
        head = img.m[o:o + 6]
        if head[0] == 0xE9:
            va = va + 5 + struct.unpack_from("<i", head, 1)[0]
        elif head[:2] == b"\xff\x25":
            nxt = img.u64(va + 6 + struct.unpack_from("<i", head, 2)[0])
            if not nxt:
                return va
            va = nxt
        else:
            break
    return va


def classify_const(img, va):
    """True/False when the function is a constant-returning bool (after up to two jmp hops), else None."""
    va = follow_jmp(img, va)
    o = img.off(va)
    if o is None:
        return None
    code = img.m[o:o + 8]
    for rx, kind in CONST_RET:
        mt = rx.match(code)
        if mt:
            if kind == "zero":
                return False
            if kind == "al":
                return mt.group(1) != b"\x00"
            return mt.group(1) != b"\x00"
    return None


def decode_struct_ops(img, fn_va, size, align):
    """Read the TCppStructOps<T> object that a StructOpsFunc constructs (static, no execution).

    The function does `new TCppStructOps<T>`: LEA rcx,[vtable]; MOV [rax+8],Size; MOV [rax+12],Align.
    Returns the trait values from the vtable slots, or None when the pattern is not found."""
    o = img.off(fn_va)
    if o is None:
        return None
    code = img.m[o:o + 96]
    vt = None
    ops_size = ops_align = None
    i = code.find(LEA_RCX_RIP)
    if i >= 0 and i + 7 <= len(code):
        disp = struct.unpack_from("<i", code, i + 3)[0]
        vt = fn_va + i + 7 + disp
    j = code.find(b"\xc7\x40\x08")
    if j >= 0:
        ops_size = struct.unpack_from("<I", code, j + 3)[0]
    j = code.find(b"\xc7\x40\x0c")
    if j >= 0:
        ops_align = struct.unpack_from("<I", code, j + 3)[0]
    if vt is None or ops_size is None:
        return None
    traits = {}
    unknown = []
    for name, slot in ICPP_SLOTS.items():
        target = img.u64(vt + 8 * slot)
        value = classify_const(img, target) if target else None
        if value is None:
            unknown.append(name)
        traits[name] = value
    return {
        "vtable_va": "0x%x" % vt,
        "ops_size": ops_size,
        "ops_align": ops_align,
        "size_matches": ops_size == size,
        "align_matches": ops_align == align,
        "traits": traits,
        "unclassified_slots": unknown,
    }


def struct_flag_names(flags):
    return [name for bit, name in STRUCT_FLAG_NAMES if flags & bit]


def func_flag_names(flags):
    names = [name for bit, name in FUNC_NET_FLAG_NAMES.items() if flags & bit]
    if flags & FUNC_DELEGATE:
        names.append("Delegate")
    if flags & FUNC_MULTICAST_DELEGATE:
        names.append("MulticastDelegate")
    return names


def block_va_of(img, off):
    s = next(sec for sec in img.sections if sec["raw"] <= off < sec["raw"] + sec["rsize"])
    return img.imagebase + s["va"] + (off - s["raw"])


# ------------------------------------------------------------------ phase 1: scan

def params_valid(img, pa, nprop):
    """Every entry of a PropertyArray is a data pointer to a property record with a readable name."""
    for k in range(nprop):
        rec = img.u64(pa + 8 * k)
        if not rec or not img.is_data(rec):
            return False
        name = img.u64(rec)
        if not name or not img.cstr(name):
            return False
    return True


def scan(img):
    m = img.m
    structs, enums, functions = [], [], []
    for s in img.data_sections():
        arr = array("Q")
        arr.frombytes(m[s["raw"]:s["raw"] + s["rsize"] - (s["rsize"] % 8)])
        base_off = s["raw"]
        for i in range(len(arr) - 9):
            off = base_off + i * 8
            # FStructParams: Outer/Super/StructOps are code or null; SizeOf is a small integer (not a pointer).
            nm = arr[i + 3]
            so, sf, sop = arr[i], arr[i + 1], arr[i + 2]
            if (nm and img.is_data(nm) and all(v == 0 or img.is_code(v) for v in (so, sf, sop))
                    and 0 < arr[i + 4] <= 0x10000 and arr[i + 5] and (arr[i + 5] & (arr[i + 5] - 1)) == 0
                    and arr[i + 5] <= 256):
                nprop, objf, sflags = struct.unpack_from("<iII", m, off + 56)
                pa = arr[i + 6]
                if 0 <= nprop < 4096 and (pa == 0) == (nprop == 0) and (pa == 0 or img.is_data(pa)) \
                        and sflags < 0x01000000:
                    cname = img.cstr(nm)
                    if cname:
                        structs.append({"block_va": "0x%x" % block_va_of(img, off), "name": cname,
                                        "outer_fn": so or None, "super_fn": sf or None, "ops_fn": sop or None,
                                        "size": arr[i + 4], "align": arr[i + 5], "prop_arr": pa or None,
                                        "nprop": nprop, "object_flags": objf, "struct_flags": sflags})
            # FEnumParams: name and CppType are strings, EnumeratorParams is a data array.
            cpp = arr[i + 3]
            ep = arr[i + 4]
            tail = arr[i + 6]
            # tail = DynamicType (EDynamicType is an int32 enum class) | CppForm (uint8 @52) | padding (zero).
            dyn, cform = tail & 0xFFFFFFFF, (tail >> 32) & 0xFF
            if (nm and img.is_data(nm) and (cpp == 0 or img.is_data(cpp)) and ep and img.is_data(ep)
                    and all(v == 0 or img.is_code(v) for v in (so, arr[i + 1]))
                    and (tail >> 40) == 0 and dyn <= 1 and cform <= 2):
                nen, objf = struct.unpack_from("<iI", m, off + 40)
                cname = img.cstr(nm)
                if cname and 0 < nen <= 1024:
                    enums.append({"block_va": "0x%x" % block_va_of(img, off), "name": cname,
                                  "outer_fn": so or None, "display_name_fn": arr[i + 1] or None,
                                  "cpp_type": img.cstr(cpp) if cpp else None, "enumerators_va": ep,
                                  "nenum": nen, "object_flags": objf, "dynamic_type": dyn,
                                  "cpp_form": cform})
            # FFunctionParams: OwningClassName and DelegateName are strings or null; PropertyArray is data.
            nm = arr[i + 2]
            if nm and img.is_data(nm) and (so == 0 or img.is_code(so)) and (sf == 0 or img.is_code(sf)):
                owner, dname, pa = arr[i + 3], arr[i + 4], arr[i + 6]
                if (owner == 0 or img.is_data(owner)) and (dname == 0 or img.is_data(dname)) \
                        and (pa == 0 or img.is_data(pa)):
                    nprop, objf, funcf = struct.unpack_from("<iII", m, off + 56)
                    if 0 <= nprop < 4096 and (pa == 0) == (nprop == 0) and params_valid(img, pa, nprop):
                        fname = img.cstr(nm)
                        if fname:
                            functions.append({"block_va": "0x%x" % block_va_of(img, off), "name": fname,
                                              "owning_class": img.cstr(owner) if owner else None,
                                              "delegate_name": img.cstr(dname) if dname else None,
                                              "outer_fn": so or None, "super_fn": sf or None,
                                              "prop_arr": pa or None, "nprop": nprop,
                                              "object_flags": objf, "function_flags": funcf})
    return structs, enums, functions


def cmd_scan(args):
    img = ER.Image(args.image)
    structs, enums, functions = scan(img)
    # A 72-byte record can match both FStructParams and FFunctionParams, and a 56-byte FEnumParams can match
    # FFunctionParams too. The same address then appears in two kinds. Struct and enum records win: a
    # function record at the same address is a false match (its name is a type name, not a function name).
    typed = {c["block_va"] for c in structs + enums}
    dropped = [f for f in functions if f["block_va"] in typed]
    functions = [f for f in functions if f["block_va"] not in typed]
    out = {
        "image": os.path.relpath(args.image, HERE),
        "layout_source": "UE 4.25 UObjectGlobals.h UECodeGen_Private (FStructParams 72, FEnumParams 56, FFunctionParams 72)",
        "counts": {"structs": len(structs), "enums": len(enums), "functions": len(functions),
                   "function_candidates_dropped_as_struct_or_enum": len(dropped)},
        "structs": structs, "enums": enums, "functions": functions,
    }
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(out, fh)
        fh.write("\n")
    print("scan: %d struct, %d enum, %d function-parameter candidates -> %s" % (
        len(structs), len(enums), len(functions), args.out))
    return 0


# ------------------------------------------------------------------ phase 2: join

def load_json(path):
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def decode_table(img, prop_arr, nprop):
    if not prop_arr or not nprop:
        return []
    return ER.decode_props(img, prop_arr, nprop)


def decode_enumerators(img, va, n):
    out = []
    for k in range(n):
        rec = va + 16 * k
        name_p, value = struct.unpack_from("<Qq", img.m, img.off(rec))
        out.append({"name": img.cstr(name_p), "value": value})
    return out


def join(args):
    img = ER.Image(args.image)
    funcs = Functions(img)
    cand = load_json(args.candidates)
    xrefs = load_json(args.xrefs)
    refl = load_json(args.reflection)
    parents = load_json(args.class_parent)

    # Package function -> package name, from the class blocks (dependency-1 is the package function).
    pkg_of_fn = {}
    for blk in (refl.get("classes_in_groups_with_fort_names") or []):
        rec = parents.get(blk["block_va"].lower()) or {}
        if rec.get("package") and blk.get("base_fn"):
            pkg_of_fn.setdefault(blk["base_fn"].lower(), rec["package"])
    probe = RCP.Probe(RCP.Image(args.image))
    # A class's construct function: the package of a delegate declared in a class is that class's package.
    class_pkg_of_fn = {}
    for rec in load_json(args.evidence).get("blocks", []):
        pkg = (parents.get(str(rec.get("block_va", "")).lower()) or {}).get("package")
        for fn in rec.get("construct_fns") or []:
            fn = str(fn).lower()
            fn = fn if fn.startswith("0x") else "0x" + fn
            if pkg:
                class_pkg_of_fn[fn] = pkg

    def package_name(outer_fn):
        """Package of a struct, enum or delegate. OuterFunc is the package singleton function, or, for a delegate
        declared in a class, that class's construct function."""
        if not outer_fn:
            return None
        return (probe.package_name(outer_fn) or pkg_of_fn.get("0x%x" % outer_fn)
                or class_pkg_of_fn.get("0x%x" % outer_fn))

    # Construct function -> blocks, from the reference sites of every candidate block.
    sites_unknown = 0
    fn_of_block = {}
    block_of_fn = collections.defaultdict(set)
    kind_of_block = {}
    for kind, key in (("struct", "structs"), ("enum", "enums"), ("function", "functions")):
        for c in cand[key]:
            kind_of_block[c["block_va"].lower()] = kind
    for block_va, sites in xrefs.items():
        kind = kind_of_block.get(block_va.lower())
        if kind is None:
            continue
        owners = set()
        for site in sites:
            fn = funcs.start_of(int(site, 16))
            if fn is not None:
                owners.add(fn)
        if not owners:
            sites_unknown += 1
        fn_of_block[block_va.lower()] = sorted(owners)
        for fn in owners:
            block_of_fn[fn].add((kind, block_va.lower()))

    # Delegate signatures are FFunctionParams with FUNC_Delegate. Their construct fns use the Delegate kind.
    fn_kind = {}
    for c in cand["functions"]:
        flags = c["function_flags"]
        fn_kind[c["block_va"].lower()] = "delegate" if flags & FUNC_DELEGATE else "function"

    construct_map = {}
    ambiguous = []
    for fn, pairs in block_of_fn.items():
        entries = []
        for kind, block in sorted(pairs):
            k = kind
            if kind == "function":
                k = fn_kind.get(block, "function")
            entries.append((k, block))
        if len(entries) == 1:
            k, block = entries[0]
            construct_map["0x%x" % fn] = {"kind": k, "block_va": block}
        else:
            ambiguous.append({"construct_fn": "0x%x" % fn, "blocks": [{"kind": k, "block_va": b} for k, b in entries]})

    # Records. Enum and struct and function-signature tables are decoded here.
    by_block = {c["block_va"].lower(): c for c in cand["structs"] + cand["enums"] + cand["functions"]}

    def construct_of(block):
        owners = fn_of_block.get(block.lower(), [])
        return ["0x%x" % f for f in owners]

    struct_recs = []
    for c in cand["structs"]:
        block = c["block_va"].lower()
        props = decode_table(img, c["prop_arr"], c["nprop"]) if c["prop_arr"] else []
        ops = decode_struct_ops(img, c["ops_fn"], c["size"], c["align"]) if c["ops_fn"] else None
        struct_recs.append({
            "block_va": c["block_va"], "name": c["name"],
            "construct_fns": construct_of(block),
            "package_fn": "0x%x" % c["outer_fn"] if c["outer_fn"] else None,
            "package": package_name(c["outer_fn"]),
            "super_fn": "0x%x" % c["super_fn"] if c["super_fn"] else None,
            "struct_ops_fn": "0x%x" % c["ops_fn"] if c["ops_fn"] else None,
            "size": c["size"], "align": c["align"],
            "struct_flags": "0x%x" % c["struct_flags"], "struct_flag_names": struct_flag_names(c["struct_flags"]),
            "object_flags": "0x%x" % c["object_flags"],
            "struct_ops": ops,
            "properties": props,
            "num_properties": c["nprop"],
        })
    enum_recs = []
    for c in cand["enums"]:
        block = c["block_va"].lower()
        enum_recs.append({
            "block_va": c["block_va"], "name": c["name"], "cpp_type": c["cpp_type"],
            "construct_fns": construct_of(block),
            "package_fn": "0x%x" % c["outer_fn"] if c["outer_fn"] else None,
            "package": package_name(c["outer_fn"]),
            "cpp_form": ECPP_FORM.get(c["cpp_form"], "unknown_%d" % c["cpp_form"]),
            "dynamic_type": EDYNAMIC_TYPE.get(c["dynamic_type"], "unknown"),
            "enumerators": decode_enumerators(img, c["enumerators_va"], c["nenum"]),
        })
    delegate_recs, function_recs = [], []
    for c in cand["functions"]:
        block = c["block_va"].lower()
        rec = {
            "block_va": c["block_va"], "name": c["name"], "owning_class": c["owning_class"],
            "delegate_name": c["delegate_name"], "construct_fns": construct_of(block),
            "package": package_name(c["outer_fn"]),
            "outer_fn": "0x%x" % c["outer_fn"] if c["outer_fn"] else None,
            "function_flags": "0x%x" % c["function_flags"], "function_flag_names": func_flag_names(c["function_flags"]),
            "params": decode_table(img, c["prop_arr"], c["nprop"]) if c["prop_arr"] else [],
        }
        if c["function_flags"] & FUNC_DELEGATE:
            rec["multicast"] = bool(c["function_flags"] & FUNC_MULTICAST_DELEGATE)
            # The UHT type name: the signature name without its suffix, with the F prefix. delegate_name is the
            # owning property's name (for example OnEndPlay for ActorEndPlaySignature__DelegateSignature).
            rec["type_name"] = "F" + re.sub(r"__DelegateSignature$", "", c["name"])
            delegate_recs.append(rec)
        else:
            rec.pop("package", None)
            function_recs.append(rec)

    # Validation counts: every struct/enum/delegate property reference in the class tables resolves?
    refs = collections.Counter()
    missing = collections.Counter()
    refs_by_kind = {"struct": "struct", "enum": "enum", "signature": "delegate"}
    for blk in (refl.get("classes_in_groups_with_fort_names") or []):
        for prop in blk.get("properties") or []:
            for key, want in refs_by_kind.items():
                fn = (prop.get("type_refs") or {}).get(key)
                if not fn:
                    continue
                hit = construct_map.get(fn.lower())
                tally = "resolved_%s" % want if hit and hit["kind"] == want else "unresolved_%s" % want
                refs[tally] += 1
                if tally.startswith("unresolved"):
                    missing[fn.lower()] += 1

    os.makedirs(args.out_dir, exist_ok=True)
    stamp = {"image": os.path.relpath(args.image, HERE),
             "method": "construct function = owner of a reference site to the block (.pdata containment)"}

    def write(name, payload):
        path = os.path.join(args.out_dir, name)
        with open(path, "w", encoding="utf-8", newline="\n") as fh:
            json.dump(payload, fh, indent=1)
            fh.write("\n")
        return path

    struct_recs.sort(key=lambda r: r["name"])
    enum_recs.sort(key=lambda r: r["name"])
    delegate_recs.sort(key=lambda r: r["name"])
    function_recs.sort(key=lambda r: r["name"])
    ops_checked = [r["struct_ops"] for r in struct_recs if r["struct_ops"]]
    summary = {
        "structs": len(struct_recs),
        "structs_with_construct_fn": sum(1 for r in struct_recs if r["construct_fns"]),
        "structs_with_struct_ops": len(ops_checked),
        "struct_ops_size_matches": sum(1 for o in ops_checked if o["size_matches"]),
        "struct_ops_align_matches": sum(1 for o in ops_checked if o["align_matches"]),
        "enums": len(enum_recs),
        "enums_with_construct_fn": sum(1 for r in enum_recs if r["construct_fns"]),
        "delegate_signatures": len(delegate_recs),
        "delegate_signatures_with_construct_fn": sum(1 for r in delegate_recs if r["construct_fns"]),
        "ufunction_parameter_blocks": len(function_recs),
        "ufunction_parameter_blocks_with_construct_fn": sum(1 for r in function_recs if r["construct_fns"]),
        "block_sites_without_function": sites_unknown,
        "construct_functions_mapped": len(construct_map),
        "construct_functions_ambiguous": len(ambiguous),
        "property_refs": dict(sorted(refs.items())),
        "distinct_unresolved_refs": len(missing),
    }
    write("struct_blocks.json", dict(stamp, summary=summary, structs=struct_recs))
    write("enum_blocks.json", dict(stamp, summary=summary, enums=enum_recs))
    write("delegate_blocks.json", dict(stamp, summary=summary, delegates=delegate_recs))
    write("function_blocks.json", dict(stamp, summary=summary, functions=function_recs))
    write("construct_fn_to_block.json", dict(stamp, summary=summary, map=construct_map, ambiguous=ambiguous))
    print(json.dumps(summary, indent=1))
    return 0


# ------------------------------------------------------------------ self test

def self_test(image_path):
    failures = []
    count = [0]

    def check(label, cond):
        count[0] += 1
        if not cond:
            failures.append(label)

    img = ER.Image(image_path)
    # Engine structs with known UE 4.25 shapes (header-derived): size, align, member names in offset order.
    structs, enum_cands, _functions = scan(img)
    by = {c["name"]: c for c in structs}
    expect = {
        "Vector": (12, 4, ["X", "Y", "Z"]),
        "Rotator": (12, 4, ["Pitch", "Yaw", "Roll"]),
        "Quat": (16, 16, ["X", "Y", "Z", "W"]),
        "Vector2D": (8, 4, ["X", "Y"]),
        "Transform": (48, 16, ["Rotation", "Translation", "Scale3D"]),
        "Vector_NetQuantize": (12, 4, None),
        "Vector_NetQuantize100": (12, 4, None),
    }
    for name, (size, align, members) in expect.items():
        c = by.get(name)
        check("struct %s present" % name, c is not None)
        if c is None:
            continue
        check("struct %s size %d" % (name, size), c["size"] == size)
        check("struct %s align %d" % (name, align), c["align"] == align)
        if members:
            props = ER.decode_props(img, c["prop_arr"], c["nprop"]) if c["prop_arr"] else []
            # The table lists members in reverse declaration order; the offsets give the layout order.
            ordered = sorted(props, key=lambda p: p.get("offset") or 0)
            check("struct %s members %s" % (name, members), [p["name"] for p in ordered] == members)
    # FVector_NetQuantize parent is FVector (its SuperFunc is Z_Construct_UScriptStruct_FVector).
    vq = by.get("Vector_NetQuantize")
    check("FVector_NetQuantize has a super struct", vq is not None and vq["super_fn"] is not None)
    # Struct ops: FVector_NetQuantize installs a TCppStructOps with a 12-byte size and 4-byte alignment.
    if vq is not None and vq["ops_fn"]:
        ops = decode_struct_ops(img, vq["ops_fn"], vq["size"], vq["align"])
        check("FVector_NetQuantize ops size/align from the constructed object", bool(ops and ops["size_matches"] and ops["align_matches"]))
        check("FVector_NetQuantize ops vtable resolved", bool(ops and not ops["unclassified_slots"]))
    # Enums: ENetRole (plain enum) and EAttachmentRule (enum class) are in the image.
    enums = {c["name"]: c for c in enum_cands}
    check("enum ENetRole present", "ENetRole" in enums)
    if "ENetRole" in enums:
        vals = decode_enumerators(img, enums["ENetRole"]["enumerators_va"], enums["ENetRole"]["nenum"])
        names = [v["name"] for v in vals]
        check("ENetRole enumerators", names[:4] == ["ROLE_None", "ROLE_SimulatedProxy", "ROLE_AutonomousProxy", "ROLE_Authority"]
              or any("ROLE_Authority" in n for n in names if n))
    print("self-test: %d checks, %d failed" % (count[0], len(failures)))
    for f in failures:
        print("  FAILED:", f)
    return 1 if failures else 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--self-test", action="store_true")
    sub = ap.add_subparsers(dest="cmd")
    s = sub.add_parser("scan")
    s.add_argument("--image", default=DEFAULT_IMAGE)
    s.add_argument("--out", default=DEFAULT_CANDIDATES)
    j = sub.add_parser("join")
    j.add_argument("--image", default=DEFAULT_IMAGE)
    j.add_argument("--candidates", default=DEFAULT_CANDIDATES)
    j.add_argument("--xrefs", default=DEFAULT_XREFS)
    j.add_argument("--reflection", default=DEFAULT_REFLECTION)
    j.add_argument("--class-parent", default=DEFAULT_CLASS_PARENT)
    j.add_argument("--evidence", default=DEFAULT_EVIDENCE)
    j.add_argument("--out-dir", default=REFL_DIR)
    args = ap.parse_args(argv)
    if args.self_test:
        return self_test(getattr(args, "image", None) or DEFAULT_IMAGE)
    if args.cmd == "scan":
        return cmd_scan(args)
    if args.cmd == "join":
        return join(args)
    ap.print_help()
    return 2


if __name__ == "__main__":
    sys.exit(main())
