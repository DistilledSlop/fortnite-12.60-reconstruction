#!/usr/bin/env python3
"""Offline UE 4.25 class net checksums from the recovered reflection tables (stdlib only).

Computes what the engine computes for a class, without a native build (UE 4.25 sources, cited by line):

  ClassChecksum (CoreNet.cpp GetClassNetCache, Class.cpp SetUpRuntimeReplicationData):
                      ClassChecksum(C) = fold GetPropertyChecksum over C's owned replicated properties, sorted by
                      (offset, lowercased name), starting from ClassChecksum(super). Static arrays count once.
                      This is the value registered as the class GUID's NetworkChecksum (PackageMapClient.cpp
                      GetClassNetworkChecksum), so it is chained through the super class.
  GetPropertyChecksum(Property, Checksum, bIncludeChildren):
                      CRC(lower name), CRC(lower GetCPPType(nullptr, 0)), CRC("%u" ArrayDim); if children:
                      array -> recurse into the inner property; struct -> CRC(lower struct name), then the struct's
                      non-RepSkip fields sorted by (offset, name), each with children.
  GetFunctionChecksum: CRC(lower name), CRC("%u" FunctionFlags). Seeded with 0 per function (GetFieldChecksum(F, 0)).
  NetFields:          UFunctions with FUNC_Net that do not override a function of a super class, sorted by name.
  RPC matching:       UActorChannel::WriteFieldHeaderAndPayload / DataChannel.cpp: the NetFieldExportGroup of an
                      object class holds the FieldChecksum of every net function of the class AND of its super
                      classes (for (C = ClassCache; C; C = C->GetSuper())). Functions are matched by that checksum
                      (FindNetFieldExportHandleByChecksum). The RPC side is therefore a SET of per-function
                      checksums over the class chain. It is NOT chained: an RPC checksum never folds the parent's
                      value. (The v1 tool chained it, which is not what CoreNet.cpp does; kept as rpc_chain_v1 for
                      comparison only.)

CRC: FCrc::StrCrc32 on a 16-bit TCHAR (Windows client): per code unit c the table step runs on c & 0xFF, then on
(c >> 8) & 0xFF, then twice on 0. The result is checked against the handshake value of the original client
(683914758 = StrCrc32("fortnitegame 1.0.0, netcl: 12896485, enginenetver: 15, gamenetver: 0"), see --self-test).

C++ type strings (GetCPPType(nullptr, 0), UE 4.25 CoreUObject Private/UObject/Property*.cpp, validated in --self-test):
  Object -> prefix+Name+"*"; WeakObject -> TWeakObjectPtr<..> (TAutoWeakObjectPtr<..> with CPF_AutoWeak);
  Class -> TSubclassOf<..> + " " (trailing space, with CPF_UObjectWrapper) or "UClass*"; SoftClass -> TSoftClassPtr<..> + " ";
  SoftObject -> TSoftObjectPtr<..>; LazyObject -> TLazyObjectPtr<..>; Interface -> TScriptInterface;
  Struct -> "F"+Name; Enum -> CppType or Name; Byte with enum -> Name (EnumClass form) or TEnumAsByte<Name>;
  Bool -> bool (native bool flag) or uintN (bitfield); Array -> TArray; Map -> TMap; Set -> TSet;
  Delegate kinds -> "F" + signature name without "__DelegateSignature"; Name/Str/Text -> FName/FString/FText;
  numeric kinds -> int8..uint64, float, double. Prefix: "A" for Actor-derived classes, "U" otherwise (the name map
  already carries it), with "DEPRECATED_" when CLASS_Deprecated.

Byte offsets of bool members are not in the param struct. UE computes them from SetBitFunc at load time. The setter is
`or byte ptr [rcx+disp], mask; ret` (or `mov byte ptr [rcx+disp], imm; ret`); the byte at disp is the offset.
The setter code is read from the runtime dump (the shipping exe is encrypted on disk).

Emitted side (ground truth): the generator output under porting/native/Source/<Module>/Generated/<Class>.h. A
replicated property is emitted when the class header declares it UPROPERTY(Replicated[Using=...]); its checksum uses
the header's declared C++ type and array dimension (the reflection supplies offsets and the rest). A net RPC is
emitted when the header declares it UFUNCTION(Server|Client|NetMulticast, ...), or UFUNCTION(ServiceRequest(...)) or
UFUNCTION(ServiceResponse(...)) for a net service function (no role bit: its flags are NetRequest or NetResponse). The
header's table-index comment (flags) and role specifiers are cross-checked against the reflection. Structs are taken as the reflection describes
them (their emitted headers are checked for existence only).

Inputs (porting/generated/reflection/, git-ignored): fortnite_game_reflection.json (with set_bit_fn),
class_name_map.json, class_parent_map.json, class_ref_map.json, construct_fn_to_block.json, struct_blocks.json,
enum_blocks.json, delegate_blocks.json, skipped_properties.json (reason of each skip, used to explain gaps); the
runtime dump for bool setters; porting/native_class_set.txt (every '#' comment line is ignored, the round is taken
from the most recent '# Round N' line); porting/native/Source/*/Generated/*.h.

Outputs: porting/generated/reflection/class_checksums.json (every class of the set) and the markdown block between
the markers of porting/class_checksum_report.md (replaced in place, or appended when absent).

Usage:
  python3 -I porting/class_checksum.py [--image EXE] [--out JSON] [--report-md MD] [--no-md] [--set LIST]
                                       [--source-root DIR]
  python3 -I porting/class_checksum.py --self-test [--image EXE]
"""
import argparse
import collections
import glob
import json
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import extract_reflection as ER  # noqa: E402  (Image reader shared with the extractor)

REFL = os.path.join(HERE, "generated", "reflection")
IMAGE = os.path.join(HERE, "generated", "dump", "FortniteClient-Win64-Shipping.runtime.exe")
OUT = os.path.join(REFL, "class_checksums.json")
REPORT_MD = os.path.join(HERE, "class_checksum_report.md")
CLASS_SET = os.path.join(HERE, "native_class_set.txt")
SOURCE_ROOT = os.path.join(HERE, "native", "Source")
ENGINE_SRC = os.environ.get("FN_ENGINE_RUNTIME", "/opt/fortnite/UnrealEngine/Engine/Source/Runtime")
ENGINE_HEADERS = {
    "AActor": "Engine/Classes/GameFramework/Actor.h",
    "APawn": "Engine/Classes/GameFramework/Pawn.h",
    "ACharacter": "Engine/Classes/GameFramework/Character.h",
    "APlayerState": "Engine/Classes/GameFramework/PlayerState.h",
    "AGameStateBase": "Engine/Classes/GameFramework/GameStateBase.h",
    "AWorldSettings": "Engine/Classes/GameFramework/WorldSettings.h",
}
HANDSHAKE_STRING = "FortniteGame 1.0.0, NetCL: 12896485, EngineNetVer: 15, GameNetVer: 0"
HANDSHAKE_CHECKSUM = 683914758          # client log "(Checksum: 683914758)", see native_classes_status.md step 0
MD_BEGIN = "<!-- BEGIN class-checksum-parity (porting/class_checksum.py) -->"
MD_END = "<!-- END class-checksum-parity -->"

M32 = 0xFFFFFFFF
CPF_NET = 0x00000020
CPF_REPSKIP = 0x80000000
CPF_AUTOWEAK = 0x0000004000000000
CPF_UOBJECTWRAPPER = 0x0004000000000000
CLASS_DEPRECATED = 0x02000000
FUNC_NET = 0x00000040
FUNC_NET_RELIABLE = 0x00000080
FUNC_NET_MULTICAST = 0x00004000
FUNC_NET_SERVER = 0x00200000
FUNC_NET_CLIENT = 0x01000000
FUNC_NET_REQUEST = 0x00000100   # UE net service request (UHT ServiceRequest)
FUNC_NET_RESPONSE = 0x00001000  # UE net service response (UHT ServiceResponse)
FUNC_CONST = 0x40000000
# Specifiers of a net function that must agree with its reflection flags: the roles, and const (the RPC checksum
# covers the flags, so a const RPC without FUNC_Const would not match).
ROLE_BITS = {"Server": FUNC_NET_SERVER, "Client": FUNC_NET_CLIENT, "NetMulticast": FUNC_NET_MULTICAST,
             "Request": FUNC_NET_REQUEST, "Response": FUNC_NET_RESPONSE, "Const": FUNC_CONST}
GEN_NATIVE_BOOL = 0x20
DELEGATE_SUFFIX = "__DelegateSignature"
TABLE_FLAGS_RE = re.compile(r"table index \d+, flags (0x[0-9A-Fa-f]+)")
ENGINE_PACKAGES = ("/Script/Engine", "/Script/CoreUObject")
# CoreUObject intrinsic UClass constructors without a name literal or FClassParams block. Names come from the child
# headers (porting/binary_analysis.md section 8, step 4), or from a property literal (UStruct). Their refs stay
# unresolved in class_ref_map.json: there is no block to map them to.
# 0x142e49d80 is the UStruct constructor: its body builds the property "SuperStruct" (UStruct's own property), in the
# UClass -> UStruct -> UField chain of binary_analysis.md section 8.
INTRINSIC_CLASS_REFS = {
    "0x142e497d0": "UClass",
    "0x142e49a40": "UEnum",
    "0x142e49c00": "UPackageMap",
    "0x142e49c80": "UScriptStruct",
    "0x142e73ed0": "UInterface",
    "0x142e49d80": "UStruct",
}
NUMERIC_TYPES = {
    "Int8": "int8", "Int16": "int16", "Int": "int32", "Int64": "int64", "UInt16": "uint16",
    "UInt32": "uint32", "UInt64": "uint64", "Float": "float", "Double": "double",
    "Name": "FName", "Str": "FString", "Text": "FText",
}
BOOL_UINT = {1: "uint8", 2: "uint16", 4: "uint32", 8: "uint64"}


def _make_crc_table():
    table = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = (c >> 1) ^ 0xEDB88320 if c & 1 else c >> 1
        table.append(c)
    return table


CRC_TABLE = _make_crc_table()


def str_crc32(text, crc=0):
    """FCrc::StrCrc32 for a 16-bit TCHAR (the generic template, sizeof(CharType) == 2)."""
    crc = ~crc & M32
    for ch in text:
        c = ord(ch) & 0xFFFF
        crc = ((crc >> 8) ^ CRC_TABLE[(crc ^ c) & 0xFF]) & M32
        c >>= 8
        crc = ((crc >> 8) ^ CRC_TABLE[(crc ^ c) & 0xFF]) & M32
        c >>= 8
        crc = ((crc >> 8) ^ CRC_TABLE[(crc ^ c) & 0xFF]) & M32
        c >>= 8
        crc = ((crc >> 8) ^ CRC_TABLE[(crc ^ c) & 0xFF]) & M32
    return ~crc & M32


def lower(text):
    """FString::ToLower for the ASCII names used here."""
    return "".join(chr(ord(c) + 32) if "A" <= c <= "Z" else c for c in text)


def sort_key_name(name):
    """FString operator< is FPlatformString::Stricmp: compare the lowercased strings."""
    return lower(name)


class UnknownType(Exception):
    """A property, struct member or RPC parameter whose C++ type cannot be recovered from the tables."""


def hexint(value):
    return int(str(value), 16)


def bool_setter_offset(img, va):
    """(offset, mask) of the first byte a bool setter writes, as UE's DoDetermineBitfieldOffsetAndMask finds it.

    The setter is one store into the object (rcx) followed by ret: or/mov on a byte (80/C6), a dword (81/83/C7).
    The store is with a [rcx+disp8] or [rcx+disp32] operand. The offset is disp plus the index of the first nonzero
    byte of the immediate (little endian). Returns None for any other shape.
    """
    if not va:
        return None
    off = img.off(va)
    if off is None:
        return None
    b = img.m[off:off + 16]
    op = b[0]
    if op not in (0x80, 0x81, 0x83, 0xC6, 0xC7):
        return None
    modrm = b[1]
    is_or = op in (0x80, 0x81, 0x83)
    if is_or and modrm not in (0x49, 0x89):          # reg=/1 (or), base=rcx
        return None
    if not is_or and modrm not in (0x41, 0x81):      # reg=/0 (mov), base=rcx
        return None
    if modrm in (0x49, 0x41):
        disp, p = b[2], 3
    else:
        disp, p = struct.unpack_from("<I", b, 2)[0], 6
    if op in (0x80, 0xC6):
        imm = b[p]
        p += 1
        if b[p] != 0xC3:
            return None
        return disp, imm
    if op == 0x83:
        imm8 = b[p]
        imm = (imm8 | 0xFFFFFF00) if imm8 & 0x80 else imm8
        p += 1
    else:
        imm = struct.unpack_from("<I", b, p)[0]
        p += 4
    if b[p] != 0xC3:
        return None
    value = (imm & M32).to_bytes(4, "little")
    for k in range(4):
        if value[k]:
            return disp + k, value[k]
    return None


# ------------------------------------------------------------------ generated headers (emitted side)

def _balanced(text, start):
    """Index just past the ')' that closes the '(' at text[start]."""
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i + 1
    return len(text)


def header_type(ctype, bitfield):
    """Normalises a declaration type to the GetCPPType string (the same rules as Model.cpp_type)."""
    t = " ".join(ctype.replace("const ", "").split())
    t = re.sub(r"\b(struct|enum|class) (?=[A-Za-z_])", "", t)      # elaborated type specifiers
    if bitfield and t == "uint8":
        return "uint8"
    for prefix, out in (("TArray<", "TArray"), ("TMap<", "TMap"), ("TSet<", "TSet")):
        if t.startswith(prefix):
            return out
    if t.startswith("TSubclassOf<") and t.endswith(">"):
        return t + " "
    if t.startswith("TSoftClassPtr<") and t.endswith(">"):
        return t + " "
    return t.replace(" *", "*")


def scan_members(text, class_name):
    """UPROPERTY/UFUNCTION members of the class body: [{kind, args, statement, table_flags}] in source order.

    Comments are blanked out with same-length spaces, so the positions of the '// table index N, flags F' comments
    in the raw text line up with the cleaned text. A member's table flags are the last such comment between the
    previous member and its own UPROPERTY.
    """
    clean = re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)
    clean = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), clean, flags=re.S)
    decl = re.search(r"\bclass\s+(?:[A-Z_0-9]+\s+)?%s\s*:" % re.escape(class_name), clean)
    if not decl:
        raise UnknownType("class %s not found in its generated header" % class_name)
    open_brace = clean.index("{", decl.end())
    depth = 0
    end = len(clean)
    for i in range(open_brace, len(clean)):
        if clean[i] == "{":
            depth += 1
        elif clean[i] == "}":
            depth -= 1
            if depth == 0:
                end = i
                break
    marks = [(m.start(), int(m.group(1), 16)) for m in TABLE_FLAGS_RE.finditer(text)]
    members = []
    prev_end = open_brace
    for m in re.finditer(r"\b(UPROPERTY|UFUNCTION)\(", clean):
        idx = m.start()
        if idx < prev_end or idx >= end:
            continue
        close = _balanced(clean, m.end() - 1)
        semi = clean.find(";", close)
        if semi < 0 or semi > end:
            break
        flags = [v for p, v in marks if prev_end <= p < idx]
        members.append({
            "kind": m.group(1),
            "args": clean[m.end() - 1:close],
            "statement": " ".join(clean[close:semi].split()),
            "table_flags": flags[-1] if flags else None,
        })
        prev_end = semi + 1
    return members


def parse_class_header(text, class_name):
    """(replicated, rpcs) declared by one generated class.

    replicated: [{name, type, dim (int or None), table_flags}] for UPROPERTY(Replicated / ReplicatedUsing=...).
    rpcs:       [{name, roles, reliable}] for UFUNCTION with a Server, Client, NetMulticast, ServiceRequest or
                ServiceResponse specifier. roles also holds "Const" for a const function (see ROLE_BITS).
    """
    rep = []
    rpcs = []
    for mem in scan_members(text, class_name):
        if mem["kind"] == "UPROPERTY":
            if not re.search(r"\breplicated\b|replicatedusing\s*=", mem["args"], re.I):
                continue
            statement = mem["statement"]
            bitfield = re.search(r":\s*\d+$", statement)
            statement = re.sub(r":\s*\d+$", "", statement).strip()
            m = re.match(r"(?P<type>.+?)\s+(?P<name>[A-Za-z_]\w*)\s*(?P<dim>\[[^\]]*\])?$", statement)
            if not m:
                raise UnknownType("cannot parse replicated declaration: %s" % statement)
            dim = 1
            if m.group("dim"):
                inner = m.group("dim")[1:-1].strip()
                dim = int(inner, 0) if inner.isdigit() else None
            rep.append({"name": m.group("name"), "type": header_type(m.group("type"), bool(bitfield)),
                        "dim": dim, "table_flags": mem["table_flags"]})
        else:
            roles = set(re.findall(r"\b(Server|Client|NetMulticast)\b", mem["args"]))
            if re.search(r"\bServiceRequest\b", mem["args"]):
                roles.add("Request")
            if re.search(r"\bServiceResponse\b", mem["args"]):
                roles.add("Response")
            if not roles:
                continue
            fm = re.match(r"[^(]*?(\w+)\s*\(", mem["statement"])
            if not fm:
                raise UnknownType("cannot parse RPC declaration: %s" % mem["statement"])
            if re.search(r"\)\s*const\s*$", mem["statement"]):
                roles.add("Const")
            rpcs.append({"name": fm.group(1), "roles": sorted(roles),
                         "reliable": bool(re.search(r"\bReliable\b", mem["args"]))})
    return rep, rpcs


def header_replicated(path, class_name):
    """[(name, normalised C++ type)] for UPROPERTY(...Replicated...) members of the class declared in path."""
    with open(path, encoding="utf-8", errors="replace") as fh:
        text = fh.read()
    rep, _ = parse_class_header(text, class_name)
    return [(r["name"], r["type"]) for r in rep]


def index_headers(root):
    """{class name: (module dir, path)} for every porting/native/Source/<Module>/Generated/<Name>.h."""
    out = {}
    for path in sorted(glob.glob(os.path.join(root, "*", "Generated", "*.h"))):
        module = os.path.basename(os.path.dirname(os.path.dirname(path)))
        out[os.path.basename(path)[:-2]] = (module, path)
    return out


def load_class_set(path):
    """[(name, round)] for every entry of the class set, in file order. '#' lines are comments; '# Round N' sets N."""
    rnd = 1
    out = []
    seen = set()
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            if line.startswith("#"):
                # The header comment block names every round before the first entry; only the section markers
                # that come after the first entry set the round of the entries that follow.
                m = re.match(r"#\s*Round\s+(\d+)", line)
                if m and out:
                    rnd = int(m.group(1))
                continue
            if line in seen:
                raise SystemExit("class set lists %s twice" % line)
            seen.add(line)
            out.append((line, rnd))
    return out


# ------------------------------------------------------------------ model

class Model:
    """The recovered tables, indexed for the checksum walk, and the emitted headers of the class set."""

    def __init__(self, refl_dir, image_path, skipped_path=None, set_path=CLASS_SET, source_root=SOURCE_ROOT):
        self.refl_dir = refl_dir
        self.img = None
        self.image_path = image_path
        with open(os.path.join(refl_dir, "fortnite_game_reflection.json"), encoding="utf-8") as fh:
            data = json.load(fh)
        self.blocks = {b["block_va"].lower(): b for b in data["classes_in_groups_with_fort_names"]}
        self.names = {k.lower(): v for k, v in self._load("class_name_map.json").items()}
        self.parents = {k.lower(): v for k, v in self._load("class_parent_map.json").items()}
        self.evidence = {}
        for rec in self._load("class_name_map.evidence.json", "blocks"):
            self.evidence[rec["block_va"].lower()] = [hexint(f) for f in rec.get("construct_fns") or []]
        self.class_ref = {k.lower(): v.lower() for k, v in self._load("class_ref_map.json", "resolved").items()}
        self.construct = {k.lower(): v for k, v in self._load("construct_fn_to_block.json", "map").items()}
        self.structs = {s["block_va"].lower(): s for s in self._load("struct_blocks.json", "structs")}
        self.enums = {e["block_va"].lower(): e for e in self._load("enum_blocks.json", "enums")}
        self.delegates = {d["block_va"].lower(): d for d in self._load("delegate_blocks.json", "delegates")}
        self.by_name = {}
        for va, name in self.names.items():
            if name and name[0] in "AUFIETS":
                self.by_name.setdefault(name[1:], []).append(va)
        skipped_path = skipped_path or os.path.join(refl_dir, "skipped_properties.json")
        with open(skipped_path, encoding="utf-8") as fh:
            skipped = json.load(fh)
        self.skip_props = {}
        self.skip_funcs = {}
        self.skip_rep_by_class = collections.defaultdict(set)
        self.skip_fn_by_class = collections.defaultdict(set)
        for row in skipped.get("properties", []):
            self.skip_props[(row["class"], row["property"])] = row
            if row.get("replicated"):
                self.skip_rep_by_class[row["class"]].add(row["property"])
        for row in skipped.get("functions", []):
            self.skip_funcs[(row["class"], row["name"])] = row
            self.skip_fn_by_class[row["class"]].add(row["name"])
        # The class set (every entry) and the emitted headers of each entry.
        self.set_entries = load_class_set(set_path)
        self.set_vas = {}
        self.unresolved_set = []
        for name, rnd in self.set_entries:
            match = [va for va in self.by_name.get(name[1:], []) if self.names.get(va) == name]
            if len(match) == 1:
                self.set_vas[match[0]] = (name, rnd)
            else:
                self.unresolved_set.append(name)
        self.headers = index_headers(source_root)
        self.unresolved_tokens = set()
        self.bool_cache = {}
        self.bool_failures = []
        self._memo = {}
        self._funcs_memo = {}
        self._gen = {}
        self._gaps = {}

    def _load(self, name, key=None):
        with open(os.path.join(self.refl_dir, name), encoding="utf-8") as fh:
            data = json.load(fh)
        return data[key] if key else data

    def image(self):
        if self.img is None:
            self.img = ER.Image(self.image_path)
        return self.img

    # ---- class identity -------------------------------------------------------------------------
    def cpp_class_name(self, va):
        """UClass::GetPrefixCPP() + GetName() for the class block va."""
        name = self.names.get(va)
        if not name:
            raise UnknownType("class block %s has no name" % va)
        prefix = name[0] + ("DEPRECATED_" if (hexint(self.blocks[va]["class_flags"]) & CLASS_DEPRECATED) else "")
        return prefix + name[1:]

    def class_ref_block(self, ref):
        """(block_va or None, cpp name) for a class reference (type_refs value)."""
        ref = ref.lower()
        if ref in self.class_ref:
            va = self.class_ref[ref]
            return va, self.cpp_class_name(va)
        if ref in INTRINSIC_CLASS_REFS:
            name = INTRINSIC_CLASS_REFS[ref]
            return None, name
        raise UnknownType("class reference %s is not resolved" % ref)

    def construct_block(self, ref, kind):
        rec = self.construct.get(ref.lower())
        if not rec or rec["kind"] != kind:
            raise UnknownType("%s reference %s has no %s block" % (kind, ref, kind))
        return rec["block_va"].lower()

    def block_label(self, va, fallback):
        """'struct FX' / 'enum EX' / 'delegate FX' / 'class AX' for a block va (the fix's name)."""
        if va in self.structs:
            return "struct F" + self.structs[va]["name"]
        if va in self.enums:
            e = self.enums[va]
            return "enum " + (e.get("cpp_type") or e["name"])
        if va in self.delegates:
            name = self.delegates[va]["name"]
            if name.endswith(DELEGATE_SUFFIX):
                name = name[:-len(DELEGATE_SUFFIX)]
            return "delegate F" + name
        if va in self.names:
            return "class " + self.names[va]
        return fallback

    def resolve_token(self, token):
        """A skip token (TODO_<Kind>_<ctor va>) as the named type it stands for, else the token itself."""
        m = re.match(r"^TODO_([A-Za-z]+)_([0-9a-fA-F]{6,})$", token)
        if not m:
            return token
        addr = "0x" + m.group(2).lower()
        rec = self.construct.get(addr)
        if rec:
            return self.block_label(rec["block_va"].lower(), token)
        if addr in self.class_ref:                      # class constructors (ClassOutsideNativeSet, Class, Interface)
            return self.block_label(self.class_ref[addr], token)
        self.unresolved_tokens.add(token)
        return token

    def struct_label(self, ref):
        try:
            return self.block_label(self.construct_block(ref, "struct"), "struct ?")
        except UnknownType:
            return "struct (unresolved reference)"

    # ---- parents -----------------------------------------------------------------------------------
    def parent_va(self, va):
        rec = self.parents.get(va)
        if not rec:
            return None
        pva = (rec.get("parent_va") or "").lower() or None
        return pva if pva in self.blocks else None

    def ancestors(self, va):
        out = []
        seen = set()
        cur = self.parent_va(va)
        while cur and cur not in seen:
            seen.add(cur)
            out.append(cur)
            cur = self.parent_va(cur)
        return out

    def package(self, va):
        return self.parents.get(va, {}).get("package")

    # ---- properties --------------------------------------------------------------------------------
    @staticmethod
    def _records(props):
        """Child-property records: the array inner and enum underlying records are not UE child properties."""
        out = []
        for p in props or []:
            if p.get("array_inner_of") or p.get("enum_underlying_of"):
                continue
            out.append(p)
        return out

    def offset(self, prop):
        if prop["kind"] != "Bool":
            if prop.get("offset") is None:
                raise UnknownType("property %s has no offset" % prop.get("name"))
            return prop["offset"]
        set_bit = prop.get("set_bit_fn")
        if not set_bit:
            return 0                                  # no setter: UE leaves Offset at 0
        key = set_bit.lower()
        if key not in self.bool_cache:
            found = bool_setter_offset(self.image(), hexint(key))
            self.bool_cache[key] = found
            if found is None:
                self.bool_failures.append(key)
        found = self.bool_cache[key]
        if found is None:
            raise UnknownType("bool setter %s has an unknown shape" % key)
        return found[0]

    def inner_of(self, outer_props, prop):
        for p in outer_props:
            if p.get("array_inner_of") == prop.get("name") and p.get("kind") != "Array":
                return p
        raise UnknownType("array %s has no inner record" % prop.get("name"))

    def cpp_type(self, prop):
        """GetCPPType(nullptr, 0) for one property record (UE 4.25 rules listed in the module docstring)."""
        kind = prop["kind"]
        refs = prop.get("type_refs") or {}
        if kind == "Bool":
            if int(prop["kind_raw"], 16) & GEN_NATIVE_BOOL:
                return "bool"
            size = prop.get("element_size") or 1
            return BOOL_UINT.get(size, "uint8")
        if kind in NUMERIC_TYPES:
            return NUMERIC_TYPES[kind]
        if kind == "Byte":
            if "enum" not in refs:
                return "uint8"
            enum = self.enums[self.construct_block(refs["enum"], "enum")]
            name = enum["cpp_type"] or enum["name"]
            return name if enum["cpp_form"] == "EnumClass" else "TEnumAsByte<%s>" % name
        if kind == "Enum":
            enum = self.enums[self.construct_block(refs["enum"], "enum")]
            return enum["cpp_type"] or enum["name"]
        if kind == "Struct":
            struct = self.structs[self.construct_block(refs["struct"], "struct")]
            return "F" + struct["name"]
        if kind in ("Delegate", "InlineMulticastDelegate", "SparseMulticastDelegate"):
            sig = self.delegates[self.construct_block(refs["signature"], "delegate")]
            return "F" + sig["name"][:-len(DELEGATE_SUFFIX)]   # LeftChop by the suffix length, as UE does
        if kind == "Object":
            _, name = self.class_ref_block(refs["class"])
            return name + "*"
        if kind == "WeakObject":
            _, name = self.class_ref_block(refs["class"])
            return ("TAutoWeakObjectPtr<%s>" if prop.get("property_flags") and
                    int(prop["property_flags"], 16) & CPF_AUTOWEAK else "TWeakObjectPtr<%s>") % name
        if kind == "LazyObject":
            _, name = self.class_ref_block(refs["class"])
            return "TLazyObjectPtr<%s>" % name
        if kind == "SoftObject":
            _, name = self.class_ref_block(refs["class"])
            return "TSoftObjectPtr<%s>" % name
        if kind == "Class":
            _, name = self.class_ref_block(refs["meta_class"])
            if int(prop["property_flags"], 16) & CPF_UOBJECTWRAPPER:
                return "TSubclassOf<%s> " % name
            return "UClass*"
        if kind == "SoftClass":
            _, name = self.class_ref_block(refs["meta_class"])
            return "TSoftClassPtr<%s> " % name
        if kind == "Interface":
            return "TScriptInterface"
        if kind == "Array":
            return "TArray"
        if kind == "Map":
            return "TMap"
        if kind == "Set":
            return "TSet"
        if kind == "FieldPath":
            return "TFieldPath"
        raise UnknownType("property kind %s has no type rule" % kind)

    # ---- checksums ---------------------------------------------------------------------------------
    def struct_checksum(self, struct_va, crc):
        struct = self.structs[struct_va]
        fields = [p for p in self._records(struct.get("properties")) if not (int(p["property_flags"], 16) & CPF_REPSKIP)]
        keyed = sorted(fields, key=lambda p: (self.offset(p), sort_key_name(p["name"])))
        for field in keyed:
            crc = self.property_checksum(field, crc, True, struct.get("properties"))
        return crc

    def property_checksum(self, prop, crc, include_children, outer_props, type_override=None, dim_override=None):
        crc = str_crc32(lower(prop["name"]), crc)
        crc = str_crc32(lower(type_override or self.cpp_type(prop)), crc)
        dim = dim_override if dim_override is not None else prop["array_dim"]
        crc = str_crc32(str(dim), crc)
        if include_children:
            if prop["kind"] == "Array":
                return self.property_checksum(self.inner_of(outer_props, prop), crc, True, outer_props)
            if prop["kind"] == "Struct":
                struct_va = self.construct_block(prop["type_refs"]["struct"], "struct")
                crc = str_crc32(lower(self.structs[struct_va]["name"]), crc)
                crc = self.struct_checksum(struct_va, crc)
        return crc

    def owned_replicated(self, va):
        props = self._records(self.blocks[va].get("properties"))
        return [p for p in props if int(p["property_flags"], 16) & CPF_NET]

    def generated(self, va):
        """Emitted view of a set class: {module, path, rep, rep_list, rpc, rpc_list}; None for non-set classes."""
        if va not in self.set_vas:
            return None
        if va in self._gen:
            return self._gen[va]
        name = self.names[va]
        hit = self.headers.get(name)
        if not hit:
            self._gen[va] = None
            return None
        module, path = hit
        with open(path, encoding="utf-8", errors="replace") as fh:
            text = fh.read()
        rep, rpcs = parse_class_header(text, name)
        rec = {"module": module, "path": os.path.relpath(path, HERE), "rep_list": rep,
               "rep": {r["name"]: r for r in rep}, "rpc_list": rpcs, "rpc": {r["name"]: r for r in rpcs}}
        self._gen[va] = rec
        return rec

    def emitted_rep(self, va, mode):
        """[(prop, type override, dim override)] of the owned replicated properties the class emits."""
        props = self.owned_replicated(va)
        g = self.generated(va) if mode == "emitted" else None
        if g is None:
            return [(p, None, None) for p in props]
        by_name = {p["name"]: p for p in props}
        return [(by_name[r["name"]], r["type"], r["dim"]) for r in g["rep_list"] if r["name"] in by_name]

    def class_checksum(self, va, mode):
        """ClassChecksum for mode 'expected' (full reflection) or 'emitted' (generated headers)."""
        key = ("prop", va, mode)
        if key in self._memo:
            value = self._memo[key]
            if isinstance(value, Exception):
                raise value
            return value
        try:
            parent = self.parent_va(va)
            crc = self.class_checksum(parent, mode) if parent else 0
            entries = self.emitted_rep(va, mode)
            entries.sort(key=lambda e: (self.offset(e[0]), sort_key_name(e[0]["name"])))
            for prop, tover, dover in entries:
                crc = self.property_checksum(prop, crc, True, self.blocks[va].get("properties"), tover, dover)
            self._memo[key] = crc
            return crc
        except UnknownType as exc:
            self._memo[key] = exc
            raise

    def own_functions(self, va):
        """(name, flags) for each function of the class block, with the candidate that belongs to this class."""
        block = self.blocks[va]
        own = set(self.evidence.get(va, []))
        out = []
        for fn in block.get("functions") or []:
            if fn.get("replicated_function") is not None:
                flags = hexint(fn["flags"][0])
            else:
                hits = [c for c in fn.get("candidates") or [] if c.get("outer_fn") and hexint(c["outer_fn"]) in own]
                if len(hits) != 1:
                    raise UnknownType("function %s flags are ambiguous" % fn["name"])
                flags = hexint(hits[0]["flags"])
            out.append((fn["name"], flags))
        return out

    def function_names(self, va):
        return [fn["name"] for fn in self.blocks[va].get("functions") or []]

    def net_functions(self, va):
        """NetFields of the class: FUNC_Net functions that do not override a super function, sorted by name."""
        key = ("net", va)
        if key in self._funcs_memo:
            return self._funcs_memo[key]
        ancestor_names = set()
        for anc in self.ancestors(va):
            for name in self.function_names(anc):
                ancestor_names.add(lower(name))
        net = [(n, f) for n, f in self.own_functions(va) if f & FUNC_NET and lower(n) not in ancestor_names]
        out = sorted(net, key=lambda nf: sort_key_name(nf[0]))
        self._funcs_memo[key] = out
        return out

    def emitted_net_functions(self, va, mode):
        nets = self.net_functions(va)
        g = self.generated(va) if mode == "emitted" else None
        if g is None:
            return nets
        return [(n, f) for n, f in nets if n in g["rpc"]]

    @staticmethod
    def function_checksum(name, flags, crc):
        crc = str_crc32(lower(name), crc)
        return str_crc32(str(flags & M32), crc)

    def rpc_group(self, va, mode):
        """Sorted per-function checksums over the class and its ancestors (the NetFieldExportGroup of UE)."""
        out = []
        for c in [va] + self.ancestors(va):
            for name, flags in self.emitted_net_functions(c, mode):
                out.append(self.function_checksum(name, flags, 0))
        return sorted(out)

    def rpc_chain_v1(self, va, mode):
        """The v1 tool's chained RPC digest. Not what CoreNet.cpp computes; kept only for comparison."""
        parent = self.parent_va(va)
        crc = self.rpc_chain_v1(parent, mode) if parent else 0
        for name, flags in self.emitted_net_functions(va, mode):
            crc = self.function_checksum(name, flags, crc)
        return crc

    # ---- gaps: expected minus emitted, with the skip reason of each ------------------------------
    def _prop_keys(self, va, prop, row):
        tokens = row.get("tokens") or []
        if tokens:
            return [self.resolve_token(t) for t in tokens]
        if row.get("category") == "struct_custom_netserialize":
            if prop["kind"] == "Struct":
                return ["custom NetSerialize " + self.struct_label(prop["type_refs"]["struct"])]
            inner = next((q for q in self.blocks[va].get("properties") or []
                          if q.get("array_inner_of") == prop["name"] and q.get("kind") == "Struct"), None)
            if inner:
                return ["custom NetSerialize " + self.struct_label(inner["type_refs"]["struct"])]
            return ["custom NetSerialize (struct not identified)"]
        return [row.get("category") or "UNEXPLAINED"]

    def _fn_keys(self, va, name, row):
        tokens = row.get("tokens") or []
        if row.get("category") == "net_flag_without_role":
            return ["RPC flags without a Server/Client/NetMulticast role"]
        if tokens:
            return [self.resolve_token(t) for t in tokens]
        if row.get("category") == "struct_custom_netserialize":
            fn = next((f for f in self.blocks[va].get("functions") or [] if f["name"] == name), None)
            structs = [self.struct_label(p["type_refs"]["struct"]) for p in (fn or {}).get("params") or []
                       if p.get("kind") == "Struct"]
            return ["custom NetSerialize " + s for s in structs] or ["custom NetSerialize (struct not identified)"]
        return [row.get("category") or "UNEXPLAINED"]

    def gaps(self, va):
        """Everything the emitted class does not match the reflection on, for one set class."""
        if va in self._gaps:
            return self._gaps[va]
        out = {"rep": [], "rpc": [], "header_only_rep": [], "header_only_rpc": [], "mismatch": [],
               "override_declared": [], "validation": collections.Counter()}
        g = self.generated(va)
        self._gaps[va] = out
        if g is None:
            return out
        cname = self.names[va]
        props = self.owned_replicated(va)
        by_name = {p["name"]: p for p in props}
        for p in props:
            if p["name"] in g["rep"]:
                continue
            row = self.skip_props.get((cname, p["name"]))
            if row is None:
                out["validation"]["gap_without_skip_row"] += 1
            out["rep"].append({"kind": "replicated", "name": p["name"], "type": self._safe_type(p),
                               "category": (row or {}).get("category") or "UNEXPLAINED",
                               "reason": (row or {}).get("reason") or "no skip row",
                               "keys": self._prop_keys(va, p, row or {})})
        gap_names = {e["name"] for e in out["rep"]}
        skip_names = self.skip_rep_by_class.get(cname, set())
        out["validation"]["skip_row_not_in_header"] += len(skip_names - gap_names)
        out["validation"]["header_gap_not_in_skip"] += len(gap_names - skip_names)
        for r in g["rep_list"]:
            p = by_name.get(r["name"])
            if p is None:
                out["header_only_rep"].append({"kind": "header_only_replicated", "name": r["name"],
                                               "type": r["type"], "category": "header_only",
                                               "reason": "declared replicated in the header, not in the reflection",
                                               "keys": ["header-only %s.%s" % (cname, r["name"])]})
                continue
            refl_type = self._safe_type(p)
            if refl_type != r["type"]:
                out["mismatch"].append({"kind": "type_mismatch", "name": p["name"], "type": r["type"],
                                        "reflection_type": refl_type, "category": "emitted_type_mismatch",
                                        "reason": "header type differs from the reflection's GetCPPType",
                                        "keys": ["type mismatch (header vs reflection)"]})
            if r["dim"] is not None and r["dim"] != p["array_dim"]:
                out["mismatch"].append({"kind": "dim_mismatch", "name": p["name"], "type": r["type"],
                                        "reflection_dim": p["array_dim"], "header_dim": r["dim"],
                                        "category": "emitted_type_mismatch",
                                        "reason": "header array dimension differs from the reflection",
                                        "keys": ["type mismatch (header vs reflection)"]})
            if r["table_flags"] is not None and r["table_flags"] != hexint(p["property_flags"]):
                out["validation"]["table_flags_mismatch"] += 1
            if p["kind"] == "Struct":
                try:
                    sblock = self.structs[self.construct_block(p["type_refs"]["struct"], "struct")]
                except UnknownType:
                    continue
                # Engine structs (FVector, ...) are not generated; a game struct must have its F<Name>.h.
                if sblock.get("package") not in ENGINE_PACKAGES and "F" + sblock["name"] not in self.headers:
                    out["validation"]["struct_header_missing"] += 1
        nets = self.net_functions(va)
        net_names = {n for n, _ in nets}
        anc_names = set()
        for anc in self.ancestors(va):
            anc_names.update(lower(n) for n in self.function_names(anc))
        for name, flags in nets:
            if name in g["rpc"]:
                roles = set(g["rpc"][name]["roles"])
                bits = {r for r, b in ROLE_BITS.items() if flags & b}
                if roles != bits:
                    out["validation"]["role_flag_mismatch"] += 1
                continue
            row = self.skip_funcs.get((cname, name))
            if row is None:
                out["validation"]["rpc_gap_without_skip_row"] += 1
            out["rpc"].append({"kind": "rpc", "name": name, "flags": "0x%X" % (flags & M32),
                               "category": (row or {}).get("category") or "UNEXPLAINED",
                               "reason": (row or {}).get("reason") or "no skip row",
                               "keys": self._fn_keys(va, name, row or {})})
        rpc_gap_names = {e["name"] for e in out["rpc"]}
        fn_skip = self.skip_fn_by_class.get(cname, set())
        out["validation"]["skip_fn_not_in_header"] += len(fn_skip - rpc_gap_names)
        out["validation"]["header_rpc_gap_not_in_skip"] += len(rpc_gap_names - fn_skip)
        for name, r in g["rpc"].items():
            if name in net_names:
                continue
            if lower(name) in anc_names:
                out["override_declared"].append(name)
            else:
                out["header_only_rpc"].append({"kind": "header_only_rpc", "name": name, "type": None,
                                               "category": "header_only",
                                               "reason": "declared with a net role in the header, not a net "
                                                         "function in the reflection",
                                                "keys": ["header-only %s.%s" % (cname, name)]})
        return out

    def _safe_type(self, prop):
        try:
            return self.cpp_type(prop)
        except UnknownType:
            return None


# ------------------------------------------------------------------ header cross-check (engine classes)

def validate_engine_headers(model, image_available):
    """Replicated-property names and C++ types: reflection-derived versus UE 4.25 headers, per engine class."""
    report = {}
    total = {"classes": 0, "header_props": 0, "name_match": 0, "type_match": 0, "reflection_props": 0}
    for cname, rel in ENGINE_HEADERS.items():
        path = os.path.join(ENGINE_SRC, rel)
        va = next((v for v, n in model.names.items()
                   if n == cname and model.parents.get(v, {}).get("package") == "/Script/Engine"), None)
        if va is None or not os.path.exists(path):
            report[cname] = {"status": "missing"}
            continue
        header = header_replicated(path, cname)
        refl = {}
        for prop in model.owned_replicated(va):
            try:
                refl[prop["name"]] = model.cpp_type(prop)
            except UnknownType as exc:
                refl[prop["name"]] = "UNKNOWN(%s)" % exc
        # Inherited replicated properties are checked on the class that declares them, so the reflection side is
        # the owned list only; the header side is the class's own declarations.
        h_names = [n for n, _ in header]
        name_hits = sum(1 for n in h_names if n in refl)
        type_hits = sum(1 for n, t in header if n in refl and refl[n] == t)
        mismatches = [{"name": n, "header": t, "reflection": refl.get(n)} for n, t in header
                      if n in refl and refl[n] != t]
        missing_in_refl = [n for n in h_names if n not in refl]
        extra_in_refl = sorted(set(refl) - set(h_names))
        report[cname] = {
            "status": "ok", "header_file": rel, "header_replicated": len(header),
            "reflection_owned_replicated": len(refl), "name_match": name_hits, "type_match": type_hits,
            "type_mismatches": mismatches, "header_only": missing_in_refl, "reflection_only": extra_in_refl,
        }
        total["classes"] += 1
        total["header_props"] += len(header)
        total["name_match"] += name_hits
        total["type_match"] += type_hits
        total["reflection_props"] += len(refl)
    total["name_accuracy"] = round(total["name_match"] / max(1, total["header_props"]), 4)
    total["type_accuracy"] = round(total["type_match"] / max(1, total["header_props"]), 4)
    return {"per_class": report, "total": total}


# ------------------------------------------------------------------ self-test

SYNTHETIC_HEADER = """
UCLASS(config=Game)
class FORTNITEGAME_API AFortSynthetic : public AFortBase
{
	GENERATED_BODY()
public:
	// table index 3, flags 0x20080100000020
	UPROPERTY(ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;
	// table index 2, flags 0x20080000000020
	UPROPERTY(Replicated)
	TArray<TSubclassOf<UObject>> Classes;
	// table index 1, flags 0x10000000000000
	UPROPERTY()
	uint8 bNotReplicated : 1;
	// table index 0, flags 0x20000000000020
	UPROPERTY(Replicated)
	bool bFlag : 1;
	UFUNCTION(Server, Reliable)
	void ServerDo(const FVector& Where);
	void ServerDo_Implementation(const FVector& Where);
	UFUNCTION()
	void Local();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPing();
	UFUNCTION(ServiceRequest(UnrecoveredEndpoint))
	void ServiceAsk();
	UFUNCTION(ServiceResponse(UnrecoveredEndpoint))
	void ServiceAnswer();
	UFUNCTION(Client, Reliable)
	void ClientTellConst() const;
};
"""


def self_test(model, image_path):
    checks = []

    def check(label, ok, detail=""):
        checks.append((label, bool(ok), detail))

    check("StrCrc32 of the handshake string equals the client's 683914758",
          str_crc32(lower(HANDSHAKE_STRING)) == HANDSHAKE_CHECKSUM,
          "got %d" % str_crc32(lower(HANDSHAKE_STRING)))
    check("CRC table entry 1 is the reflected CRC-32 table (0x77073096)", CRC_TABLE[1] == 0x77073096)
    check("StrCrc32 of empty string is 0", str_crc32("") == 0)

    # The C++ type rules on synthetic records (the strings UE 4.25 returns, including the trailing space).
    sample = [
        ({"kind": "Float", "kind_raw": "0x0a"}, "float"),
        ({"kind": "Bool", "kind_raw": "0x2c", "element_size": 1}, "bool"),
        ({"kind": "Bool", "kind_raw": "0x0c", "element_size": 1}, "uint8"),
        ({"kind": "Name", "kind_raw": "0x14"}, "FName"),
        ({"kind": "Array", "kind_raw": "0x16"}, "TArray"),
    ]
    for rec, expect in sample:
        try:
            got = model.cpp_type(rec)
        except UnknownType as exc:
            got = "UNKNOWN(%s)" % exc
        check("type rule %s %s -> %s" % (rec["kind"], rec["kind_raw"], expect), got == expect, "got %s" % got)
    check("header type rule: TSubclassOf keeps the trailing space",
          header_type("TSubclassOf<AActor>", False) == "TSubclassOf<AActor> ")
    check("header type rule: TArray collapses to TArray", header_type("TArray<AActor*>", False) == "TArray")
    check("header type rule: uint8 bitfield is uint8", header_type("uint8", True) == "uint8")

    # The header parser on a synthetic class (comments, ReplicatedUsing, bitfield, RPC roles, table flags).
    rep, rpcs = parse_class_header(SYNTHETIC_HEADER, "AFortSynthetic")
    check("header parser: replicated members are Ammo, Classes, bFlag (not the plain UPROPERTY)",
          [r["name"] for r in rep] == ["Ammo", "Classes", "bFlag"], str([r["name"] for r in rep]))
    check("header parser: table flags attach to their own UPROPERTY",
          [r["table_flags"] for r in rep] == [0x20080100000020, 0x20080000000020, 0x20000000000020])
    check("header parser: a bool bitfield is bool, a uint8 bitfield is uint8, TSubclassOf array is TArray",
          rep[2]["type"] == "bool" and rep[1]["type"] == "TArray" and header_type("uint8", True) == "uint8",
          str([r["type"] for r in rep]))
    check("header parser: RPCs are the Server, NetMulticast, service and const functions by their role",
          [(r["name"], r["roles"], r["reliable"]) for r in rpcs] ==
          [("ServerDo", ["Server"], True), ("MulticastPing", ["NetMulticast"], False),
           ("ServiceAsk", ["Request"], False), ("ServiceAnswer", ["Response"], False),
           ("ClientTellConst", ["Client", "Const"], True)], str(rpcs))

    # Bool setters: every bool record with a setter decodes to a byte offset inside the outer object.
    if os.path.exists(image_path):
        census = bool_census(model)
        check("bool setters decode to (offset, mask) in the runtime dump",
              census["total"] > 0 and census["decoded"] == census["total"],
              "%d of %d" % (census["decoded"], census["total"]))
        check("decoded bool offsets lie inside their outer object",
              census["offset_inside_outer"] == census["decoded"], "%d of %d" % (
                  census["offset_inside_outer"], census["decoded"]))
    else:
        check("bool setters (runtime dump not present, skipped)", True)

    # Header cross-check for the six engine classes.
    if os.path.isdir(ENGINE_SRC):
        val = validate_engine_headers(model, os.path.exists(image_path))
        t = val["total"]
        check("engine header replicated names match the reflection (AActor..AWorldSettings)",
              t["name_accuracy"] >= 0.95, "names %d/%d, types %d/%d" % (
                  t["name_match"], t["header_props"], t["type_match"], t["header_props"]))
        check("engine header C++ types match the reflection (known skew allowed)",
              t["type_accuracy"] >= 0.95, "types %d/%d" % (t["type_match"], t["header_props"]))
    else:
        check("engine headers (not present, skipped)", True)

    # The class set: every entry resolves to one block and has a generated header.
    check("class set: every entry resolves to exactly one reflection block",
          not model.unresolved_set, "unresolved %s" % model.unresolved_set[:5])
    no_header = [n for n, _ in model.set_entries if n not in model.headers]
    check("class set: every entry has a generated header under porting/native/Source/*/Generated",
          not no_header, "missing %d: %s" % (len(no_header), no_header[:5]))
    check("class set: the set is read whole (more than the 168 of v1)", len(model.set_entries) > 168,
          "%d entries" % len(model.set_entries))

    # Emitted versus the generator's own skip list, and no header-only members.
    set_items = list(model.set_vas.items())
    counter = collections.Counter()
    for va, _ in set_items:
        g = model.gaps(va)
        counter.update(g["validation"])
        counter["header_only"] += len(g["header_only_rep"]) + len(g["header_only_rpc"])
        counter["mismatch"] += len(g["mismatch"])
    check("headers declare no replicated property or RPC that the reflection lacks",
          counter["header_only"] == 0, "%d header-only" % counter["header_only"])
    check("generated headers agree with skipped_properties.json (replicated gaps, both directions)",
          counter["skip_row_not_in_header"] == 0 and counter["header_gap_not_in_skip"] == 0,
          "skip-only %d, header-only %d" % (counter["skip_row_not_in_header"], counter["header_gap_not_in_skip"]))
    check("generated headers agree with skipped_properties.json (RPC gaps, both directions)",
          counter["skip_fn_not_in_header"] == 0 and counter["header_rpc_gap_not_in_skip"] == 0,
          "skip-only %d, header-only %d" % (counter["skip_fn_not_in_header"], counter["header_rpc_gap_not_in_skip"]))
    # Known generator skew: a native bool (reflection GetCPPType "bool") is emitted as a uint8 bitfield. Any other
    # type or dim difference is a new failure.
    skew = [e for va, _ in set_items for e in model.gaps(va)["mismatch"]]
    known = [e for e in skew if e["kind"] == "type_mismatch" and e["type"] == "uint8"
             and e.get("reflection_type") == "bool"]
    check("header C++ types and dims equal the reflection, except the known skew (native bool as uint8 bitfield)",
          len(known) == len(skew), "%d known skew, %d other" % (len(known), len(skew) - len(known)))
    check("header table-index flags equal the reflection property flags", counter["table_flags_mismatch"] == 0,
          "%d mismatches" % counter["table_flags_mismatch"])
    check("header UFUNCTION roles equal the reflection net flags", counter["role_flag_mismatch"] == 0,
          "%d mismatches" % counter["role_flag_mismatch"])

    # Inheritance: a child's expected checksum folds its parent's value first.
    chains = 0
    ok_chain = True
    for va in list(model.blocks)[:4000]:
        parent = model.parent_va(va)
        if not parent:
            continue
        try:
            child = model.class_checksum(va, "expected")
            par = model.class_checksum(parent, "expected")
        except UnknownType:
            continue
        chains += 1
        own = model.owned_replicated(va)
        if not own and child != par:
            ok_chain = False
    check("a class with no owned replicated properties keeps its parent's ClassChecksum", ok_chain,
          "%d chains" % chains)

    # RPC groups grow along the chain (the NetFieldExportGroup walks every super class).
    grows = True
    for va, _ in set_items:
        parent = model.parent_va(va)
        if parent and not (collections.Counter(model.rpc_group(parent, "expected"))
                           - collections.Counter(model.rpc_group(va, "expected")) == collections.Counter()):
            grows = False
            break
    check("RPC group of a class contains every net function of its parents (expected)", grows)

    # The emitted subset never adds a property or an RPC.
    subset_ok = True
    for va, _ in set_items:
        if len(model.emitted_rep(va, "emitted")) > len(model.owned_replicated(va)):
            subset_ok = False
        if len(model.emitted_net_functions(va, "emitted")) > len(model.net_functions(va)):
            subset_ok = False
    check("emitted replicated and RPC subsets are never larger than the expected sets", subset_ok)

    failed = [c for c in checks if not c[1]]
    for label, ok, detail in checks:
        print("%-4s %s%s" % ("ok" if ok else "FAIL", label, (" (%s)" % detail) if detail else ""))
    print("self-test: %d checks, %d failed" % (len(checks), len(failed)))
    return 1 if failed else 0


# ------------------------------------------------------------------ main walk

def bool_census(model):
    """Bool setters decoded, and how many decoded offsets lie inside the outer object (SizeOfOuter)."""
    total = decoded = in_range = 0
    for block in model.blocks.values():
        for prop in block.get("properties") or []:
            if prop["kind"] == "Bool" and prop.get("set_bit_fn"):
                total += 1
                found = bool_setter_offset(model.image(), hexint(prop["set_bit_fn"]))
                if found is not None:
                    decoded += 1
                    if found[0] < prop["size_of_outer"]:
                        in_range += 1
    return {"total": total, "decoded": decoded, "offset_inside_outer": in_range}


def _entry(e):
    return {k: e[k] for k in ("kind", "name", "type", "category", "reason", "keys") if k in e}


def class_record(model, name, va, rnd):
    rec = {"class": name, "round": rnd, "block_va": va, "package": model.package(va),
           "depth": len(model.ancestors(va)), "parent": model.names.get(model.parent_va(va)) if model.parent_va(va) else None}
    g = model.generated(va)
    rec["module"] = g["module"] if g else None
    try:
        exp_p = model.class_checksum(va, "expected")
        emi_p = model.class_checksum(va, "emitted")
        exp_g = model.rpc_group(va, "expected")
        emi_g = model.rpc_group(va, "emitted")
        v1 = model.rpc_chain_v1(va, "expected") == model.rpc_chain_v1(va, "emitted")
        own = model.gaps(va)
    except UnknownType as exc:
        rec.update({"parity": None, "notes": [str(exc)], "cause": "unknown", "_active": []})
        return rec
    chain = [va] + model.ancestors(va)
    gen_chain = [c for c in chain if c in model.set_vas]
    prop_blockers, rpc_blockers = [], []
    hdr_prop = hdr_rpc = False
    anc_gap = False
    for c in gen_chain:
        gc = model.gaps(c)
        prop_blockers += gc["rep"] + gc["header_only_rep"] + gc["mismatch"]
        rpc_blockers += gc["rpc"] + gc["header_only_rpc"]
        hdr_prop = hdr_prop or bool(gc["header_only_rep"])
        hdr_rpc = hdr_rpc or bool(gc["header_only_rpc"])
        if c != va and (gc["rep"] or gc["rpc"] or gc["header_only_rep"] or gc["header_only_rpc"] or gc["mismatch"]):
            anc_gap = True
    prop_parity = exp_p == emi_p and not hdr_prop
    rpc_parity = exp_g == emi_g and not hdr_rpc
    parity = prop_parity and rpc_parity
    active = (prop_blockers if not prop_parity else []) + (rpc_blockers if not rpc_parity else [])
    own_any = bool(own["rep"] or own["rpc"] or own["header_only_rep"] or own["header_only_rpc"] or own["mismatch"])
    if parity:
        cause = "parity"
    elif own_any and anc_gap:
        cause = "own+ancestor"
    elif own_any:
        cause = "own"
    elif anc_gap:
        cause = "ancestor-only"
    else:
        cause = "unexplained"
    own_entries = [_entry(e) for e in own["rep"] + own["rpc"] + own["header_only_rep"] + own["header_only_rpc"]
                   + own["mismatch"]]
    keys = sorted({k for e in active for k in e["keys"]})
    rec.update({
        "parity": parity, "parity_props": prop_parity, "parity_rpc": rpc_parity, "cause": cause,
        "expected": {"class_checksum": exp_p, "class_checksum_hex": "0x%08X" % exp_p,
                     "rpc_set_size": len(exp_g), "replicated_owned": len(model.owned_replicated(va)),
                     "rpcs_net": len(model.net_functions(va))},
        "emitted": {"class_checksum": emi_p, "class_checksum_hex": "0x%08X" % emi_p,
                    "rpc_set_size": len(emi_g), "replicated_owned": len(model.emitted_rep(va, "emitted")),
                    "rpcs_net": len(model.emitted_net_functions(va, "emitted"))},
        "own_gaps": own_entries,
        "own_override_declared": own["override_declared"],
        "chain_gap_classes": [model.names[c] for c in gen_chain[1:]
                              if model.gaps(c)["rep"] or model.gaps(c)["rpc"] or model.gaps(c)["mismatch"]],
        "blocker_keys": keys,
        "non_set_ancestors": [model.names.get(a) for a in chain[1:] if a not in model.set_vas
                              and model.package(a) not in ENGINE_PACKAGES],
        "rpc_chain_v1_parity": v1,
        "notes": [],
    })
    rec["_active"] = active
    rec["_prop_fail"] = not prop_parity
    rec["_rpc_fail"] = not rpc_parity
    rec["_anc_roots"] = [model.names[a] for a in gen_chain[1:] if model.gaps(a)["rep"] or model.gaps(a)["rpc"]
                         or model.gaps(a)["mismatch"] or model.gaps(a)["header_only_rep"]]
    return rec


def _fixable(keys):
    return all(not k.startswith("UNEXPLAINED") and k not in ("UNEXPLAINED",) for k in keys)


def analyse(model, records):
    """Aggregates: parity totals, causes, reasons, fix worklist, root ancestors."""
    def tally(recs):
        known = [r for r in recs if r.get("parity") is not None]
        return {"classes": len(recs), "parity_yes": sum(1 for r in known if r["parity"]),
                "parity_no": sum(1 for r in known if not r["parity"]),
                "parity_unknown": len(recs) - len(known),
                "props_yes": sum(1 for r in known if r["parity_props"]),
                "rpc_yes": sum(1 for r in known if r["parity_rpc"]),
                "ancestor_only": sum(1 for r in known if r["cause"] == "ancestor-only")}

    by_module = collections.OrderedDict()
    for mod in sorted({r["module"] or "(none)" for r in records}):
        by_module[mod] = tally([r for r in records if (r["module"] or "(none)") == mod])
    by_depth = collections.OrderedDict()
    for d in sorted({r["depth"] for r in records}):
        by_depth[str(d)] = tally([r for r in records if r["depth"] == d])
    by_round = collections.OrderedDict()
    for rnd in sorted({r["round"] for r in records}):
        by_round[str(rnd)] = tally([r for r in records if r["round"] == rnd])

    causes = collections.Counter(r["cause"] for r in records)
    nonpar = [r for r in records if r.get("parity") is False]

    cat_touch, cat_solo, cat_gaps = collections.Counter(), collections.Counter(), collections.Counter()
    key_touch = collections.Counter()
    for r in nonpar:
        cats = {e["category"] for e in r["_active"]}
        for c in cats:
            cat_touch[c] += 1
            if cats == {c}:
                cat_solo[c] += 1
        for e in r["_active"]:
            cat_gaps[e["category"]] += 1
    for r in nonpar:
        for k in r["blocker_keys"]:
            key_touch[k] += 1

    # Greedy worklist over fixable classes: each step adds the fix that completes the most remaining classes.
    todo = {}
    unfixable = 0
    for r in nonpar:
        keys = frozenset(r["blocker_keys"])
        if not keys or not _fixable(keys) or r["cause"] == "unexplained":
            unfixable += 1
            continue
        todo[r["class"]] = keys
    fixed = set()
    worklist = []
    remaining = dict(todo)
    while remaining and len(worklist) < 25:
        candidates = set().union(*remaining.values())
        best = None
        for k in candidates:
            trial = fixed | {k}
            done = [c for c, ks in remaining.items() if ks <= trial]
            touched = sum(1 for ks in remaining.values() if k in ks)
            score = (len(done), touched)
            if best is None or score > best[0]:
                best = (score, k, done)
        score, k, done = best
        fixed.add(k)
        for c in done:
            remaining.pop(c, None)
        worklist.append({"fix": k, "classes_completed": score[0], "classes_touched": score[1],
                         "cumulative_flipped": len(todo) - len(remaining)})
        if score[0] == 0 and score[1] == 0:
            break
    roots = collections.Counter()
    for r in nonpar:
        for a in r["_anc_roots"]:
            roots[a] += 0
        for a in r["_anc_roots"]:
            roots[a] += 1
    summary = {
        "classes": len(records), "all": tally(records), "round": by_round,
        "by_module": by_module, "by_depth": by_depth, "causes": dict(causes),
        "reasons": [{"category": c, "classes_touched": n, "classes_solo": cat_solo[c], "gaps": cat_gaps[c]}
                    for c, n in cat_touch.most_common()],
        "top_keys": [{"key": k, "classes_touched": n,
                      "classes_solo": sum(1 for r in nonpar if r["blocker_keys"] == [k])}
                     for k, n in key_touch.most_common(40)],
        "worklist": worklist, "worklist_classes_total": len(todo), "unfixable_nonparity": unfixable,
        "worklist_still_open": len(remaining),
        "root_ancestors": [{"class": a, "descendant_failures": n} for a, n in roots.most_common(10)],
    }
    return summary


def markdown_block(report):
    s = report["summary"]
    v = report["validation"]
    t = v["engine_headers"]["total"]
    r1 = s["round"].get("1", {}).get("classes", 0)
    lines = [MD_BEGIN, "",
             "## Offline class checksum parity (all generated classes)", "",
             "Generated by `porting/class_checksum.py` from the recovered reflection tables and the generated headers "
             "under `porting/native/Source/*/Generated` (the emitted side). Method: the module docstring. Per-class "
             "detail: `porting/generated/reflection/class_checksums.json` (git-ignored). Set: `native_class_set.txt`, "
             "%d entries (round 1: %d, later rounds: %d)." % (len(report["classes"]), r1, len(report["classes"]) - r1),
             "",
             "Validation: StrCrc32 of the handshake string = %d (client log). Engine headers: %d/%d replicated names, "
             "%d/%d types. Bool setters decoded: %d/%d. Header vs `skipped_properties.json`: %d replicated and %d RPC "
             "gaps differ. Header types or dims differing from the reflection: %d. Header-only members: %d." % (
                 HANDSHAKE_CHECKSUM, t["name_match"], t["header_props"], t["type_match"], t["header_props"],
                 report["bool_setters"]["decoded"], report["bool_setters"]["total"],
                 v["gap_crosscheck"]["replicated_both_ways"], v["gap_crosscheck"]["rpc_both_ways"],
                 v["gap_crosscheck"]["type_mismatch"], v["gap_crosscheck"]["header_only"]),
             "Type mismatches: %s, in %d classes (a native `bool` is emitted as a `uint8` bitfield; the emitted "
             "checksum uses the header's type, assuming UHT records that type). Skip tokens that name no block: %d (listed in the "
             "JSON). Set entries that do not resolve: %d." % (
                 ", ".join("%s: %d" % (k, n) for k, n in v["gap_crosscheck"]["type_mismatch_pairs"].items()) or "none",
                 v["gap_crosscheck"]["type_mismatch_classes"], len(v["unresolved_skip_tokens"]),
                 len(v["unresolved_set_entries"])), "",
             "### Totals", "",
             "| Classes | Parity yes | Parity no | Unknown | Class checksum yes | RPC set yes | Ancestor-only no |",
             "|---|---|---|---|---|---|---|",
             "| %d | %d | %d | %d | %d | %d | %d |" % (
                 s["all"]["classes"], s["all"]["parity_yes"], s["all"]["parity_no"], s["all"]["parity_unknown"],
                 s["all"]["props_yes"], s["all"]["rpc_yes"], s["all"]["ancestor_only"]), "",
             "Parity = ClassChecksum equal and the RPC set equal (UE NetFieldExportGroup semantics, see the docstring). "
             "Ancestor-only = no own gap, but a generated ancestor's gap changes the class.", "",
             "### By module", "",
             "| Module | Classes | Yes | No | Unknown | Ancestor-only |", "|---|---|---|---|---|---|"]
    for mod, x in s["by_module"].items():
        lines.append("| %s | %d | %d | %d | %d | %d |" % (mod, x["classes"], x["parity_yes"], x["parity_no"],
                                                          x["parity_unknown"], x["ancestor_only"]))
    lines += ["", "### By depth (number of ancestors in the reflection)", "",
              "| Depth | Classes | Yes | No | Ancestor-only |", "|---|---|---|---|---|"]
    for d, x in s["by_depth"].items():
        lines.append("| %s | %d | %d | %d | %d |" % (d, x["classes"], x["parity_yes"], x["parity_no"], x["ancestor_only"]))
    lines += ["", "### Cause of each non-parity class", "", "| Cause | Classes |", "|---|---|"]
    for c in ("own", "own+ancestor", "ancestor-only", "unexplained", "unknown"):
        if c in s["causes"]:
            lines.append("| %s | %d |" % (c, s["causes"][c]))
    lines += ["", "### Top skip reasons (classes made non-parity)", "",
              "A class counts under a reason when one of its blocking gaps (own, or in a generated ancestor) has that "
              "reason. Solo = the reason is the only one blocking the class.", "",
              "| Reason | Classes touched | Solo | Gaps |", "|---|---|---|---|"]
    for r in s["reasons"][:12]:
        lines.append("| %s | %d | %d | %d |" % (r["category"], r["classes_touched"], r["classes_solo"], r["gaps"]))
    lines += ["", "### Worklist: fixes that flip the most classes to parity (greedy)", "",
              "Each step adds the fix that completes the most remaining classes (all of a class's blocking keys must "
              "be fixed). %d non-parity classes are fixable this way; %d are not (unexplained or ungenerated "
              "ancestors). After the %d steps below, %d classes stay open." % (
                  s["worklist_classes_total"], s["unfixable_nonparity"], len(s["worklist"]),
                  s["worklist_still_open"]), "",
              "| # | Fix | Classes completed now | Classes touched | Cumulative flipped |", "|---|---|---|---|---|"]
    for i, w in enumerate(s["worklist"], 1):
        lines.append("| %d | %s | %d | %d | %d |" % (i, w["fix"], w["classes_completed"], w["classes_touched"],
                                                    w["cumulative_flipped"]))
    lines += ["", "### Generated ancestors that fail the most descendants", "",
              "| Ancestor | Descendant failures |", "|---|---|"]
    for a in s["root_ancestors"][:8]:
        lines.append("| %s | %d |" % (a["class"], a["descendant_failures"]))
    lines += ["", "### RPC digest: what changed from v1", "",
              "v1 chained the RPC digest through the super class (`rpc_chain_v1`). CoreNet.cpp does not: each function "
              "checksum is seeded with 0, and the export group matches functions by checksum over the class chain. "
              "The change is in the definition. Parity flags are unchanged: v1 and the set comparison disagree on %d "
              "classes (v1 fails and the set passes: %d; the reverse: %d)." % (
                  v["rpc_v1"]["v1_only_fail"] + v["rpc_v1"]["set_only_fail"], v["rpc_v1"]["v1_only_fail"],
                  v["rpc_v1"]["set_only_fail"]), "",
             "### Assumptions", ""]
    for a in report["assumptions"]:
        lines.append("- " + a)
    lines += ["", MD_END, ""]
    return "\n".join(lines)


def write_markdown(path, block):
    text = ""
    if os.path.exists(path):
        with open(path, encoding="utf-8") as fh:
            text = fh.read()
    if MD_BEGIN in text and MD_END in text:
        head = text[:text.index(MD_BEGIN)]
        tail = text[text.index(MD_END) + len(MD_END):].lstrip("\n")
        new = head + block + ("\n" + tail if tail else "")
    else:
        new = (text.rstrip("\n") + "\n\n" if text.strip() else "# Class checksum report\n\n") + block
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(new)


def run(args):
    model = Model(REFL, args.image, set_path=args.set_path, source_root=args.source_root)
    va_by_name = {name: va for va, (name, _) in model.set_vas.items()}
    records = [class_record(model, name, va_by_name[name], rnd)
               for name, rnd in model.set_entries if name in va_by_name]
    for name in model.unresolved_set:
        records.append({"class": name, "round": None, "block_va": None, "parity": None, "cause": "unknown",
                        "notes": ["set entry does not map to exactly one reflection block"], "module": None,
                        "depth": None, "_active": []})
    summary = analyse(model, records)
    summary["classes"] = len(records)
    v1_only = sum(1 for r in records if r.get("rpc_chain_v1_parity") is False and r.get("parity_rpc") is True)
    set_only = sum(1 for r in records if r.get("rpc_chain_v1_parity") is True and r.get("parity_rpc") is False)
    gap_sum = collections.Counter()
    for va in model.set_vas:
        gap_sum.update(model.gaps(va)["validation"])
        gap_sum["type_mismatch"] += len(model.gaps(va)["mismatch"])
        gap_sum["header_only"] += len(model.gaps(va)["header_only_rep"]) + len(model.gaps(va)["header_only_rpc"])
    cross = {
        "replicated_both_ways": gap_sum["skip_row_not_in_header"] + gap_sum["header_gap_not_in_skip"],
        "rpc_both_ways": gap_sum["skip_fn_not_in_header"] + gap_sum["header_rpc_gap_not_in_skip"],
        "type_mismatch": gap_sum["type_mismatch"],
        "header_only": gap_sum["header_only"],
        "struct_header_missing": gap_sum["struct_header_missing"],
        "type_mismatch_pairs": dict(collections.Counter(
            "%s (header) vs %s (reflection)" % (e["type"], e.get("reflection_type"))
            for va in model.set_vas for e in model.gaps(va)["mismatch"])),
        "type_mismatch_classes": sum(1 for va in model.set_vas if model.gaps(va)["mismatch"]),
        "table_flags_mismatch": gap_sum["table_flags_mismatch"],
        "role_flag_mismatch": gap_sum["role_flag_mismatch"],
    }
    eh = validate_engine_headers(model, os.path.exists(args.image))
    report = {
        "method": "UE 4.25 FClassNetCacheMgr (CoreNet.cpp) and UClass::SetUpRuntimeReplicationData (Class.cpp). "
                  "ClassChecksum: owned replicated properties sorted by (offset, name), chained from the super class. "
                  "RPC set: per-function checksums (name, flags; seed 0) over the class and its super classes, "
                  "as the NetFieldExportGroup matches them.",
        "inputs": {"reflection": "fortnite_game_reflection.json", "skipped": "skipped_properties.json",
                   "class_set": os.path.relpath(args.set_path, HERE), "emitted": "porting/native/Source/*/Generated/*.h",
                   "image": os.path.basename(args.image)},
        "assumptions": [
            "Emitted side = the generated header of each set class: replicated UPROPERTY names, the header's C++ type "
            "and array dimension; UFUNCTION(Server|Client|NetMulticast|ServiceRequest|ServiceResponse) names. "
            "Offsets come from the reflection.",
            "Emitted UFUNCTION flags are the reflection's (the header's role specifiers are cross-checked against "
            "the net bits; table-index flags of properties are cross-checked too).",
            "Engine and non-set ancestors are not generated: their expected value is their emitted value.",
            "Struct layouts are the reflection's (the emitted F<Struct>.h is checked for existence only).",
            "RPC parity uses per-function checksums over the class chain (NetFieldExportGroup); the v1 chained digest is "
            "not used for parity. Blueprint-implementable events are not in the reflection's net functions "
            "(no FUNC_BlueprintEvent net functions), so no BlueprintEvent case arises.",
            "Ancestor function names (for NetFields' override exclusion) come from the reflection's per-class "
            "function lists; an engine ancestor whose function list is incomplete would change the exclusion.",
        ],
        "bool_setters": bool_census(model),
        "validation": {"handshake_crc": {"string": HANDSHAKE_STRING, "expected": HANDSHAKE_CHECKSUM,
                                         "computed": str_crc32(lower(HANDSHAKE_STRING))},
                       "engine_headers": eh, "gap_crosscheck": cross,
                       "rpc_v1": {"v1_only_fail": v1_only, "set_only_fail": set_only},
                       "unresolved_set_entries": model.unresolved_set,
                       "unresolved_skip_tokens": sorted(model.unresolved_tokens),
                       "bool_setter_failures": len(model.bool_failures)},
        "summary": summary,
        "classes": [{k: v for k, v in r.items() if not k.startswith("_")} for r in records],
    }
    with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(report, fh, indent=1)
        fh.write("\n")
    s = summary
    print("classes %d: parity yes %d, no %d, unknown %d (props yes %d, rpc yes %d)" % (
        s["all"]["classes"], s["all"]["parity_yes"], s["all"]["parity_no"], s["all"]["parity_unknown"],
        s["all"]["props_yes"], s["all"]["rpc_yes"]))
    print("causes %s" % json.dumps(s["causes"], sort_keys=True))
    print("by module %s" % json.dumps({k: [v["parity_yes"], v["classes"]] for k, v in s["by_module"].items()}))
    print("top reasons %s" % json.dumps([(r["category"], r["classes_touched"]) for r in s["reasons"][:6]]))
    print("worklist %s" % json.dumps([(w["fix"], w["classes_completed"]) for w in s["worklist"][:8]]))
    if not args.no_md:
        assumptions = report["assumptions"]
        write_markdown(args.report_md, markdown_block(dict(report, classes=records, assumptions=assumptions)))
        print("updated %s" % args.report_md, file=sys.stderr)
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--image", default=IMAGE, help="runtime dump (bool setter code)")
    ap.add_argument("--out", default=OUT)
    ap.add_argument("--report-md", default=REPORT_MD)
    ap.add_argument("--set", dest="set_path", default=CLASS_SET, help="class set file (native_class_set.txt)")
    ap.add_argument("--source-root", default=SOURCE_ROOT, help="directory holding <Module>/Generated/*.h")
    ap.add_argument("--no-md", action="store_true", help="do not write the markdown block")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)
    if args.self_test:
        model = Model(REFL, args.image, set_path=args.set_path, source_root=args.source_root)
        return self_test(model, args.image)
    return run(args)


if __name__ == "__main__":
    sys.exit(main())
