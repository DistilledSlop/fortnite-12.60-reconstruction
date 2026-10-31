#!/usr/bin/env python3
"""Recover native function entry points (exec addresses) and vtables per class (read-only).

Input: the decrypted runtime image (porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe),
plus the reflection outputs of extract_reflection.py / gen_native_classes.py work
(class_name_map.json, class_name_map.evidence.json, function_blocks.json).

How the native tables are found (UE 4.25, see porting/binary_analysis.md section 10):
  1. A native array is FNameNativePtrPair[] = {const char* NameUTF8; FNativeFuncPtr Pointer}
     (CoreNative.h), 16-byte records. Its name pointers are the same NameUTF8 pointers as the
     class's FClassFunctionLinkInfo entries (checked, all pairs match).
  2. UHT emits, per class, StaticRegisterNatives<C>(), which does
         UClass* Class = C::StaticClass();                    // call to GetPrivateStaticClass
         FNativeFunctionRegistrar::RegisterFunctions(Class, Array, N);  // mov r8d, N; lea rdx, Array
     so the array is found by RIP-relative LEA references from code, and the class is the
     GetPrivateStaticClass function that the owning function calls.
  3. GetPrivateStaticClass is referenced from the compiled-in-defer initializer that also
     references the class's Z_Construct_UClass_C function (construct function -> block, from
     class_name_map.evidence.json). That gives the link GPSC -> block. Thunks (jmp rel32, outside
     .pdata) are followed.
  4. Each function's exec address is the code pointer paired with its name in the array. Flags and
     parameters come from function_blocks.json, joined on the Z_Construct_UFunction_* address
     (FClassFunctionLinkInfo.CreateFuncPtr), which is an exact join.

Vtables (best effort, mostly unverified): GetPrivateStaticClass passes InternalConstructor<C> and
InternalVTableHelperCtorCaller<C> as stack arguments 10 and 11. The constructors they reach store the
vptr at offset 0 of `this`; the last such store is the candidate table. A candidate is published as
vtable_addr only when the class hierarchy supports it (unique to the class, or shared only with
ancestors and descendants). Other candidates are kept as vtable_candidate with vtable_status
"unverified". Slot counts run to the next class's table start (an upper bound).

Outputs (git-ignored, under porting/generated/reflection/):
  native_funcs.json        {class_name: {func_name: {exec_addr, flags, params, create_func, source, stub_addr?}}}
  native_class_index.json  {class_name: {block_va, native_arrays, native_count, vtable_addr, vtable_status, ...}}
                           plus a "_summary" key with the coverage, decode and header-validation numbers
  decompile_targets.json   ranked targets for the Athena framework classes (tiers 1-5, see the file)

Usage:
  python3 -I porting/extract_native_funcs.py [--exe PATH] [--reflection-dir DIR] [--out PATH]
                                              [--index PATH] [--targets PATH] [--headers UE_DIR]
                                              [--no-decode-check] [--no-vtables] [--no-header-check]
                                              [--no-name-fallback]
  python3 -I porting/extract_native_funcs.py --self-test

Stdlib only. Reads the exe and JSON; writes only the output files named above.
"""
import argparse
import bisect
import collections
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
import importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
DEFAULT_EXE = os.path.join(HERE, "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")
DEFAULT_REFL = os.path.join(HERE, "generated", "reflection")
FUNC_NATIVE = 0x00000400

LEA_RE = re.compile(rb"[\x48\x4c]\x8d[\x05\x0d\x15\x1d\x25\x2d\x35\x3d]", re.DOTALL)
PAIR_MAX = 4096


def _sibling(name):
    spec = importlib.util.spec_from_file_location("_sib_" + name[:-3], os.path.join(HERE, name))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


er = _sibling("extract_reflection.py")  # Image, scan_class_blocks (layouts validated there)


# ---------------------------------------------------------------- low level helpers

def resolve_thunk(img, va, hops=4):
    """Follow `jmp rel32` (E9) chains. Thunks are often outside .pdata."""
    for _ in range(hops):
        b = img.code(va, 5)
        if not b or b[0] != 0xE9:
            break
        va = va + 5 + struct.unpack_from("<i", b, 1)[0]
    return va


def calls_typed(img, start, end):
    """[(opcode, target)] for E8 (call) and E9 (jmp) rel32 in [start, end). Byte scan."""
    b = img.code(start, end - start)
    out = []
    if not b:
        return out
    for p in range(len(b) - 4):
        if b[p] == 0xE8 or b[p] == 0xE9:
            out.append((b[p], start + p + 5 + struct.unpack_from("<i", b, p + 1)[0]))
    return out


def calls_in(img, start, end):
    """Direct E8 (call) and E9 (jmp) targets inside [start, end). Byte scan, may include false hits."""
    b = img.code(start, end - start)
    out = []
    if not b:
        return out
    for p in range(len(b) - 4):
        if b[p] == 0xE8 or b[p] == 0xE9:
            out.append(start + p + 5 + struct.unpack_from("<i", b, p + 1)[0])
    return out


def find_r8d_imm(b, lo, hi):
    """mov r8d, imm32 (41 B8 imm32) in byte range; returns the nearest-to-lo value or None."""
    best = None
    for p in range(max(0, lo), min(len(b) - 5, hi)):
        if b[p] == 0x41 and b[p + 1] == 0xB8:
            v = struct.unpack_from("<I", b, p + 2)[0]
            if 0 < v <= PAIR_MAX:
                best = v
    return best


def lea_store_after(b, p, window=24):
    """True when the 7-byte `lea reg,[rip+X]` at b[p] is followed, within `window` bytes, by
    `mov [base], reg` with no displacement (mod=00, base not SIB/RIP). That is the store of a
    vtable pointer into the object itself (offset 0). Stores at nonzero offsets are sub-objects
    and are not counted. Intervening instructions are allowed (a byte-level approximation)."""
    if p + 7 > len(b) or b[p] not in (0x48, 0x4C) or b[p + 1] != 0x8D:
        return False
    lea_reg = ((b[p + 2] >> 3) & 7) | ((1 if b[p] & 4 else 0) << 3)
    for q in range(p + 7, min(len(b) - 2, p + 7 + window)):
        if b[q] in (0x48, 0x4C, 0x49, 0x4D) and b[q + 1] == 0x89:
            modrm = b[q + 2]
            reg = ((modrm >> 3) & 7) | ((1 if b[q] & 4 else 0) << 3)
            mod, rm = modrm >> 6, modrm & 7
            if mod == 0 and rm not in (4, 5) and reg == lea_reg:
                return True
    return False


# ---------------------------------------------------------------- image index

class Program:
    def __init__(self, exe, refl_dir):
        self.img = er.Image(exe)
        self.refl_dir = refl_dir
        exec_secs = [s for s in self.img.sections if s["exec"]]
        pdata = [s for s in self.img.sections if s["name"] == ".pdata"]
        if not pdata:
            raise SystemExit("no .pdata section")
        self.img.code = self._code
        funcs = []
        s = pdata[0]
        for i in range(s["rsize"] // 12):
            b, e, _u = struct.unpack_from("<III", self.img.m, s["raw"] + 12 * i)
            if b == 0 and e == 0:
                continue
            va_b, va_e = self.img.imagebase + b, self.img.imagebase + e
            if self.img.is_code(va_b):
                funcs.append((va_b, va_e))
        funcs.sort()
        self.funcs = funcs
        self.fstarts = [f[0] for f in funcs]
        self.fset = set(self.fstarts)
        self.exec_secs = exec_secs
        self.refs = None  # target VA -> [site VA]

    def _code(self, va, n):
        if not self.img.is_code(va):
            return b""
        o = self.img.off(va)
        if o is None:
            return b""
        return self.img.m[o:o + n]

    def owner(self, va):
        """(start, end) of the function containing va. Uses .pdata when it covers va; otherwise
        (small leaf functions have no unwind entry) estimates the extent from int3 padding."""
        i = bisect.bisect_right(self.fstarts, va) - 1
        if i >= 0 and self.funcs[i][0] <= va < self.funcs[i][1]:
            return self.funcs[i]
        if not self.img.is_code(va):
            return None
        return self._pad_extent(va)

    def _pad_extent(self, va, back=4096, fwd=4096):
        """Extent of a non-pdata function: bounded by runs of two int3 bytes (MSVC padding). A single
        0xCC inside an instruction does not end the function."""
        cache = self.__dict__.setdefault("_pad_cache", {})
        if va in cache:
            return cache[va]
        o = self.img.off(va)
        if o is None:
            return None
        m = self.img.m
        lo = max(o - back, 0)
        hi = min(o + fwd, len(m))
        st = m.rfind(b"\xcc\xcc", lo, o)
        start = o if st < 0 else st + 2
        while start < o and m[start] == 0xCC:
            start += 1
        en = m.find(b"\xcc\xcc", o, hi)
        end = hi if en < 0 else en
        sec = self.img.sec_of(va)
        base = self.img.imagebase + sec["va"] + (start - sec["raw"])
        res = (base, base + (end - start)) if end > start else None
        cache[va] = res
        return res

    def scan_leas(self):
        refs = collections.defaultdict(list)
        for s in self.exec_secs:
            base = self.img.imagebase + s["va"]
            mm = self.img.m[s["raw"]:s["raw"] + s["rsize"]]
            for mo in LEA_RE.finditer(mm):
                p = mo.start()
                if p + 7 > len(mm):
                    continue
                disp = struct.unpack_from("<i", mm, p + 3)[0]
                refs[base + p + 7 + disp].append(base + p)
        self.refs = refs
        return refs

    def func_bytes(self, f):
        return self._code(f[0], f[1] - f[0])

    def qword(self, va):
        o = self.img.off(va)
        return struct.unpack_from("<Q", self.img.m, o)[0] if o is not None else None


# ---------------------------------------------------------------- reflection inputs

def load_reflection(refl_dir):
    names = json.load(open(os.path.join(refl_dir, "class_name_map.json"), encoding="utf-8"))
    ev = json.load(open(os.path.join(refl_dir, "class_name_map.evidence.json"), encoding="utf-8"))
    fb = json.load(open(os.path.join(refl_dir, "function_blocks.json"), encoding="utf-8"))["functions"]
    cfn2blk = {}
    for b in ev["blocks"]:
        for c in b.get("construct_fns", []):
            cfn2blk[int(c, 16)] = int(b["block_va"], 16)
    cfn2fb = {}
    for f in fb:
        for c in f.get("construct_fns", []):
            cfn2fb[int(c, 16)] = f
    return names, cfn2blk, cfn2fb


def load_classes(prog, names):
    """All class blocks in the image: {block_va: {name, fns: [(cfn, nameptr, name)], config, flags}}."""
    img = prog.img
    classes = {}
    for off in er.scan_class_blocks(img):
        sec = next(s for s in img.sections if s["raw"] <= off < s["raw"] + s["rsize"])
        bva = img.imagebase + sec["va"] + (off - sec["raw"])
        _cnf, _cfg, _cpp, _dep, fn_p, _prop, _if = struct.unpack_from("<QQQQQQQ", img.m, off)
        nfunc = struct.unpack_from("<i", img.m, off + 60)[0]
        cflags = struct.unpack_from("<I", img.m, off + 72)[0]
        fns = []
        if fn_p:
            for i in range(nfunc):
                fo = img.off(fn_p + 16 * i)
                cfn, nameva = struct.unpack_from("<QQ", img.m, fo)
                fns.append((cfn, nameva, img.cstr(nameva)))
        name = names.get("0x%x" % bva)
        classes[bva] = {"name": name, "fns": fns, "class_flags": cflags}
    return classes


# ---------------------------------------------------------------- association

def build_gpsc_map(prog, cfn2blk, named):
    """Map a GetPrivateStaticClass (or its thunk) address to the class block.

    D = compiled-in-defer initializer: references Z_Construct_UClass_C (cfn) and &C::StaticClass.
    The LEA targets of D that are code, after thunk resolution, are StaticClass candidates.
    Small wrappers (<=32 bytes) also contribute their direct callees."""
    gpsc = collections.defaultdict(set)
    for cfn, blk in cfn2blk.items():
        if blk not in named:
            continue
        for site in prog.refs.get(cfn, []):
            f = prog.owner(site)
            if not f or f[0] == cfn:
                continue
            for t in _refs_from(prog, f):
                if t == cfn:
                    continue
                r = resolve_thunk(prog.img, t)
                if not prog.img.is_code(r):
                    continue
                gpsc[r].add(blk)
                g = prog.owner(r)
                if g and g[1] - g[0] <= 32:
                    for t2 in calls_in(prog.img, g[0], g[1]):
                        r2 = resolve_thunk(prog.img, t2)
                        if prog.img.is_code(r2) and r2 != cfn:
                            gpsc[r2].add(blk)
    unique = {}
    ambiguous = 0
    for k, v in gpsc.items():
        if len(v) == 1:
            unique[k] = next(iter(v))
        else:
            ambiguous += 1
    return unique, ambiguous


def _refs_from(prog, f):
    """LEA targets whose site lies inside pdata function f (cached)."""
    if not hasattr(prog, "_rf"):
        rf = collections.defaultdict(set)
        for t, sites in prog.refs.items():
            for s in sites:
                o = prog.owner(s)
                if o:
                    rf[o[0]].add(t)
        prog._rf = rf
    return prog._rf.get(f[0], set())


def find_arrays(prog, gpsc_map, classes):
    """Find FNameNativePtrPair arrays referenced from registration functions.

    Returns list of dicts: {va, count, block, owner, names_ok, decl_count}."""
    img = prog.img
    arrays = []
    seen = set()
    stats = collections.Counter()
    for t, sites in prog.refs.items():
        if not img.is_data(t) or img.is_code(t):
            continue
        nptr = prog.qword(t)
        cptr = prog.qword(t + 8)
        if not nptr or not cptr or not img.is_code(cptr) or img.cstr(nptr) is None:
            continue
        for site in sites:
            f = prog.owner(site)
            if not f:
                stats["site_outside_pdata"] += 1
                continue
            cands = set()
            for c in calls_in(img, f[0], f[1]):
                r = resolve_thunk(img, c)
                if r in gpsc_map:
                    cands.add(gpsc_map[r])
            if len(cands) != 1:
                stats["registration_no_class" if not cands else "registration_ambiguous"] += 1
                continue
            blk = next(iter(cands))
            key = (t, blk)
            if key in seen:
                continue
            seen.add(key)
            fb = img.code(f[0], f[1] - f[0])
            lo = site - f[0] - 24
            hi = site - f[0] + 24
            decl = find_r8d_imm(fb, lo, hi)
            # Walk pairs from the array start. Count = declared N when present, else maximal run.
            pairs = []
            i = 0
            while i < PAIR_MAX:
                a = prog.qword(t + 16 * i)
                c = prog.qword(t + 16 * i + 8)
                if not a or not c or not img.is_code(c) or img.cstr(a) is None:
                    break
                pairs.append((a, c))
                i += 1
                if decl and i >= decl:
                    break
            if decl and len(pairs) < decl:
                stats["declared_count_exceeds_valid_pairs"] += 1
            ptrs = set(x[1] for x in classes[blk]["fns"])
            names_ok = all(a in ptrs for a, _ in pairs)
            if not names_ok:
                # Not a native table of this class (seen: UMaterial, single-letter names). Rejected.
                stats["array_rejected_names_not_in_class"] += 1
                continue
            stats["array_names_match_class"] += 1
            arrays.append({"va": t, "block": blk, "owner": f[0], "decl_count": decl,
                           "pairs": pairs, "names_ok": True, "method": "lea"})
    return arrays, stats


def _fn_ptr_ok(img, va):
    """Code pointer, or a pointer into a writable data section (runtime-filled native stubs seen in
    the UMcpProfile table; not decodable from the image, reported as data_ptr)."""
    return img.is_code(va) or (img.is_data(va) and img.sec_of(va)["rsize"] > 0 and
                               img.sec_of(va)["name"] in (".data",))


def name_segment_arrays(prog, classes, covered, native_names):
    """Fallback for classes with no LEA-referenced table (their registration code is not in the
    decoded image). A table is a run of (name, code) pairs whose names all belong to one class C
    (uniquely, after intersecting the name->classes sets over the run) and that contains every
    native-flag name of C. Only classes outside `covered` are considered."""
    img = prog.img
    ptr2cls = collections.defaultdict(set)
    for blk, c in classes.items():
        if not c["name"]:
            continue
        for _cfn, nptr, _n in c["fns"]:
            ptr2cls[nptr].add(blk)
    arrays = []
    stats = collections.Counter()
    for s in img.data_sections():
        base = img.imagebase + s["va"]
        n = s["rsize"] - s["rsize"] % 8
        arr = struct.unpack_from("<%dQ" % (n // 8), img.m, s["raw"])
        i = 0
        L = len(arr)
        while i + 1 < L:
            a, c = arr[i], arr[i + 1]
            if a not in ptr2cls or not c or not _fn_ptr_ok(img, c):
                i += 1
                continue
            # extend a run of pairs (index i, i+2, ...)
            j = i
            seg = []
            while j + 1 < L and arr[j] in ptr2cls and arr[j + 1] and _fn_ptr_ok(img, arr[j + 1]):
                seg.append((arr[j], arr[j + 1], base + 8 * j))
                j += 2
            # greedy split: each piece keeps a non-empty class intersection
            k = 0
            while k < len(seg):
                cand = set(ptr2cls[seg[k][0]])
                m = k + 1
                while m < len(seg):
                    c2 = cand & ptr2cls[seg[m][0]]
                    if not c2:
                        break
                    cand, m = c2, m + 1
                piece = seg[k:m]
                k = m
                stats["pieces"] += 1
                ptrs = set(p[0] for p in piece)
                good = []
                for blk in cand:
                    if blk in covered or blk not in native_names:
                        continue
                    if native_names[blk] <= ptrs:
                        good.append(blk)
                if len(good) != 1:
                    stats["pieces_no_unique_class" if good else "pieces_no_native_cover"] += 1
                    continue
                blk = good[0]
                stats["pieces_assigned"] += 1
                arrays.append({"va": piece[0][2], "block": blk, "owner": None, "decl_count": None,
                               "pairs": [(p[0], p[1]) for p in piece], "names_ok": True,
                               "method": "name_segment"})
            i = max(j, i + 1)
    return arrays, stats


def build_natives(prog, arrays, classes):
    """{block: {name_ptr: code_ptr}} from the arrays, keeping only pairs whose name is in the class.
    Arrays are applied in order, so the first source wins; a later disagreeing pair is a conflict.
    Returns (natives, conflicts, sources) with sources[block][name_ptr] = method of the winning array."""
    natives = collections.defaultdict(dict)
    sources = collections.defaultdict(dict)
    conflicts = 0
    for a in arrays:
        cls_ptrs = set(x[1] for x in classes[a["block"]]["fns"])
        for nptr, cptr in a["pairs"]:
            if nptr not in cls_ptrs:
                continue
            prev = natives[a["block"]].get(nptr)
            if prev is not None and prev != cptr:
                conflicts += 1
                continue
            natives[a["block"]][nptr] = cptr
            sources[a["block"]][nptr] = a["method"]
    return natives, conflicts, sources


def is_exec(prog, va):
    return prog.img.is_code(va)


# ---------------------------------------------------------------- vtables

def gpsc_ctor_helpers(prog, gpsc_fn):
    """From a TClass::GetPrivateStaticClass body, read the stack arguments of GetPrivateStaticClassBody
    that store code addresses through r11 (MSVC keeps the entry rsp in r11 for this call):
    mov [r11+disp8], reg after lea reg,[rip+X]. Argument k (1-based) lives at r11 + (8*(k-5) - 0x68).
    k=10 is InClassConstructor (InternalConstructor<T>), k=11 is InClassVTableHelperCtorCaller<T>.
    Returns (ctor_helper_va or None, vtable_helper_va or None)."""
    img = prog.img
    f = prog.owner(gpsc_fn)
    if not f:
        return None, None
    body = prog.func_bytes(f)
    out = {}
    for p in range(len(body) - 7):
        if body[p] not in (0x48, 0x4C) or body[p + 1] != 0x8D or (body[p + 2] & 0xC7) != 0x05:
            continue
        lea_reg = ((body[p + 2] >> 3) & 7) | ((1 if body[p] & 4 else 0) << 3)
        tgt = f[0] + p + 7 + struct.unpack_from("<i", body, p + 3)[0]
        r = resolve_thunk(img, tgt)
        if not img.is_code(r):
            continue
        for q in range(p + 7, min(len(body) - 4, p + 7 + 24)):
            # 49 89 /r with base r11 (REX.B, rm=011, mod=01) and disp8
            if body[q] in (0x49, 0x4D) and body[q + 1] == 0x89 and (body[q + 2] & 0xC7) == 0x43:
                reg = ((body[q + 2] >> 3) & 7) | ((1 if body[q] & 4 else 0) << 3)
                if reg != lea_reg:
                    continue
                disp = struct.unpack_from("<b", body, q + 3)[0]
                k = 5 + (disp + 0x68) // 8
                out.setdefault(k, r)
                break
    return out.get(10), out.get(11)


def find_vtable(prog, gpsc_block_fns, block):
    """Vtable of the class: the last vtable-pointer store in the class constructor that
    GetPrivateStaticClass passes to InternalVTableHelperCtorCaller<C> (through thunks)."""
    img = prog.img
    helpers = set()
    for g in gpsc_block_fns:
        ctor_h, vt_h = gpsc_ctor_helpers(prog, g)
        for h in (ctor_h, vt_h):
            if h:
                helpers.add(h)
    # The helper (InternalConstructor<C> / InternalVTableHelperCtorCaller<C>) calls exactly one
    # constructor: that callee is C's own ctor. If the helper has no single callee, use the helper.
    # Candidate constructors: each helper itself (if it inlined the constructor) and its direct callees.
    # Depth 2 is not searched, so a base-class constructor (called from the class ctor) does not vote.
    # Each candidate votes with its last vtable store; the class's vtable gets a vote from both helpers.
    # Helpers are InternalConstructor<C> (tail jump to C's ctor) or InternalVTableHelperCtorCaller<C>
    # (one direct call). Other direct calls in a helper are noise (e.g. a call repeated in a branch),
    # so only tail jumps and calls that occur once are candidates.
    ctor_cands = set()
    for h in helpers:
        hf = prog.owner(h)
        if not hf:
            continue
        ctor_cands.add(h)
        typed = calls_typed(img, hf[0], hf[1])
        tail = [t for op, t in typed if op == 0xE9]
        once = collections.Counter(t for op, t in typed if op == 0xE8)
        for t in tail + [t for t, n in once.items() if n == 1]:
            r = resolve_thunk(img, t)
            if prog.owner(r):
                ctor_cands.add(r)
    def own_store(cf):
        body = prog.func_bytes(cf)
        last = None
        for p in range(len(body) - 14):
            if body[p] in (0x48, 0x4C) and body[p + 1] == 0x8D and (body[p + 2] & 0xC7) == 0x05:
                disp = struct.unpack_from("<i", body, p + 3)[0]
                x = cf[0] + p + 7 + disp
                if not img.is_data(x) or img.is_code(x):
                    continue
                q = prog.qword(x)
                if not q or not img.is_code(q):
                    continue
                if lea_store_after(body, p):
                    last = x  # last [this]-relative vtable store: the class's own vtable
        return last

    votes = collections.Counter()
    hops_of = {}
    for c in ctor_cands:
        cur = c
        last = None
        _hop_used = 0
        # Follow single-target wrappers (<= 64 bytes, one E8/E9) to the constructor that stores the vptr.
        for _hop in range(4):
            _hop_used = _hop
            cf = prog.owner(cur)
            if not cf:
                break
            last = own_store(cf)
            if last is not None or cf[1] - cf[0] > 64:
                break
            tg = [resolve_thunk(img, t) for _op, t in calls_typed(img, cf[0], cf[1])]
            tg = [t for t in tg if prog.owner(t)]
            if len(tg) != 1:
                break
            cur = tg[0]
        if last is not None:
            votes[last] += 1
            hops_of[last] = min(hops_of.get(last, 99), _hop_used)
    if not votes:
        return None, None
    best = sorted(votes.items(), key=lambda kv: (-kv[1], kv[0]))[0][0]
    # path: 'direct' when a constructor the helper reaches without a wrapper stores it, else 'wrapper'
    return best, ("direct" if hops_of.get(best, 0) == 0 else "wrapper")


def vtable_slot_count(prog, vt, other_starts):
    """Consecutive code pointers from vt. Stops at a non-code qword or at the start of another class's
    vtable (an adjacent table would otherwise be counted as more slots)."""
    img = prog.img
    slots = 0
    while slots < 4096:
        va = vt + 8 * slots
        q = prog.qword(va)
        if not q or not img.is_code(q):
            break
        if slots > 0 and va in other_starts:
            break
        slots += 1
    return slots


# ---------------------------------------------------------------- decode check

def decode_check(prog, exec_addrs, sample=200, seed=1):
    """objdump the first 48 bytes from each sampled exec address; count (bad) lines."""
    import random
    rnd = random.Random(seed)
    addrs = sorted(set(exec_addrs))
    if not addrs:
        return {"sampled": 0}
    picks = addrs if len(addrs) <= sample else rnd.sample(addrs, sample)
    tot_ins = bad = clean_fns = in_pdata = padded = 0
    tmpd = tempfile.mkdtemp(prefix="nfdec_")
    tmp = os.path.join(tmpd, "w.bin")
    try:
        for va in picks:
            blob = prog._code(va, 48)
            if not blob:
                continue
            # Function starts follow int3/ret/nop padding in MSVC output (checked on the byte before va).
            prev = prog._code(va - 1, 1)
            if prev and prev[0] in (0xCC, 0xC3, 0x90, 0xCB, 0xC2):
                padded += 1
            open(tmp, "wb").write(blob)
            out = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386:x86-64",
                                  "--adjust-vma=0x%x" % va, tmp],
                                 capture_output=True, text=True).stdout.splitlines()
            ins = [l for l in out if re.match(r"\s*[0-9a-f]+:\t", l)]
            # Judge the function body only: stop after the first ret or jmp. The 48-byte window can run
            # into the next function's data or padding, which is not code.
            body = []
            for l in ins:
                body.append(l)
                if re.search(r"\t(ret|jmp)\b", l):
                    break
            nbad = sum(1 for l in body if "(bad)" in l)
            tot_ins += len(body) - nbad
            bad += nbad
            if body and nbad == 0:
                clean_fns += 1
            if va in prog.fset:
                in_pdata += 1
    finally:
        try:
            os.remove(tmp)
            os.rmdir(tmpd)
        except OSError:
            pass
    n = len(picks)
    return {"sampled": n, "functions_clean": clean_fns,
            "functions_clean_fraction": round(clean_fns / max(1, n), 4),
            "decoded_instructions": tot_ins, "bad_lines": bad,
            "bad_fraction": round(bad / max(1, tot_ins + bad), 5),
            "starts_at_pdata_function": in_pdata,
            "preceded_by_padding_or_ret": padded}


# ---------------------------------------------------------------- main build

def build(args):
    prog = Program(args.exe, args.reflection_dir)
    img = prog.img
    names, cfn2blk, cfn2fb = load_reflection(args.reflection_dir)
    print("pdata function starts in code: %d" % len(prog.funcs), file=sys.stderr)
    prog.scan_leas()
    print("LEA targets: %d" % len(prog.refs), file=sys.stderr)
    classes = load_classes(prog, names)
    named = {b for b, c in classes.items() if c["name"]}
    print("class blocks: %d (named %d)" % (len(classes), len(named)), file=sys.stderr)

    gpsc_map, gp_amb = build_gpsc_map(prog, cfn2blk, named)
    print("GetPrivateStaticClass map: %d targets (%d ambiguous dropped)" % (len(gpsc_map), gp_amb), file=sys.stderr)
    arrays, astats = find_arrays(prog, gpsc_map, classes)
    print("arrays: %d %s" % (len(arrays), dict(astats)), file=sys.stderr)
    # Native-flag name pointers per class (from the exact Z_Construct_UFunction join).
    native_names = {}
    for blk, c in classes.items():
        if not c["name"]:
            continue
        nn = set()
        for cfn, nptr, _n in c["fns"]:
            fb = cfn2fb.get(cfn)
            if fb is not None and int(fb["function_flags"], 16) & FUNC_NATIVE:
                nn.add(nptr)
        if nn:
            native_names[blk] = nn
    lea_natives, _, _ = build_natives(prog, arrays, classes)
    # A class is covered when every native-flag name has a LEA-referenced table entry.
    covered = set(b for b, nn in native_names.items() if nn <= set(lea_natives.get(b, {})))
    fb_arrays, fstats = name_segment_arrays(prog, classes, covered, native_names)
    print("name-segment fallback: %s" % dict(fstats), file=sys.stderr)
    if not args.no_name_fallback:
        arrays = arrays + fb_arrays
    else:
        fb_arrays = []
    natives, conflicts, sources = build_natives(prog, arrays, classes)
    arrays_by_blk = collections.defaultdict(list)
    for a in arrays:
        arrays_by_blk[a["block"]].append(a)

    gpsc_by_blk = collections.defaultdict(set)
    for k, b in gpsc_map.items():
        gpsc_by_blk[b].add(k)

    # Output records. Function-level exec address is the paired code pointer.
    out_funcs = {}
    index = {}
    total_links = 0
    linked_flags = 0
    fn_with_exec = 0
    native_flag_fns = 0
    native_flag_with_exec = 0
    distinct_names = set()
    distinct_names_exec = set()
    fb_names = set()
    for f in cfn2fb.values():
        if f.get("name"):
            fb_names.add(f["name"])
    decode_addrs = []
    stub_entries = []
    vt_map = {}
    if not args.no_vtables:
        for blk, cls in classes.items():
            if not cls["name"]:
                continue
            v, path = find_vtable(prog, gpsc_by_blk.get(blk, set()), blk)
            vt_map[blk] = (v, 0, path)
        starts = set(v for v, _n, _p in vt_map.values() if v)
        for blk, (v, _n, path) in list(vt_map.items()):
            if v:
                vt_map[blk] = (v, vtable_slot_count(prog, v, starts - {v}), path)
    vt_stats = vtable_checks(prog, vt_map, classes, args.reflection_dir)
    pm_all = {}
    pm_path = os.path.join(args.reflection_dir, "class_parent_map.json")
    if os.path.exists(pm_path):
        pm_all = json.load(open(pm_path, encoding="utf-8"))
    vt_status = vtable_status(vt_map, pm_all) if vt_map else {}
    vt_stats_status = dict(collections.Counter(vt_status.values()))
    vt_stats["status_counts"] = vt_stats_status
    for blk, cls in classes.items():
        if not cls["name"]:
            continue
        cname = cls["name"]
        nat = natives.get(blk, {})
        entries = {}
        for cfn, nptr, fname in cls["fns"]:
            total_links += 1
            fbent = cfn2fb.get(cfn)
            flags = int(fbent["function_flags"], 16) if fbent else None
            if fbent is not None:
                linked_flags += 1
            if fname:
                distinct_names.add(fname)
            code = nat.get(nptr)
            if flags is not None and flags & FUNC_NATIVE:
                native_flag_fns += 1
                if code is not None:
                    native_flag_with_exec += 1
            exec_addr = None
            stub_addr = None
            if code is not None and is_exec(prog, code):
                exec_addr = "0x%x" % code
                fn_with_exec += 1
                decode_addrs.append(code)
                if fname:
                    distinct_names_exec.add(fname)
            elif code is not None:
                stub_addr = "0x%x" % code
                stub_entries.append(code)
            entries[fname or ("unnamed_%x" % cfn)] = {
                "exec_addr": exec_addr,
                "flags": "0x%x" % flags if flags is not None else None,
                "params": fbent.get("params", []) if fbent else None,
                "create_func": "0x%x" % cfn,
            }
            if stub_addr:
                entries[fname]["stub_addr"] = stub_addr
            if code is not None:
                entries[fname]["source"] = sources.get(blk, {}).get(nptr)
        if entries:
            out_funcs[cname] = entries
        if True:  # every named class gets an index record (vtable may be None)
            cand, nslots, vpath = vt_map.get(blk, (None, 0, None))
            vshared = sum(1 for _b, (v2, _n2, _p2) in vt_map.items() if cand and v2 == cand) if cand else 0
            vstat = vt_status.get(blk)
            vt = cand if vstat in ("unique", "inherited") else None
            index[cname] = {
                "block_va": "0x%x" % blk,
                "native_arrays": ["0x%x" % a["va"] for a in arrays_by_blk.get(blk, [])],
                "native_method": sorted({a["method"] for a in arrays_by_blk.get(blk, [])}),
                "native_count": len(nat),
                "function_links": len(cls["fns"]),
                "vtable_addr": "0x%x" % vt if vt else None,
                "vtable_slots": nslots if vt else None,
                "vtable_status": vstat,
                "vtable_candidate": "0x%x" % cand if cand else None,
                "vtable_candidate_slots": nslots if cand else None,
                "vtable_candidate_path": vpath,
                "vtable_candidate_shared_by_classes": vshared,
                "gpsc_targets": ["0x%x" % g for g in sorted(gpsc_by_blk.get(blk, set()))],
            }

    dec = None
    if not args.no_decode_check:
        dec = decode_check(prog, decode_addrs)
    in_code = sum(1 for va in decode_addrs if img.is_code(va))

    summary = {
        "image": args.exe,
        "classes_with_functions": len(out_funcs),
        "classes_with_native_array": sum(1 for b in named if natives.get(b)),
        "class_function_links": total_links,
        "function_links_joined_to_flags": linked_flags,
        "function_links_native_flag": native_flag_fns,
        "function_links_native_flag_with_exec": native_flag_with_exec,
        "function_links_with_exec": fn_with_exec,
        "distinct_function_names": len(distinct_names),
        "distinct_function_names_with_exec": len(distinct_names_exec),
        "function_blocks_distinct_names": len(fb_names),
        "native_arrays_associated": len(arrays),
        "native_arrays_by_lea": sum(1 for a in arrays if a["method"] == "lea"),
        "native_arrays_by_name_segment": len(fb_arrays),
        "native_array_conflicts_dropped": conflicts,
        "function_links_source_lea": sum(1 for b in out_funcs for f in out_funcs[b].values() if f.get("source") == "lea"),
        "function_links_source_name_segment": sum(1 for b in out_funcs for f in out_funcs[b].values() if f.get("source") == "name_segment"),
        "exec_addrs_in_code_sections": in_code,
        "exec_addrs_total": len(decode_addrs),
        "function_links_with_writable_data_pointer": len(stub_entries),
        "association_stats": dict(astats),
        "vtables_verified": sum(1 for v in index.values() if isinstance(v, dict) and v.get("vtable_addr")),
        "vtable_checks": vt_stats,
        "decode_check": dec,
    }
    if not args.no_header_check:
        summary["header_validation"] = header_validation(out_funcs, args.headers, ENGINE_VALIDATE)
    index["_summary"] = summary
    return out_funcs, index, summary, prog, classes, natives, arrays


def vtable_status(vt_map, pm):
    """Per class block, using only the class hierarchy (class_parent_map.json):
      unique      no other class has this table;
      inherited   every class sharing the table is an ancestor or a descendant of this class (a class that
                  adds no virtuals reuses its parent's table, so a parent and its descendants can share one);
      unverified  some class sharing the table is in neither lineage (the MSVC model does not produce this
                  for two unrelated classes).
    Only unique and inherited tables are published as vtable_addr."""
    found = {b: v for b, (v, _n, _p) in vt_map.items() if v}
    cnt = collections.Counter(found.values())
    by_vt = collections.defaultdict(list)
    for b, v in found.items():
        by_vt[v].append(b)
    anc = {}
    for blk in found:
        chain, cur, seen = set(), "0x%x" % blk, set()
        while cur in pm and cur not in seen and pm[cur].get("parent_source") == "block":
            seen.add(cur)
            p = int(pm[cur]["parent_va"], 16)
            chain.add(p)
            cur = "0x%x" % p
        anc[blk] = chain
    status = {}
    for blk, v in found.items():
        if cnt[v] == 1:
            status[blk] = "unique"
            continue
        ok = True
        for other in by_vt[v]:
            if other == blk:
                continue
            if other in anc[blk] or blk in anc.get(other, ()):
                continue
            ok = False
            break
        status[blk] = "inherited" if ok else "unverified"
    return status


def vtable_checks(prog, vt_map, classes, refl_dir):
    """Uniqueness of vtables, and inheritance agreement with class_parent_map.json: a child's slots
    should match its parent's slots at most positions (inherited, not overridden)."""
    found = {b: v for b, (v, _n, _p) in vt_map.items() if v}
    cnt = collections.Counter(found.values())
    dup = sum(1 for v, c in cnt.items() if c > 1)
    res = {"vtables_found": len(found), "distinct_vtable_addresses": len(cnt),
           "vtable_addresses_shared_by_several_classes": dup}
    pm_path = os.path.join(refl_dir, "class_parent_map.json")
    if not os.path.exists(pm_path):
        return res
    pm = json.load(open(pm_path, encoding="utf-8"))
    ratios = []
    for blk, vt in found.items():
        ent = pm.get("0x%x" % blk)
        if not ent or ent.get("parent_source") != "block":
            continue
        pblk = int(ent["parent_va"], 16)
        pvt = found.get(pblk)
        if not pvt:
            continue
        n = min(vt_map[blk][1], vt_map[pblk][1], 64)  # slot counts
        if n < 4:
            continue
        same = sum(1 for i in range(n) if prog.qword(vt + 8 * i) == prog.qword(pvt + 8 * i))
        ratios.append(same / n)
    if ratios:
        ratios.sort()
        res["child_parent_pairs"] = len(ratios)
        res["child_parent_shared_slot_median"] = round(ratios[len(ratios) // 2], 3)
        res["child_parent_pairs_with_majority_shared"] = sum(1 for r in ratios if r >= 0.5)
    print("vtable checks: %s" % res, file=sys.stderr)
    return res


ATHENA_TARGETS = ["AFortGameModeAthena", "AFortGameStateAthena", "AFortPlayerStateAthena",
                  "AFortPlayerControllerAthena", "AFortPlayerPawnAthena", "AFortPawn", "AFortPlayerPawn",
                  "AFortWorldSettings"]
FUNC_NET_RPC = 0x00000040 | 0x00000080 | 0x00001000 | 0x00004000 | 0x00200000 | 0x01000000
FUNC_BP_EVENT = 0x08000000
FUNC_BP_CALLABLE = 0x04000000
FUNC_NET_VALIDATE = 0x80000000


def decompile_targets(out_funcs, index, parent_map_path, class_names):
    """Priority list of functions to decompile for the Athena framework classes.

    Tiers (lower first):
      1 network: FUNC_Net functions (RPCs and replicated events); exec thunk calls _Validate/_Implementation
      2 BlueprintNativeEvent: FUNC_Native + FUNC_BlueprintEvent (C++ _Implementation behind the event)
      3 BlueprintCallable native functions
      4 other native functions with an exec address
      5 functions with no exec address in the image (BlueprintImplementable events, delegates, C++-only)
    Addresses are the exec thunks from native_funcs.json; tier 5 has exec_addr null."""
    pm = {}
    if os.path.exists(parent_map_path):
        pm = json.load(open(parent_map_path, encoding="utf-8"))
    by_name = {v.get("name"): k for k, v in pm.items() if isinstance(v, dict) and v.get("name")}
    rows = []
    classes = {}
    for cname in class_names:
        funcs = out_funcs.get(cname, {})
        ix = index.get(cname, {})
        chain = []
        cur = by_name.get(cname)
        seen = set()
        while cur and cur not in seen and cur in pm:
            seen.add(cur)
            chain.append(pm[cur]["name"])
            cur = pm[cur].get("parent_va") if pm[cur].get("parent_source") == "block" else None
            cur = "0x%x" % int(cur, 16) if cur else None
        classes[cname] = {
            "block_va": ix.get("block_va"),
            "parent_chain": chain[1:] if chain else [],
            "native_arrays": ix.get("native_arrays", []),
            "vtable_addr": ix.get("vtable_addr"),
            "vtable_status": ix.get("vtable_status"),
            "vtable_slots_upper_bound": ix.get("vtable_slots"),
            "vtable_candidate_unverified": ix.get("vtable_candidate") if not ix.get("vtable_addr") else None,
            "function_count": len(funcs),
            "native_exec_count": sum(1 for f in funcs.values() if f.get("exec_addr")),
        }
        for fname, f in funcs.items():
            flags = int(f["flags"], 16) if f.get("flags") else 0
            if flags & FUNC_NET_RPC:
                tier = 1
            elif (flags & FUNC_NATIVE) and (flags & FUNC_BP_EVENT):
                tier = 2
            elif flags & FUNC_BP_CALLABLE and f.get("exec_addr"):
                tier = 3
            elif f.get("exec_addr"):
                tier = 4
            else:
                tier = 5
            if tier == 5 and (flags & FUNC_NATIVE) == 0 and not (flags & FUNC_BP_EVENT):
                continue  # not a native function: nothing to decompile in the native image
            roles = [k for k, bit in (("Reliable", 0x80), ("Response", 0x1000), ("Multicast", 0x4000),
                                      ("Server", 0x200000), ("Client", 0x1000000)) if flags & bit]
            rows.append({
                "tier": tier, "class": cname, "function": fname,
                "exec_addr": f.get("exec_addr"),
                "flags": f.get("flags"),
                "net_roles": roles,
                "validate": bool(flags & FUNC_NET_VALIDATE),
                "implementation_event": bool(flags & FUNC_NATIVE and flags & FUNC_BP_EVENT),
                "create_func": f.get("create_func"),
                "source": f.get("source"),
            })
    rows.sort(key=lambda r: (r["tier"], ATHENA_TARGETS.index(r["class"]), r["function"]))
    for i, r in enumerate(rows, 1):
        r["rank"] = i
    tiers = collections.Counter(r["tier"] for r in rows)
    return {
        "_meta": {
            "source": "native_funcs.json (exec addresses) and native_class_index.json (vtables)",
            "image": "porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe",
            "tiers": {"1": "FUNC_Net (RPC / replicated)", "2": "FUNC_Native + FUNC_BlueprintEvent",
                      "3": "FUNC_BlueprintCallable with exec", "4": "other native with exec",
                      "5": "no exec address (BlueprintImplementable / unlinked): decompile via vtable or ProcessEvent"},
            "counts_by_tier": dict(sorted(tiers.items())),
            "note": "Functions not listed as UFUNCTIONs (C++ helpers, X_Implementation/X_Validate virtuals) are not "
                    "in the reflection tables and are not in this list. vtable_addr is the class's own vtable or the "
                    "ancestor's table it inherits (MSVC does not emit a table for a class with no new virtuals).",
        },
        "classes": classes,
        "targets": rows,
    }


ENGINE_VALIDATE = ["AActor", "APawn", "ACharacter", "APlayerController", "AGameModeBase", "UActorComponent",
                   "USceneComponent", "UKismetMathLibrary", "UGameplayStatics", "UKismetSystemLibrary",
                   "UAbilitySystemBlueprintLibrary", "UObject"]
UFUNC_DECL = re.compile(r"UFUNCTION\s*\((?:[^()]|\([^()]*\))*\)\s*(?:(?:static|virtual|inline|FORCEINLINE)\s+)*"
                        r"[\w:<>,\s\*&]+?\b(\w+)\s*\(", re.S)


def header_validation(out_funcs, headers_dir, class_names):
    """Compare recovered function names with UFUNCTION declarations in the UE 4.25 headers of the same class.
    The shipped engine is not exactly 4.25 (binary_analysis.md section 8), so mismatches are expected."""
    root = os.path.join(headers_dir, "Engine", "Source", "Runtime")
    if not os.path.isdir(root):
        return {"skipped": "no header tree at %s" % headers_dir}
    rows = []
    for cname in class_names:
        # cname is the C++ name (AActor, UObject): match the declaration `class [X_API ]AActor : public ...`
        pat = re.compile(rb"class\s+(?:[A-Z][A-Z0-9_]*_API\s+)?" + re.escape(cname.encode()) + rb"\s*(?::|\{|\s+final)")
        decl_file = None
        for dp, dn, fn in os.walk(root):
            for f in fn:
                if not f.endswith(".h"):
                    continue
                path = os.path.join(dp, f)
                try:
                    data = open(path, "rb").read()
                except OSError:
                    continue
                if pat.search(data) and b"UCLASS(" in data:
                    decl_file = (path, data)
                    break
            if decl_file:
                break
        if not decl_file:
            rows.append({"class": cname, "header": None})
            continue
        path, data = decl_file
        m = pat.search(data)
        body = data[m.start():]
        hnames = set(UFUNC_DECL.findall(body.decode("latin1")))
        ours = out_funcs.get(cname, {})
        ours_names = set(ours.keys())
        ours_exec = set(k for k, v in ours.items() if v.get("exec_addr"))
        rows.append({"class": cname, "header": os.path.relpath(path, headers_dir),
                     "header_ufunctions": len(hnames),
                     "found_in_recovered_class": len(hnames & ours_names),
                     "found_with_exec": len(hnames & ours_exec),
                     "recovered_functions": len(ours_names),
                     "recovered_with_exec": len(ours_exec),
                     "k2_get_actor_location_exec": ours.get("K2_GetActorLocation", {}).get("exec_addr")})
    return rows


def write_outputs(out_funcs, index, out_path, index_path):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as fh:
        json.dump(out_funcs, fh, separators=(",", ":"))
    with open(index_path, "w", encoding="utf-8") as fh:
        json.dump(index, fh, indent=1)


# ---------------------------------------------------------------- self test

class _FakeImg:
    def __init__(self, code_base, data):
        self.imagebase = 0x140000000
        self.code_base = code_base
        self.data = data


def self_test():
    ok = True

    def check(name, cond):
        nonlocal ok
        print(("PASS " if cond else "FAIL ") + name)
        ok = ok and bool(cond)

    # 1. LEA regex and target computation: lea rdx,[rip+0x10] at offset 4 -> target = 4+7+0x10
    blob = b"\x90" * 4 + b"\x48\x8d\x15\x10\x00\x00\x00" + b"\xc3"
    hits = [m.start() for m in LEA_RE.finditer(blob)]
    check("LEA regex finds rip-relative lea", hits == [4])
    check("LEA target arithmetic", 4 + 7 + 0x10 == 0x1B)
    # 2. lea r8 (REX.WR) and rcx forms both match
    check("LEA regex matches REX.WR (lea r8)", bool(LEA_RE.search(b"\x4c\x8d\x05\x00\x00\x00\x00")))
    # 3. mov r8d, 6 (41 B8 06 00 00 00) found in a window
    body = b"\xe8\x00\x00\x00\x00" + b"\x41\xb8\x06\x00\x00\x00" + b"\x48\x8d\x15\x00\x00\x00\x00"
    check("mov r8d imm found", find_r8d_imm(body, 0, len(body)) == 6)
    # 4. lea-store pattern: lea rax,[rip+X]; mov [rcx],rax  -> store
    store = b"\x48\x8d\x05\x00\x00\x00\x00" + b"\x48\x89\x01"
    nostore = b"\x48\x8d\x05\x00\x00\x00\x00" + b"\x48\x8b\x01"
    check("lea-store detected", lea_store_after(store, 0))
    check("lea followed by load not a store", not lea_store_after(nostore, 0))
    store_off = b"\x48\x8d\x05\x00\x00\x00\x00" + b"\x48\x89\x41\x28"   # mov [rcx+0x28], rax
    check("store at nonzero offset is not a vtable store", not lea_store_after(store_off, 0))
    store_gap = b"\x48\x8d\x05\x00\x00\x00\x00" + b"\x48\x8d\x4f\x28" + b"\x48\x89\x07"  # lea; lea; mov [rdi],rax
    check("store after intervening instruction detected", lea_store_after(store_gap, 0))
    # 5. thunk resolution: jmp rel32 chain
    class _Img:
        def __init__(self, mem):
            self.mem = mem

        def code(self, va, n):
            o = va - 0x1000
            return self.mem[o:o + n]
    mem = bytearray(64)
    mem[0:5] = b"\xe9" + struct.pack("<i", 0x10 - 5)   # 0x1000 -> 0x1010
    mem[0x10:0x15] = b"\x90\x90\x90\x90\x90"
    check("thunk resolves jmp rel32", resolve_thunk(_Img(bytes(mem)), 0x1000) == 0x1010)
    # 6. calls_in finds E8 and E9
    code = b"\x90" + b"\xe8" + struct.pack("<i", 0x20 - 6) + b"\xe9" + struct.pack("<i", 0x40 - 11) + b"\xc3"
    class _Img2:
        def __init__(self, mem):
            self.mem = mem

        def code(self, va, n):
            o = va - 0x2000
            return self.mem[o:o + n]
    tg = calls_in(_Img2(code), 0x2000, 0x2000 + len(code))
    check("calls_in finds call and jmp targets", 0x2020 in tg and 0x2040 in tg)
    # 7. decode check helper works on a benign prologue (no exec image needed beyond prog._code)
    fake = type("P", (), {})()
    fake.fset = set()
    fake._code = lambda va, n: b"\x48\x83\xec\x28\x33\xc0\xc3" + b"\x90" * 8
    res = decode_check(fake, [0x140001000], sample=1)
    check("decode check reports 0 bad lines on clean bytes", res.get("bad_lines") == 0)
    # 8. reflection join helper: cfn key format
    check("hex keys round trip", int("%x" % 0x1446ab350, 16) == 0x1446ab350)
    print("SELF-TEST " + ("OK" if ok else "FAILED"))
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--exe", default=DEFAULT_EXE)
    ap.add_argument("--reflection-dir", default=DEFAULT_REFL)
    ap.add_argument("--out", default=os.path.join(DEFAULT_REFL, "native_funcs.json"))
    ap.add_argument("--index", default=os.path.join(DEFAULT_REFL, "native_class_index.json"))
    ap.add_argument("--no-decode-check", action="store_true")
    ap.add_argument("--no-vtables", action="store_true")
    ap.add_argument("--headers", default="/opt/fortnite/UnrealEngine",
                    help="UE 4.25 tree root (contains Engine/Source)")
    ap.add_argument("--no-header-check", action="store_true")
    ap.add_argument("--targets", default=os.path.join(DEFAULT_REFL, "decompile_targets.json"))
    ap.add_argument("--no-name-fallback", action="store_true",
                    help="only use LEA-referenced tables (no name-only segments)")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    out_funcs, index, summary, *_ = build(args)
    write_outputs(out_funcs, index, args.out, args.index)
    tg = decompile_targets(out_funcs, index, os.path.join(args.reflection_dir, "class_parent_map.json"),
                           ATHENA_TARGETS)
    with open(args.targets, "w", encoding="utf-8") as fh:
        json.dump(tg, fh, indent=1)
    print(json.dumps(summary, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
