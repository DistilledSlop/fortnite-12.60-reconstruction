#!/usr/bin/env python3
"""Resolve the parent (super class) of every FClassParams block recovered by extract_reflection.py.

Why this is needed: the block's dependency_fns are DependentSingletons. UHT (UE 4.25,
Programs/UnrealHeaderTool/Private/CodeGenerator.cpp, around line 2200) emits, per class:
  - the super class's Z_Construct_UClass_* function first, if the class has a super class;
  - then the module's Z_Construct_UPackage__Script_<Module> function, always last.
A class with no super class (UObject) has only the package entry. So the parent is
dependency_fns[0] whenever a block has two entries. extract_reflection.py calls
dependency_fns[1] "base_fn", which is the package, not the parent.

Steps per block:
  1. parent_fn = dependency_fns[0] (two entries) or none (one entry: root).
  2. If parent_fn is the construct function of another block (construct functions come from
     the Ghidra evidence file, which records the function that references each block), that
     block is the parent. Thunks (single jmp) are followed when --image is given.
  3. Otherwise the parent is a class with no FClassParams block of its own. In this build those
     are the CoreUObject intrinsics UClass, UEnum, UScriptStruct, UPackageMap and UInterface.
     They are named from the child headers when --headers is given (all children must agree).
  4. chain_ok is True when the parent chain reaches a root block named UObject through blocks only.

Usage:
  python3 -I porting/resolve_class_parents.py [--reflection JSON] [--evidence JSON] [--name-map JSON]
      [--image PE] [--headers UE_ENGINE_DIR] [--out JSON] [--no-validate]
  python3 -I porting/resolve_class_parents.py --self-test

Defaults: reflection/evidence/name map under porting/generated/reflection/, --image
porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe (skipped if absent; read-only),
--headers /opt/fortnite/UnrealEngine/Engine (Source and Plugins are scanned; read-only),
--out porting/generated/reflection/class_parent_map.json.

Output: {block_va: {name, parent_name, parent_va, chain_ok, parent_source, chain, package}}.
parent_source: block | thunk_block | root | orphan | no_dependencies | unexpected_dependency_count.
The summary (chains for the 8 targets and 10 engine classes, and header accuracy) goes to stdout.
"""
import argparse
import collections
import json
import mmap
import os
import re
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REFL_DIR = os.path.join(REPO, "porting", "generated", "reflection")
DEFAULT_REFLECTION = os.path.join(REFL_DIR, "fortnite_game_reflection.json")
DEFAULT_EVIDENCE = os.path.join(REFL_DIR, "class_name_map.evidence.json")
DEFAULT_NAMES = os.path.join(REFL_DIR, "class_name_map.json")
DEFAULT_IMAGE = os.path.join(REPO, "porting", "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")
DEFAULT_HEADERS = "/opt/fortnite/UnrealEngine/Engine"
DEFAULT_OUT = os.path.join(REFL_DIR, "class_parent_map.json")

TARGETS = ["AFortWorldSettings", "AFortGameStateAthena", "AFortPlayerStateAthena",
           "AFortPlayerControllerAthena", "AFortPlayerPawnAthena", "AFortAthenaMapInfo",
           "AFortPoiManager", "AFortWorldManager"]
ENGINE_CHECK = ["AActor", "APawn", "ACharacter", "APlayerController", "APlayerState",
                "AGameStateBase", "AGameModeBase", "AWorldSettings", "UActorComponent", "USceneComponent"]

# UClass / UINTERFACE declaration with its base list. Allows one level of nested parentheses in the macro.
DECL = re.compile(rb"\b(UCLASS|UINTERFACE)\s*\((?:[^()]|\([^()]*\))*\)\s*(?:class|struct)\s+"
                  rb"(?:[A-Z][A-Z0-9_]*_API\s+)?([A-Za-z_]\w*)\s*(?:final\s*)?(?::([^{;]{0,400}))?\{")
BASE = re.compile(rb"\b(?:public|protected|private)\s+(?:virtual\s+)?([A-Za-z_]\w*)")


def hexva(value):
    return "0x%x" % value


class Image:
    """Read-only PE view (PE32+) used for thunk and package checks. Never written."""

    def __init__(self, path):
        self.f = open(path, "rb")
        self.m = mmap.mmap(self.f.fileno(), 0, access=mmap.ACCESS_READ)
        m = self.m
        e = struct.unpack_from("<I", m, 0x3C)[0]
        nsec = struct.unpack_from("<H", m, e + 6)[0]
        opt_size = struct.unpack_from("<H", m, e + 20)[0]
        opt = e + 24
        if struct.unpack_from("<H", m, opt)[0] != 0x20B:
            raise SystemExit("expected PE32+ image: %s" % path)
        self.base = struct.unpack_from("<Q", m, opt + 24)[0]
        self.sections = []
        for i in range(nsec):
            o = opt + opt_size + 40 * i
            vsize, va, rsize, raw = struct.unpack_from("<IIII", m, o + 8)
            self.sections.append((va, max(vsize, rsize), rsize, raw))

    def off(self, va, n=1):
        rva = va - self.base
        for va0, span, rsize, raw in self.sections:
            if va0 <= rva and rva + n <= va0 + span and rva - va0 + n <= rsize:
                return raw + (rva - va0)
        return None

    def read(self, va, n):
        o = self.off(va, n)
        return bytes(self.m[o:o + n]) if o is not None else None

    def u64(self, va):
        b = self.read(va, 8)
        return struct.unpack("<Q", b)[0] if b else None

    def cstr(self, va, maxlen=200):
        if not va:
            return None
        o = self.off(va, 1)
        if o is None:
            return None
        end = self.m.find(b"\0", o, o + maxlen)
        if end < 0:
            return None
        raw = self.m[o:end]
        if not raw or not all(32 <= c < 127 for c in raw):
            return None
        return raw.decode("ascii")

    def lea_targets(self, fn, n=0x100):
        """RIP-relative LEA (REX.W 8D /r, mod=00 rm=101) targets in the first n bytes of fn."""
        code = self.read(fn, n) or b""
        out = []
        for i in range(len(code) - 7):
            if 0x48 <= code[i] <= 0x4F and code[i + 1] == 0x8D and (code[i + 2] & 0xC7) == 0x05:
                out.append(fn + i + 7 + struct.unpack_from("<i", code, i + 3)[0])
        return out


class Probe:
    """Binary lookups: thunk following and package-name recognition."""

    def __init__(self, image):
        self.image = image
        self._pkg = {}

    def follow_thunk(self, fn, max_hops=4):
        """Follow single-jmp thunks (E9 rel32, or FF 25 disp32 through a pointer). Returns (target, hops)."""
        cur, hops = fn, 0
        while hops < max_hops:
            code = self.image.read(cur, 6) if self.image else None
            if not code:
                break
            if code[0] == 0xE9:
                nxt = cur + 5 + struct.unpack_from("<i", code, 1)[0]
            elif code[0:2] == b"\xff\x25":
                nxt = self.image.u64(cur + 6 + struct.unpack_from("<i", code, 2)[0])
            else:
                break
            if not nxt:
                break
            cur, hops = nxt, hops + 1
        return cur, hops

    def package_name(self, fn):
        """Package name ("/Script/Module") when fn is a Z_Construct_UPackage function (FPackageParams lea)."""
        if fn in self._pkg:
            return self._pkg[fn]
        name = None
        if self.image:
            for target in self.image.lea_targets(fn):
                s = self.image.cstr(self.image.u64(target))
                if s and s.startswith("/Script/"):
                    name = s
                    break
        self._pkg[fn] = name
        return name


class Resolver:
    """Pure parent resolution. blocks: {va: [dependency ints]}, constructs: {fn int: block va}."""

    def __init__(self, blocks, constructs, names, probe=None, orphan_names=None):
        self.blocks = blocks
        self.constructs = constructs
        self.names = names
        self.probe = probe
        self.orphan_names = orphan_names or {}
        self.recs = {}
        for va in blocks:
            self.recs[va] = self._resolve(va)

    def _resolve(self, va):
        deps = self.blocks[va]
        rec = {"parent_fn": None, "parent_va": None, "parent_source": None, "package_fn": None}
        if not deps:
            rec["parent_source"] = "no_dependencies"
            return rec
        if len(deps) == 1:
            rec["parent_source"] = "root"
            rec["package_fn"] = deps[0]
            return rec
        if len(deps) != 2:
            rec["parent_source"] = "unexpected_dependency_count"
            rec["parent_fn"] = deps[0]
            rec["package_fn"] = deps[-1]
            return rec
        super_fn, pkg_fn = deps
        rec["package_fn"] = pkg_fn
        target = super_fn
        if target in self.constructs:
            rec["parent_source"] = "block"
        elif self.probe is not None:
            target, hops = self.probe.follow_thunk(super_fn)
            if hops and target in self.constructs:
                rec["parent_source"] = "thunk_block"
                rec["thunk_hops"] = hops
            else:
                target = super_fn
        if rec["parent_source"] in ("block", "thunk_block"):
            rec["parent_va"] = self.constructs[target]
        else:
            rec["parent_source"] = "orphan"
            rec["parent_fn"] = super_fn
        rec["parent_fn"] = rec["parent_fn"] or target
        return rec

    def parent_name(self, va):
        rec = self.recs[va]
        if rec["parent_va"]:
            return self.names.get(rec["parent_va"])
        if rec["parent_source"] == "orphan":
            return self.orphan_names.get(rec["parent_fn"])
        return None

    def chain(self, va):
        """(names from va up to the end, chain_ok). chain_ok: all links are blocks and the root is UObject."""
        names_out = []
        seen = set()
        cur = va
        while True:
            if cur in seen:
                return names_out + ["<cycle>"], False
            seen.add(cur)
            names_out.append(self.names.get(cur) or "<%s>" % cur)
            rec = self.recs[cur]
            if rec["parent_source"] == "root":
                return names_out, names_out[-1] == "UObject"
            if rec["parent_va"]:
                cur = rec["parent_va"]
                continue
            if rec["parent_source"] == "orphan":
                names_out.append(self.parent_name(cur) or "<orphan %s>" % hexva(rec["parent_fn"]))
            return names_out, False

    def package_of(self, va):
        pkg = self.recs[va]["package_fn"]
        if self.probe is None or pkg is None:
            return None
        return self.probe.package_name(pkg)


def build_records(resolver):
    out = {}
    for va in sorted(resolver.blocks, key=lambda v: int(v, 16)):
        rec = resolver.recs[va]
        chain, ok = resolver.chain(va)
        out[va] = {
            "name": resolver.names.get(va),
            "parent_name": resolver.parent_name(va),
            "parent_va": rec["parent_va"] or (hexva(rec["parent_fn"]) if rec["parent_source"] == "orphan" else None),
            "chain_ok": ok,
            "parent_source": rec["parent_source"],
            "chain": chain,
            "package": resolver.package_of(va),
        }
    return out


# ---------------------------------------------------------------- header index (validation and orphan names)

def index_headers(roots):
    """{class_name: (set of base names, first path)} for UCLASS and UINTERFACE declarations."""
    index = {}
    for root in roots:
        for dirpath, dirnames, filenames in os.walk(root):
            for fn in filenames:
                if not fn.endswith(".h"):
                    continue
                path = os.path.join(dirpath, fn)
                try:
                    with open(path, "rb") as fh:
                        data = fh.read()
                except OSError:
                    continue
                if b"UCLASS" not in data and b"UINTERFACE" not in data:
                    continue
                for m in DECL.finditer(data):
                    name = m.group(2).decode("ascii")
                    bm = BASE.search(m.group(3) or b"")
                    base = bm.group(1).decode("ascii") if bm else None
                    entry = index.setdefault(name, {"bases": set(), "path": os.path.relpath(path, root)})
                    entry["bases"].add(base)
    return index


def header_base(index, name):
    """Single base class from the header, or None when unknown or ambiguous."""
    entry = index.get(name) if name else None
    if not entry or len(entry["bases"]) != 1:
        return None
    return next(iter(entry["bases"]))


def name_orphans(resolver, index):
    """Name each orphan parent after the header base its children agree on (all children must agree)."""
    votes = collections.defaultdict(collections.Counter)
    for va, rec in resolver.recs.items():
        if rec["parent_source"] != "orphan":
            continue
        base = header_base(index, resolver.names.get(va))
        if base:
            votes[rec["parent_fn"]][base] += 1
    names = {}
    report = {}
    for fn, counter in votes.items():
        if len(counter) == 1:
            names[fn] = next(iter(counter))
        report[hexva(fn)] = dict(counter)
    resolver.orphan_names = names
    return report


# ---------------------------------------------------------------- self test

def self_test():
    failures = []
    count = [0]

    def check(label, ok):
        count[0] += 1
        if not ok:
            failures.append(label)

    class FakeImage:
        base = 0
        thunks = {0x30: 0x40}      # 0x30: jmp 0x40 (E9 rel32)
        code = {0x30: b"\xe9" + struct.pack("<i", 0x40 - 0x35)}

        def read(self, va, n):
            return (self.code.get(va, b"\x90" * 6) + b"\x90" * 6)[:n] if va in self.code else b"\x90" * n

        def u64(self, va):
            return None

        def cstr(self, va, maxlen=200):
            return None

        def lea_targets(self, fn, n=0x100):
            return []

    probe = Probe(FakeImage())
    package = {0x10: "/Script/Test"}
    probe.package_name = lambda fn: package.get(fn)
    blocks = {
        "0x1000": [0x10],               # root: UObject-like, package only
        "0x1100": [0x20, 0x10],         # direct parent 0x20 (construct of 0x1000)
        "0x1200": [0x30, 0x10],         # parent reached through thunk 0x30 -> 0x40
        "0x1300": [0x50, 0x10],         # parent 0x50 is not a block (orphan)
        "0x1400": [],                   # no dependencies
    }
    constructs = {0x20: "0x1000", 0x40: "0x1100", 0x60: "0x1200", 0x70: "0x1300"}
    names = {"0x1000": "UObject", "0x1100": "AA", "0x1200": "BB", "0x1300": "CC", "0x1400": None}
    resolver = Resolver(blocks, constructs, names, probe=probe, orphan_names={0x50: "Orphan"})
    out = build_records(resolver)
    check("root has no parent", out["0x1000"]["parent_source"] == "root" and out["0x1000"]["parent_name"] is None)
    check("direct parent resolves", out["0x1100"]["parent_va"] == "0x1000" and out["0x1100"]["parent_name"] == "UObject")
    check("thunk parent resolves", out["0x1200"]["parent_source"] == "thunk_block" and out["0x1200"]["parent_va"] == "0x1100")
    check("chain through thunk ends at UObject", out["0x1200"]["chain"] == ["BB", "AA", "UObject"] and out["0x1200"]["chain_ok"])
    check("orphan parent is not a block", out["0x1300"]["parent_source"] == "orphan" and out["0x1300"]["parent_va"] == "0x50")
    check("orphan parent named from headers", out["0x1300"]["parent_name"] == "Orphan" and not out["0x1300"]["chain_ok"])
    check("no dependencies gives no parent", out["0x1400"]["parent_source"] == "no_dependencies" and not out["0x1400"]["chain_ok"])
    check("package recognised", out["0x1100"]["package"] == "/Script/Test")
    check("base is not a parent", out["0x1100"]["parent_va"] != "0x10")
    if failures:
        print("self-test FAILED: %s" % failures, file=sys.stderr)
        return 1
    print("self-test passed (%d checks)" % count[0])
    return 0


# ---------------------------------------------------------------- main

def load_inputs(args):
    with open(args.reflection, "r", encoding="utf-8") as fh:
        reflection = json.load(fh)
    with open(args.evidence, "r", encoding="utf-8") as fh:
        evidence = json.load(fh)
    with open(args.name_map, "r", encoding="utf-8") as fh:
        names_raw = json.load(fh)
    blocks_list = reflection.get("classes_in_groups_with_fort_names") or reflection.get("classes") or []
    blocks = {}
    for blk in blocks_list:
        va = blk["block_va"].lower()
        blocks[va] = [int(d, 16) for d in blk.get("dependency_fns") or [] if d]
    constructs = {}
    for ev in evidence["blocks"]:
        fns = ev.get("construct_fns") or []
        if len(fns) == 1:
            constructs[int(fns[0], 16)] = ev["block_va"].lower()
        elif len(fns) > 1:
            raise SystemExit("block %s has %d construct functions; expected one" % (ev["block_va"], len(fns)))
    names = {k.lower(): v for k, v in names_raw.items()}
    return blocks, constructs, names


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--reflection", default=DEFAULT_REFLECTION)
    ap.add_argument("--evidence", default=DEFAULT_EVIDENCE)
    ap.add_argument("--name-map", default=DEFAULT_NAMES)
    ap.add_argument("--image", default=DEFAULT_IMAGE if os.path.exists(DEFAULT_IMAGE) else None,
                    help="PE for thunk and package checks (read-only); default: the runtime dump if present")
    ap.add_argument("--headers", default=DEFAULT_HEADERS, help="UE 4.25 Engine directory (Source and Plugins)")
    ap.add_argument("--out", default=DEFAULT_OUT)
    ap.add_argument("--no-validate", action="store_true", help="skip the header scan and summary")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)
    if args.self_test:
        return self_test()

    blocks, constructs, names = load_inputs(args)
    probe = Probe(Image(args.image)) if args.image else None
    resolver = Resolver(blocks, constructs, names, probe=probe)
    index = None
    if not args.no_validate and args.headers and os.path.isdir(args.headers):
        roots = [p for p in (os.path.join(args.headers, "Source"), os.path.join(args.headers, "Plugins"))
                 if os.path.isdir(p)] or [args.headers]
        index = index_headers(roots)
        name_report = name_orphans(resolver, index)
    else:
        name_report = {}
    out = build_records(resolver)
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as fh:
        json.dump(out, fh, indent=1)

    stats = collections.Counter(r["parent_source"] for r in resolver.recs.values())
    named = [va for va in out if out[va]["name"]]
    chain_ok = sum(1 for va in named if out[va]["chain_ok"])
    print("blocks %d, named %d, parent sources %s" % (len(out), len(named), dict(stats)))
    print("named blocks with a resolved parent (block or thunk_block or root): %d" % sum(
        1 for va in named if resolver.recs[va]["parent_source"] in ("block", "thunk_block", "root")))
    print("named blocks with chain_ok (chain to UObject through blocks): %d of %d" % (chain_ok, len(named)))
    if probe:
        pkgs = collections.Counter(out[va]["package"] for va in out if out[va]["package"])
        print("package recognised for %d blocks; packages: %s" % (sum(pkgs.values()), dict(pkgs.most_common(12))))
    if name_report:
        print("orphan parents (no FClassParams block): %s" % json.dumps(
            {hexva(fn) if isinstance(fn, int) else fn: resolver.orphan_names.get(fn) for fn in resolver.orphan_names}))
        print("orphan vote detail: %s" % json.dumps(name_report))
    if index is not None:
        validate(out, resolver, index)
    print("wrote %s" % args.out)
    return 0


def validate(out, resolver, index):
    by_name = {out[va]["name"]: va for va in out if out[va]["name"]}
    print("\nchains (name -> ... -> root):")
    for name in TARGETS + ENGINE_CHECK + ["AInfo", "AController", "UObject"]:
        va = by_name.get(name)
        if va is None:
            print("  %-28s MISSING" % name)
            continue
        hb = header_base(index, name)
        print("  %-28s %s   [header base: %s]" % (name, " -> ".join(out[va]["chain"]), hb))

    # Accuracy: named blocks with a UCLASS/UINTERFACE declaration in the UE 4.25 headers whose resolved
    # parent is a named block (binary parent) compared with the header base class.
    checked = 0
    matched = 0
    mismatches = []
    orphan_checked = 0
    orphan_matched = 0
    no_header = 0
    for va, rec in out.items():
        name = rec["name"]
        if not name:
            continue
        entry = index.get(name)
        if not entry:
            no_header += 1
            continue
        hb = header_base(index, name)
        if hb is None:
            continue
        src = resolver.recs[va]["parent_source"]
        if src in ("block", "thunk_block"):
            if hb not in index:
                continue
            checked += 1
            if rec["parent_name"] == hb:
                matched += 1
            else:
                mismatches.append((name, rec["parent_name"], hb))
        elif src == "orphan":
            orphan_checked += 1
            orphan_matched += 1 if rec["parent_name"] == hb else 0
        elif src == "root":
            # UObject: the header base is a non-UCLASS C++ class (UObjectBaseUtility), so the root check is "no UCLASS parent".
            if hb not in index and rec["parent_name"] is None:
                checked += 1
                matched += 1
    print("\nheader accuracy (binary parent vs UE 4.25 header base, named blocks with headers): %d of %d = %.4f"
          % (matched, checked, matched / checked if checked else 0.0))
    print("  named blocks without a header declaration (Fort or game-only classes): %d" % no_header)
    print("  orphan parents compared with child headers: %d of %d agree" % (orphan_matched, orphan_checked))
    if mismatches:
        print("  mismatches (%d): %s" % (len(mismatches), mismatches[:25]))
    eng = [(n, out[by_name[n]]["parent_name"], header_base(index, n)) for n in ENGINE_CHECK + ["AInfo", "AController"]
           if n in by_name]
    ok = sum(1 for n, p, h in eng if p == h)
    print("  engine list (%d): %d of %d match: %s" % (len(eng), ok, len(eng),
                                                     ", ".join("%s<-%s%s" % (n, p, "" if p == h else "(hdr %s)" % h)
                                                               for n, p, h in eng)))


if __name__ == "__main__":
    sys.exit(main())
