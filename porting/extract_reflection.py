#!/usr/bin/env python3
"""Recover UHT reflection tables from the shipping client exe (read-only).

Reads FortniteClient-Win64-Shipping.exe through mmap and never writes to the
Fortnite/ tree. It scans for UE 4.25 UECodeGen_Private records that the
engine emits as plain static data (the code that references them is not
readable on disk; see porting/binary_analysis.md):

  FClassParams            -> class blocks (dependencies, function and property tables)
  FClassFunctionLinkInfo  -> function names (inline array inside a class block)
  FPropertyParamsBase     -> property name, EPropertyFlags, kind, offset (per type)
  FFunctionParams         -> UFunction flags (joined by NameUTF8 pointer)
  FPackageParams          -> module package record ("/Script/FortniteGame")

Layouts come from the UE 4.25.0 headers (UObjectGlobals.h, Class.h,
ObjectMacros.h, Script.h). Shipping builds have WITH_METADATA off, so the
metadata fields are not expected.

Output (ignored by git): porting/generated/reflection/fortnite_game_reflection.json
(the file name is historical; module membership is not established, see module_membership)

Class names are NOT in the exe as plaintext next to these records. Each block
is emitted with name null unless --name-map supplies a block-VA-to-name JSON.

Usage:
  python3 -I porting/extract_reflection.py [--exe PATH] [--out PATH] [--name-map JSON]
"""
import argparse
import collections
import json
import mmap
import os
import re
import struct
import sys
from array import array

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_EXE = os.path.join(REPO, "Fortnite", "FortniteGame", "Binaries", "Win64",
                           "FortniteClient-Win64-Shipping.exe")
DEFAULT_OUT = os.path.join(REPO, "porting", "generated", "reflection", "fortnite_game_reflection.json")

PROP_KIND = {
    0x00: "Byte", 0x01: "Int8", 0x02: "Int16", 0x03: "Int", 0x04: "Int64", 0x05: "UInt16",
    0x06: "UInt32", 0x07: "UInt64", 0x08: "UnsizedInt", 0x09: "UnsizedUInt", 0x0A: "Float",
    0x0B: "Double", 0x0C: "Bool", 0x0D: "SoftClass", 0x0E: "WeakObject", 0x0F: "LazyObject",
    0x10: "SoftObject", 0x11: "Class", 0x12: "Object", 0x13: "Interface", 0x14: "Name",
    0x15: "Str", 0x16: "Array", 0x17: "Map", 0x18: "Set", 0x19: "Struct", 0x1A: "Delegate",
    0x1B: "InlineMulticastDelegate", 0x1C: "SparseMulticastDelegate", 0x1D: "Text", 0x1E: "Enum",
    0x1F: "FieldPath",
}
# Kinds whose param struct carries a trailing Offset (FPropertyParamsBaseWithOffset).
# Bool params have ElementSize/SizeOfOuter/SetBitFunc instead (no offset).
CPF_NET = 0x0000000000000020
CPF_REPNOTIFY = 0x0000000100000000
FUNC_NET = 0x00000040
FUNC_NET_FLAGS = {
    "Reliable": 0x00000080, "Response": 0x00001000, "Multicast": 0x00004000,
    "Server": 0x00200000, "Client": 0x01000000,
}
CLASS_NATIVE = 0x00000080
TARGETS = ["FortWorldSettings", "FortGameStateAthena", "FortPlayerStateAthena",
           "FortPlayerControllerAthena", "FortPlayerPawnAthena", "FortAthenaMapInfo",
           "FortPoiManager", "FortWorldManager"]
FORT_TOKEN = re.compile(r"Fort|Athena")


class Image:
    def __init__(self, path):
        self.f = open(path, "rb")
        self.m = mmap.mmap(self.f.fileno(), 0, access=mmap.ACCESS_READ)
        m = self.m
        if m[0:2] != b"MZ":
            raise SystemExit("not a PE file")
        e = struct.unpack_from("<I", m, 0x3C)[0]
        if m[e:e + 4] != b"PE\0\0":
            raise SystemExit("bad PE signature")
        nsec = struct.unpack_from("<H", m, e + 6)[0]
        opt_size = struct.unpack_from("<H", m, e + 20)[0]
        opt = e + 24
        if struct.unpack_from("<H", m, opt)[0] != 0x20B:
            raise SystemExit("expected PE32+ (64-bit) image")
        self.imagebase = struct.unpack_from("<Q", m, opt + 24)[0]
        self.sections = []
        so = opt + opt_size
        for i in range(nsec):
            o = so + 40 * i
            name = m[o:o + 8].rstrip(b"\0").decode("latin1")
            vsize = struct.unpack_from("<I", m, o + 8)[0]
            va = struct.unpack_from("<I", m, o + 12)[0]
            rsize = struct.unpack_from("<I", m, o + 16)[0]
            raw = struct.unpack_from("<I", m, o + 20)[0]
            chars = struct.unpack_from("<I", m, o + 36)[0]
            self.sections.append(dict(name=name, vsize=vsize, va=va, rsize=rsize, raw=raw,
                                      exec=bool(chars & 0x20000000), read=bool(chars & 0x40000000)))

    def sec_of(self, va):
        rva = va - self.imagebase
        for s in self.sections:
            if s["va"] <= rva < s["va"] + max(s["vsize"], s["rsize"]):
                return s
        return None

    def off(self, va):
        s = self.sec_of(va)
        if s is None:
            return None
        r = va - self.imagebase - s["va"]
        return s["raw"] + r if r < s["rsize"] else None

    def u64(self, va):
        o = self.off(va) if va else None
        return struct.unpack_from("<Q", self.m, o)[0] if o is not None else None

    def cstr(self, va, maxlen=256):
        if not va:
            return None
        o = self.off(va)
        if o is None:
            return None
        e = self.m.find(b"\0", o, o + maxlen)
        if e < 0:
            return None
        raw = self.m[o:e]
        if not raw or not all(32 <= b < 127 for b in raw):
            return None
        return raw.decode("ascii")

    def is_code(self, va):
        s = self.sec_of(va)
        return s is not None and s["exec"]

    def is_data(self, va):
        s = self.sec_of(va)
        return s is not None and not s["exec"] and s["read"]

    def data_sections(self):
        return [s for s in self.sections if not s["exec"] and s["read"] and s["rsize"] >= 8]


def scan_class_blocks(img):
    """Return file offsets of FClassParams candidates (80-byte, metadata-free layout)."""
    hits = []
    m = img.m
    for s in img.data_sections():
        arr = array("Q")
        arr.frombytes(m[s["raw"]:s["raw"] + s["rsize"] - (s["rsize"] % 8)])
        for i in range(len(arr) - 9):
            p0 = arr[i]
            if not p0 or not img.is_code(p0):
                continue
            nDep, nFunc, nProp, nIface, cflags, _ = struct.unpack_from("<iiiiII", m, s["raw"] + i * 8 + 56)
            if not (0 <= nDep < 64 and 0 <= nFunc < 4096 and 0 <= nProp < 4096 and 0 <= nIface < 64):
                continue
            if not (cflags & CLASS_NATIVE):
                continue
            if (nDep == 0) != (arr[i + 3] == 0) or (nFunc == 0) != (arr[i + 4] == 0):
                continue
            if (nProp == 0) != (arr[i + 5] == 0) or (nIface == 0) != (arr[i + 6] == 0):
                continue
            if arr[i + 1] and not img.is_data(arr[i + 1]):
                continue
            if arr[i + 2] and not img.is_data(arr[i + 2]):
                continue
            if not all(arr[i + k] == 0 or img.is_data(arr[i + k]) or img.is_code(arr[i + k]) for k in (3, 4, 5, 6)):
                continue
            hits.append(s["raw"] + i * 8)
    return hits


def scan_function_params(img):
    """FFunctionParams is 72 bytes: 7 pointers, NumProperties, ObjectFlags, FunctionFlags, RPCId, RPCResponseId.
    Returns dict NameUTF8 VA -> list of (block VA, FunctionFlags)."""
    m = img.m
    by_name = collections.defaultdict(list)
    for s in img.data_sections():
        arr = array("Q")
        arr.frombytes(m[s["raw"]:s["raw"] + s["rsize"] - (s["rsize"] % 8)])
        base_va = img.imagebase + s["va"]
        for i in range(len(arr) - 9):
            name = arr[i + 2]
            if not name or not img.is_data(name):
                continue
            if arr[i + 6] and not img.is_data(arr[i + 6]):
                continue
            if arr[i + 0] and not img.is_code(arr[i + 0]):
                continue
            if arr[i + 1] and not img.is_code(arr[i + 1]):
                continue
            nprop, objf, funcf = struct.unpack_from("<III", m, s["raw"] + i * 8 + 56)
            if nprop > 4096 or (nprop == 0) != (arr[i + 6] == 0):
                continue
            if not img.cstr(name):
                continue
            by_name[name].append((base_va + i * 8, funcf))
    return by_name


def decode_props(img, prop_arr_va, nprop):
    props = []
    arr_va = prop_arr_va
    for i in range(nprop):
        pp = img.u64(arr_va + 8 * i)
        o = img.off(pp) if pp else None
        if o is None:
            props.append({"error": "unreadable param pointer", "index": i})
            continue
        name_va, rep_va, pflags, gflags, oflags, adim = struct.unpack_from("<QQQIII", img.m, o)
        kind_id = gflags & 0x1F
        kind = PROP_KIND.get(kind_id, "unknown_%d" % kind_id)
        entry = {
            "index": i,
            "name": img.cstr(name_va),
            "kind": kind,
            "kind_raw": "0x%x" % gflags,
            "property_flags": "0x%x" % pflags,
            "replicated": bool(pflags & CPF_NET),
            "rep_notify": img.cstr(rep_va) if rep_va else None,
            "array_dim": adim,
            "object_flags": "0x%x" % oflags,
        }
        if kind == "Bool":
            elem, outer = struct.unpack_from("<IQ", img.m, o + 36)
            entry["element_size"] = elem
            entry["size_of_outer"] = outer
            entry["offset"] = None
        else:
            entry["offset"] = struct.unpack_from("<i", img.m, o + 36)[0]
        props.append(entry)
    # array inner entries follow their outer Array property with the same name and no flags
    for j in range(1, len(props)):
        a, b = props[j - 1], props[j]
        if a.get("kind") == "Array" and b.get("name") == a.get("name") and not b.get("replicated"):
            b["array_inner_of"] = a.get("name")
    return props


def decode_class(img, block_off, fn_by_name):
    m = img.m
    s = next(sec for sec in img.sections if sec["raw"] <= block_off < sec["raw"] + sec["rsize"])
    block_va = img.imagebase + s["va"] + (block_off - s["raw"])
    cnf, cfg, cpp, dep_p, fn_p, prop_p, if_p = struct.unpack_from("<QQQQQQQ", m, block_off)
    nDep, nFunc, nProp, nIface, cflags, _ = struct.unpack_from("<iiiiII", m, block_off + 56)
    deps = []
    if dep_p:
        for i in range(nDep):
            deps.append(img.u64(dep_p + 8 * i))
    funcs = []
    if fn_p:
        for i in range(nFunc):
            fo = img.off(fn_p + 16 * i)
            cfn, nameva = struct.unpack_from("<QQ", m, fo)
            name = img.cstr(nameva)
            cands = fn_by_name.get(nameva, [])
            flags = sorted({f for _, f in cands})
            entry = {"name": name, "create_func": "0x%x" % cfn, "flags": ["0x%x" % f for f in flags]}
            if len(flags) == 1:
                f = flags[0]
                entry["replicated_function"] = bool(f & FUNC_NET)
                entry["net_roles"] = [k for k, bit in FUNC_NET_FLAGS.items() if f & bit]
            elif not flags:
                entry["replicated_function"] = None
                entry["note"] = "no FFunctionParams joined by name pointer"
            else:
                entry["replicated_function"] = None
                entry["note"] = "ambiguous: several FFunctionParams share this name pointer"
            funcs.append(entry)
    props = decode_props(img, prop_p, nProp) if prop_p else []
    cfg_name = img.cstr(cfg) if cfg else None
    return {
        "block_va": "0x%x" % block_va,
        "name": None,
        "name_status": "unresolved",
        "config_name": cfg_name,
        "class_flags": "0x%x" % cflags,
        "dependency_fns": ["0x%x" % d for d in deps if d],
        "package_fn": "0x%x" % deps[0] if deps and deps[0] else None,
        "base_fn": "0x%x" % deps[1] if len(deps) > 1 and deps[1] else None,
        "class_noregister_fn": "0x%x" % cnf,
        "interface_count": nIface,
        "properties": props,
        "replicated_properties": [p["name"] for p in props if p.get("replicated") and "array_inner_of" not in p],
        "functions": funcs,
        "replicated_functions": [f["name"] for f in funcs if f.get("replicated_function")],
    }


def find_package(img, name):
    """FPackageParams: NameUTF8, SingletonFuncArray, NumSingletons(i32), PackageFlags, BodyCRC, DeclarationsCRC."""
    hits = []
    for s in img.data_sections():
        for o in range(s["raw"], s["raw"] + s["rsize"] - 32, 8):
            p = struct.unpack_from("<Q", img.m, o)[0]
            if p and img.cstr(p) == name:
                hits.append(img.imagebase + s["va"] + (o - s["raw"]))
    return hits


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", default=DEFAULT_EXE)
    ap.add_argument("--out", default=DEFAULT_OUT)
    ap.add_argument("--name-map", default=None, help="optional JSON {block_va: class_name}")
    args = ap.parse_args()

    img = Image(args.exe)
    print("sections:", [s["name"] for s in img.sections], file=sys.stderr)
    cand_offs = scan_class_blocks(img)
    print("class block candidates:", len(cand_offs), file=sys.stderr)
    fn_by_name = scan_function_params(img)
    print("FFunctionParams name pointers:", len(fn_by_name), file=sys.stderr)

    # Group class blocks by dependency 0 (the package singleton of the owning module, inferred).
    # Module membership is NOT established by this script: see porting/binary_analysis.md.
    groups = collections.defaultdict(list)
    for o in cand_offs:
        dep_p = struct.unpack_from("<Q", img.m, o + 24)[0]
        ndep = struct.unpack_from("<i", img.m, o + 56)[0]
        dep0 = img.u64(dep_p) if (ndep and dep_p) else None
        groups[dep0].append(o)

    pkg_hits = find_package(img, "/Script/FortniteGame")
    pkg_info = None
    if pkg_hits:
        pva = pkg_hits[0]
        po = img.off(pva)
        arr_p, nsing = struct.unpack_from("<QI", img.m, po + 8)
        pkg_info = {"package_params_va": "0x%x" % pva, "singleton_array_va": "0x%x" % arr_p,
                    "declared_singletons": nsing}

    names = {}
    if args.name_map:
        with open(args.name_map, "r", encoding="utf-8") as fh:
            names = {k.lower(): v for k, v in json.load(fh).items()}

    summary = []
    classes = []
    for key, offs in sorted(groups.items(), key=lambda kv: -len(kv[1])):
        if key is None or len(offs) < 20:
            continue
        blocks = [decode_class(img, o, fn_by_name) for o in offs]
        fort_hits = 0
        rep_blocks = 0
        for blk in blocks:
            names_here = [f["name"] for f in blk["functions"] if f["name"]] + \
                         [p["name"] for p in blk["properties"] if p.get("name")]
            fort_hits += sum(1 for n in names_here if FORT_TOKEN.search(n))
            rep_blocks += 1 if blk["replicated_properties"] else 0
        summary.append({"dependency0_fn": "0x%x" % key, "class_blocks": len(offs),
                        "fort_or_athena_names": fort_hits, "blocks_with_replicated_props": rep_blocks})
        if fort_hits == 0:
            continue
        for blk in blocks:
            if blk["block_va"].lower() in names:
                blk["name"] = names[blk["block_va"].lower()]
                blk["name_status"] = "from --name-map"
            blk["dependency0_fn"] = "0x%x" % key
            classes.append(blk)
    classes.sort(key=lambda c: int(c["block_va"], 16))

    target_status = {
        t: {"status": "unresolved",
            "reason": ("no plaintext link from a class name to its FClassParams record was found; "
                       "the code that names it (Z_Construct_UClass_*, CompiledInDefer) is encrypted on disk")}
        for t in TARGETS
    }
    out = {
        "exe": args.exe,
        "image_base": "0x%x" % img.imagebase,
        "layout_source": "UE 4.25.0 UObjectGlobals.h / Class.h / ObjectMacros.h / Script.h, WITH_METADATA off",
        "module_membership": "unresolved: dependency0 groups are not mapped to module names by this script",
        "package_record_/Script/FortniteGame": pkg_info,
        "all_class_candidates": len(cand_offs),
        "dependency0_groups": summary,
        "targets": target_status,
        "classes_in_groups_with_fort_names": classes,
    }
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as fh:
        json.dump(out, fh, indent=1)
    rep = sum(len(c["replicated_properties"]) for c in classes)
    print("wrote %s: %d blocks in %d groups with Fort/Athena names, %d replicated properties" % (
        args.out, len(classes), sum(1 for g in summary if g["fort_or_athena_names"]), rep), file=sys.stderr)


if __name__ == "__main__":
    main()
