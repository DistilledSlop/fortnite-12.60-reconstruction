#!/usr/bin/env python3
"""Generate UE 4.25 C++ class scaffolds from porting/extract_reflection.py output (stdlib only).

Input:
  --reflection JSON   output of extract_reflection.py (default: porting/generated/reflection/...)
  --name-map JSON     {block_va: "ClassName"} (block VA as written by the extractor, e.g. "0x146b59ff0")
  --type-map JSON     {fn_va: "TypeName"} for Z_Construct_* function pointers (class, struct, enum and
                      parent references). Without it, every such reference is emitted as a TODO_* token
                      that fails to compile on purpose.
  --parent-map JSON   output of porting/resolve_class_parents.py ({block_va: {parent_name, parent_va, ...}}).
                      When given, it is the only source of the parent (Super). Without it, the parent comes
                      from base_fn through --type-map, which is dependency_fns[1] (the package), not the parent.

Output, for every block that has a name, under porting/generated/native_classes/ (git-ignored):
  <Name>.h    UCLASS, UPROPERTY, UFUNCTION declarations
  <Name>.cpp  GetLifetimeReplicatedProps with DOREPLIFETIME
  index.json  per-class summary (parent status, counts, unresolved TODO tokens)

Property order: the exe's FPropertyParams table lists members in the reverse of their declaration
order (checked against engine AActor). Declarations are emitted in declaration order (the reverse
of the table) so that UHT, which reverses declaration order into its table, reproduces the original
table. Use --order table for the literal table order. DOREPLIFETIME lines are always in table order.

Validation: --compare-header PATH --compare-class NAME diffs the generated class against a UE header
(property names, normalized C++ types, order, replication, UFUNCTION names and net roles).

Usage:
  python3 -I porting/gen_native_classes.py [--reflection JSON] [--name-map JSON] [--type-map JSON]
          [--parent-map JSON] [--out DIR] [--api MACRO] [--order declaration|table]
          [--compare-header HDR --compare-class NAME [--report PATH]]
  python3 -I porting/gen_native_classes.py --self-test
"""
import argparse
import json
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_REFLECTION = os.path.join(REPO, "porting", "generated", "reflection", "fortnite_game_reflection.json")
DEFAULT_OUT = os.path.join(REPO, "porting", "generated", "native_classes")
# Replication conditions per class and property, recovered from the GetLifetimeReplicatedProps registrations (see
# porting/decompilation_notes.md, "Sixth pass"). Read by --promote only; loaded into REPLICATION_CONDITIONS.
CONDITIONS_DEFAULT = os.path.join(REPO, "porting", "replication_conditions.json")
REPLICATION_CONDITIONS = {}

# Parameter order of functions and delegate signatures. The reflection table lists parameters in reverse declaration
# order (UHT emits its PropPointers from the last parameter back) and the recorded offsets ascend in declaration order
# (STRUCT_OFFSET of the Parms struct). "offset" (default) spells the parameters in declaration order, so the parameter
# block lays out at the recorded offsets. "table" is the legacy order (the table as listed) and reproduces the output
# of earlier runs. FN_PARAM_ORDER or --param-order selects it.
PARAM_ORDER_CHOICES = ("offset", "table")
PARAM_ORDER = os.environ.get("FN_PARAM_ORDER", "offset")
# Functions whose recorded offsets are not in reverse table order are kept in table order and listed here.
PARAM_ORDER_BREAKS = []

# EPropertyFlags (UObjectMacros/ObjectMacros.h, UE 4.25)
CPF_NET = 0x20
GEN_NATIVE_BOOL = 0x20   # EPropertyGenFlags::NativeBool (kind_raw of a Bool property)
CPF_CONST_PARM = 0x2
CPF_OUT_PARM = 0x100
CPF_RETURN_PARM = 0x400
CPF_TRANSIENT = 0x2000
CPF_CONFIG = 0x4000
CPF_SAVE_GAME = 0x1000000
CPF_REFERENCE_PARM = 0x8000000
CPF_INTERP = 0x200000000
# EFunctionFlags (Script.h, UE 4.25)
FUNC_NET = 0x40
FUNC_NET_RELIABLE = 0x80
FUNC_NET_REQUEST = 0x100    # UE net service request (ServiceRequest); no role bit
FUNC_NET_RESPONSE = 0x1000  # UE net service response (ServiceResponse); no role bit
FUNC_EXEC = 0x200
FUNC_STATIC = 0x2000
FUNC_NET_MULTICAST = 0x4000
FUNC_NET_SERVER = 0x200000
FUNC_NET_CLIENT = 0x1000000
FUNC_CONST = 0x40000000
# EClassFlags (ObjectMacros.h, UE 4.25)
CLASS_ABSTRACT = 0x1
CLASS_DEFAULT_CONFIG = 0x2
CLASS_CONFIG = 0x4
CLASS_TRANSIENT = 0x8
CLASS_MINIMAL_API = 0x80000
CLASS_DEPRECATED = 0x2000000
CLASS_INTERFACE = 0x4000  # UINTERFACE: no UCLASS can be emitted for it (see native_class_blocklist.txt)

SCALAR_TYPES = {
    "Int8": "int8", "Int16": "int16", "Int": "int32", "Int64": "int64",
    "UInt16": "uint16", "UInt32": "uint32", "UInt64": "uint64",
    "UnsizedInt": "int32", "UnsizedUInt": "uint32",
    "Float": "float", "Double": "double", "Bool": "bool",
    "Name": "FName", "Str": "FString", "Text": "FText",
}
DELEGATE_KINDS = ("Delegate", "InlineMulticastDelegate", "SparseMulticastDelegate")


def hx(value):
    """Parse '0x..' strings or ints from the reflection JSON."""
    if value is None:
        return 0
    return int(value, 16) if isinstance(value, str) else int(value)


def normalize_type(text):
    """Comparable form of a C++ type: no class/struct/enum keywords, no whitespace."""
    text = re.sub(r"\b(class|struct|enum)\s+", "", text.strip())
    return re.sub(r"\s+", "", text)


def lower_keys(mapping):
    return {str(k).lower(): v for k, v in (mapping or {}).items()}


class Resolver:
    """Maps reflection pointers to C++ spellings. Unresolved references become TODO_* tokens."""

    def __init__(self, type_map, known_fns=None, iface_map=None):
        self.types = lower_keys(type_map)
        # Construct functions of every named class, including those outside the native set.
        self.known = {str(k).lower() for k in (known_fns or [])}
        # Interface UClass construct function -> C++ interface name (I<Name>), for TScriptInterface<I...>.
        self.ifaces = {str(k).lower(): v for k, v in (iface_map or {}).items()}
        self.todos = []
        self.tokens = []        # every TODO token issued, in order (promote mode slices this per item)
        self.class_refs = set()  # class names referenced by emitted pointer/class types (forward declarations)
        self.skipped = []        # promote mode: skip records for the current class
        self.gate = None         # promote mode: TypeGate (struct/enum/delegate usability), None in draft mode
        self.wire_used = False   # a wire-custom struct was used since the last reset (see TypeGate)
        self.used = {}           # struct/enum/delegate fn -> set of entry kinds, since the last reset

    def fork(self):
        """A resolver with the same type map and gate and empty per-item state (used by TypeGate checks)."""
        child = Resolver.__new__(Resolver)
        child.types = self.types
        child.known = self.known
        child.ifaces = self.ifaces
        child.todos = []
        child.tokens = []
        child.class_refs = set()
        child.skipped = []
        child.gate = self.gate
        child.wire_used = False
        child.used = {}
        return child

    def name(self, fn):
        return self.types.get(fn.lower()) if fn else None

    def generated_struct(self, entry):
        """True when a struct entry names a struct generated in this module (promote mode)."""
        if getattr(self, "gate", None) is None:
            return False
        info = self.gate.info((entry.get("type_refs") or {}).get("struct"))
        return info is not None and info.package in PROMOTE_PACKAGES

    def todo(self, what, ref):
        token = "TODO_%s_%s" % (what, (ref or "none").replace("0x", ""))
        self.todos.append("%s not resolved (%s)" % (what, ref))
        self.tokens.append(token)
        return token

    def todo_named(self, what, name):
        token = "TODO_%s_%s" % (what, re.sub(r"\W", "_", name or "unnamed"))
        self.todos.append("%s type not decoded (%s)" % (what, name))
        self.tokens.append(token)
        return token

    def type_ref(self, kind, fn, what, entry_kind=None):
        """C++ name of a struct, enum or delegate type. Promote mode asks the TypeGate; a type that cannot be
        emitted or included returns a TODO token whose prefix is the skip category (see category_of)."""
        name = self.name(fn)
        if name is None:
            return self.todo(what, fn)
        if self.gate is None:
            return name
        return self.gate.check(self, kind, fn, name, entry_kind)

    def class_name(self, fn, what):
        name = self.name(fn)
        if name:
            self.class_refs.add(name)
            return name
        if fn and fn.lower() in self.known:
            # A named class that is not in the native set: its path cannot be declared in this module.
            return self.todo("ClassOutsideNativeSet", fn)
        return self.todo(what, fn)

    def ctype(self, entry, children):
        kind = entry.get("kind")
        refs = entry.get("type_refs", {})
        if kind in SCALAR_TYPES:
            return SCALAR_TYPES[kind]
        if kind == "Byte":
            if "enum" in refs:
                return "TEnumAsByte<%s>" % self.type_ref("enum", refs["enum"], "Enum", kind)
            return "uint8"
        if kind == "Enum":
            return self.type_ref("enum", refs.get("enum"), "Enum", kind)
        if kind == "Struct":
            return self.type_ref("struct", refs.get("struct"), "Struct", kind)
        if kind in ("Object", "WeakObject", "LazyObject", "SoftObject"):
            target = self.class_name(refs.get("class"), "Class")
            wrap = {"Object": "%s*", "WeakObject": "TWeakObjectPtr<%s>",
                    "LazyObject": "TLazyObjectPtr<%s>", "SoftObject": "TSoftObjectPtr<%s>"}[kind]
            return wrap % target
        if kind == "Class":
            target = self.class_name(refs.get("meta_class"), "MetaClass")
            return "TSubclassOf<%s>" % target
        if kind == "SoftClass":
            target = self.class_name(refs.get("meta_class"), "MetaClass")
            return "TSoftClassPtr<%s>" % target
        if kind == "Interface":
            fn = refs.get("interface_class")
            target = self.ifaces.get(fn.lower()) if fn else None
            if target is None:
                return self.todo("Interface", fn)
            self.class_refs.add(target)
            return "TScriptInterface<%s>" % target
        if kind == "Array":
            elem = children.get("elem")
            inner = self.ctype(elem, {}) if elem else self.todo_named("ArrayElem", entry.get("name"))
            return "TArray<%s>" % inner
        if kind == "Set":
            elem = children.get("elem")
            if elem and elem.get("kind") == "Struct" and self.generated_struct(elem):
                # TSet hashes its elements with GetTypeHash, which a generated struct does not have (no body is recovered).
                return self.todo_named("SetElem", entry.get("name"))
            inner = self.ctype(elem, {}) if elem else self.todo_named("SetElem", entry.get("name"))
            return "TSet<%s>" % inner
        if kind == "Map":
            key, value = children.get("key"), children.get("value")
            if key and key.get("kind") == "Struct" and self.generated_struct(key):
                # TMap hashes its key with GetTypeHash, which a generated struct does not have (no body is recovered).
                return self.todo_named("MapKey", entry.get("name"))
            k = self.ctype(key, {}) if key else self.todo_named("MapKey", entry.get("name"))
            v = self.ctype(value, {}) if value else self.todo_named("MapValue", entry.get("name"))
            return "TMap<%s, %s>" % (k, v)
        if kind in DELEGATE_KINDS:
            # The delegate type is named by its signature function (FFunctionParams with FUNC_Delegate).
            sig = refs.get("signature")
            if sig:
                return self.type_ref("delegate", sig, "Delegate", kind)
            return self.todo_named("Delegate", entry.get("name"))
        return self.todo_named("Kind%s" % kind, entry.get("name"))


def enum_underlying_record(rec, enum_name):
    """True when rec is the underlying-type record of the enum property enum_name. The extractor marks a Byte one
    (enum_underlying_of); an Int one (an enum with a 32-bit underlying type) is the same shape: named UnderlyingType,
    directly after the Enum entry, with no flags. It is not a member, so it must not be emitted as one or as a
    parameter (before this check, ServerRemoveMapMarker and a few members carried a bogus int32 UnderlyingType)."""
    if rec.get("enum_underlying_of") == enum_name:
        return True
    return (rec.get("name") == "UnderlyingType" and rec.get("kind") in ("Int", "Byte")
            and hx(rec.get("property_flags")) == 0 and not rec.get("replicated"))


def split_properties(props):
    """Pair each top-level table entry with its inner records (array element, map key/value, set element).

    Inner records are not class members; they are removed from the member list here. Returns a list of
    (entry, children) tuples in table order, plus a list of notes for records that could not be paired."""
    result = []
    notes = []
    i = 0
    while i < len(props):
        p = props[i]
        if "error" in p:
            notes.append("unreadable property record at index %s" % p.get("index"))
            i += 1
            continue
        if p.get("array_inner_of") or p.get("enum_underlying_of"):
            notes.append("inner record %r without its container" % p.get("name"))
            i += 1
            continue
        children = {}
        step = 1
        kind = p.get("kind")
        nxt = props[i + 1] if i + 1 < len(props) else None
        if kind == "Array" and nxt and nxt.get("array_inner_of") == p.get("name"):
            children["elem"] = nxt
            step = 2
        elif kind == "Set" and nxt and nxt.get("name") == p.get("name") and not nxt.get("replicated"):
            children["elem"] = nxt
            step = 2
        elif kind == "Map":
            after = props[i + 2] if i + 2 < len(props) else None
            if nxt and nxt.get("name") == p.get("name") + "_Key" and after and after.get("name") == p.get("name"):
                children["key"] = nxt
                children["value"] = after
                step = 3
        elif kind == "Enum" and nxt and enum_underlying_record(nxt, p.get("name")):
            step = 2
        if kind in ("Array", "Set", "Map") and step == 1:
            notes.append("%s %s has no inner records; emitted with TODO element type" % (kind, p.get("name")))
        result.append((p, children))
        i += step
    return result, notes


def property_specifiers(entry):
    flags = hx(entry.get("property_flags"))
    specs = []
    if flags & CPF_NET:
        if entry.get("rep_notify"):
            specs.append("ReplicatedUsing=%s" % entry["rep_notify"])
        else:
            specs.append("Replicated")
    if flags & CPF_TRANSIENT:
        specs.append("Transient")
    if flags & CPF_CONFIG:
        specs.append("Config")
    if flags & CPF_SAVE_GAME:
        specs.append("SaveGame")
    if flags & CPF_INTERP:
        specs.append("Interp")
    return specs


def build_members(props, resolver, promote=False, struct_mode=False):
    """struct_mode: members of a USTRUCT. A wire-custom member type does not skip a struct member, because a
    struct's NetSerialize is the one that changes the wire, and the struct itself is then skipped where it is used."""
    members = []
    seen = set()
    pairs, notes = split_properties(props)
    for index, (entry, children) in enumerate(pairs):
        mark = len(resolver.tokens)
        resolver.wire_used = False
        resolver.used = {}
        ctype = resolver.ctype(entry, children)
        new_tokens = resolver.tokens[mark:]
        wire = resolver.wire_used
        used = resolver.used
        # A native bool (EPropertyGenFlags::NativeBool, 0x20 in kind_raw) is a `bool` member. Only the other bools are
        # bitfields. SetBitFunc is set for both kinds, so it does not tell them apart.
        bitfield = entry.get("kind") == "Bool" and not (hx(entry.get("kind_raw")) & GEN_NATIVE_BOOL)
        dim = int(entry.get("array_dim") or 1)
        name = entry.get("name") or "UnnamedProperty%d" % index
        if promote:
            # Promote mode emits only properties whose every type resolves to a declared native class.
            if new_tokens:
                resolver.skipped.append(skip_record("property", entry, name, category_of(new_tokens),
                                                    "unresolved type: " + ", ".join(sorted(set(new_tokens))), new_tokens))
                continue
            # A replicated property of a struct with a custom NetSerialize is emitted (wire_custom is recorded on the
            # member): an FFastArraySerializer struct gets its native NetDeltaSerialize, any other one a declared
            # fallback (see render_struct_header and porting/decompilation_notes.md, "Sixth pass").
            if name in seen:
                resolver.skipped.append(skip_record("property", entry, name, "duplicate_name",
                                                    "duplicate property name in the table", []))
                continue
            seen.add(name)
        if bitfield:
            decl = "uint8 %s : 1;" % name
        elif entry.get("kind") == "Bool":
            decl = "bool %s;" % name
        elif dim > 1:
            decl = "%s %s[%d];" % (ctype, name, dim)
        else:
            decl = "%s %s;" % (ctype, name)
        flags = hx(entry.get("property_flags"))
        members.append({
            "name": name,
            "kind": entry.get("kind"),
            "ctype": ctype,
            "decl": decl,
            "bitfield": bitfield,
            "table_index": entry.get("index", index),
            "property_flags": entry.get("property_flags"),
            "replicated": bool(flags & CPF_NET),
            "wire_custom": bool(wire),
            "rep_notify": entry.get("rep_notify"),
            "specifiers": property_specifiers(entry),
            "used": used,
            "offset": entry.get("offset"),
        })
    return members, notes


def decode_params(params, resolver, enum_args=None):
    """Return (return_type, [arg decls]) for a decoded parameter table. A by-value enum parameter is spelled as
    the plain enum (UHT's out-of-line signature), not TEnumAsByte<E>; its decl is appended to enum_args."""
    ret = "void"
    args = []
    offsets = []
    pairs, _ = split_properties(params)
    for entry, children in pairs:
        pf = hx(entry.get("property_flags"))
        refs = entry.get("type_refs") or {}
        by_value_enum = entry.get("kind") == "Byte" and "enum" in refs and not pf & (CPF_OUT_PARM | CPF_REFERENCE_PARM)
        if by_value_enum:
            ctype = resolver.type_ref("enum", refs["enum"], "Enum", "Byte")
        else:
            ctype = resolver.ctype(entry, children)
        if pf & CPF_RETURN_PARM:
            ret = ctype
            continue
        # Spelling follows UHT's own out-of-line signatures: T& for out, const T& (T* const& for pointers)
        # for a const reference, and by value otherwise (a CPF_ConstParm without a reference is emitted by value).
        if pf & CPF_OUT_PARM:
            ctype = ctype + "&"
        elif pf & CPF_REFERENCE_PARM:
            if pf & CPF_CONST_PARM:
                ctype = ctype + " const&" if ctype.endswith("*") else "const " + ctype + "&"
            else:
                ctype = ctype + "&"
        decl = "%s %s" % (ctype, entry.get("name") or "Param%d" % len(args))
        if by_value_enum and enum_args is not None:
            enum_args.append(decl)
        args.append(decl)
        if entry.get("offset") is not None:
            offsets.append(int(entry["offset"]))
    if PARAM_ORDER == "offset" and args:
        if any(b >= a for a, b in zip(offsets, offsets[1:])):
            PARAM_ORDER_BREAKS.append({"params": args, "offsets": offsets})
        else:
            args.reverse()
    return ret, args


def build_functions(functions, resolver):
    out = []
    for fn in functions:
        name = fn.get("name") or "UnnamedFunction"
        flag_values = fn.get("flags") or []
        flags = hx(flag_values[0]) if len(flag_values) == 1 else None
        specs = []
        if flags is not None:
            if flags & FUNC_NET_SERVER:
                specs.append("Server")
            if flags & FUNC_NET_CLIENT:
                specs.append("Client")
            if flags & FUNC_NET_MULTICAST:
                specs.append("NetMulticast")
            if flags & FUNC_NET_RELIABLE and flags & (FUNC_NET | FUNC_NET_SERVER | FUNC_NET_CLIENT | FUNC_NET_MULTICAST):
                specs.append("Reliable")
            if flags & FUNC_EXEC:
                specs.append("Exec")
        prefix = "static " if flags is not None and flags & FUNC_STATIC else ""
        suffix = " const" if flags is not None and flags & FUNC_CONST else ""
        todo = []
        if flags is None:
            todo.append("flags not joined (%s)" % (fn.get("note") or "ambiguous"))
        if fn.get("params") is not None:
            ret, args = decode_params(fn["params"], resolver)
        else:
            ret, args = "void", []
            todo.append("parameter table not decoded (%s)" % (fn.get("params_note") or "no FFunctionParams"))
        sig = "%s%s %s(%s)%s;" % (prefix, ret, name, ", ".join(args), suffix)
        out.append({
            "name": name,
            "flags": None if flags is None else "0x%x" % flags,
            "specifiers": specs,
            "decl": sig,
            "todo": todo,
        })
    return out


def uproperty_line(specs):
    return "UPROPERTY(%s)" % ", ".join(specs) if specs else "UPROPERTY()"


def uclass_line(block, minimal_api=True):
    """minimal_api=False for promoted classes: UHT rejects minimalapi on a class that uses the MODULE_API macro."""
    flags = hx(block.get("class_flags"))
    specs = []
    cfg = block.get("config_name")
    if cfg:
        specs.append("config=%s" % cfg)
    if flags & CLASS_DEFAULT_CONFIG:
        specs.append("defaultconfig")
    if flags & CLASS_ABSTRACT:
        specs.append("abstract")
    if flags & CLASS_TRANSIENT:
        specs.append("transient")
    if flags & CLASS_MINIMAL_API and minimal_api:
        specs.append("minimalapi")
    if flags & CLASS_DEPRECATED:
        specs.append("deprecated")
    return "UCLASS(%s)" % ", ".join(specs) if specs else "UCLASS()"


def build_class(block, name, resolver, order, parent_entry=None):
    """Build the class model. Parent and member types use resolver; order is 'declaration' or 'table'.

    parent_entry is this block's record from --parent-map (or None when no map was given). When it is
    given it decides the parent alone; an empty record means the parent is unresolved."""
    resolver.todos = []
    if parent_entry is not None:
        parent_fn = parent_entry.get("parent_va")
        parent = parent_entry.get("parent_name")
    else:
        parent_fn = block.get("base_fn")
        parent = resolver.name(parent_fn)
    if parent is None:
        parent_expr = "TODO_Parent_%s" % (parent_fn or "none").replace("0x", "")
        resolver.todos.append("parent not resolved (base_fn %s)" % parent_fn)
    else:
        parent_expr = parent
    members, notes = build_members(block.get("properties") or [], resolver)
    functions = build_functions(block.get("functions") or [], resolver)
    decl_members = list(reversed(members)) if order == "declaration" else list(members)
    rep_members = [m for m in members if m["replicated"]]
    function_names = {f["name"] for f in functions}
    rep_notify_missing = sorted({m["rep_notify"] for m in rep_members
                                 if m["rep_notify"] and m["rep_notify"] not in function_names})
    for notify in rep_notify_missing:
        resolver.todos.append("RepNotify function %s not in function table" % notify)
    return {
        "name": name,
        "block_va": block.get("block_va"),
        "parent": parent,
        "parent_fn": parent_fn,
        "parent_expr": parent_expr,
        "uclass": uclass_line(block),
        "config_name": block.get("config_name"),
        "members_table_order": members,
        "members_decl_order": decl_members,
        "functions": functions,
        "rep_notify_missing": rep_notify_missing,
        "notes": notes,
        "todos": sorted(set(resolver.todos)),
    }


def render_header(model, api, source_label):
    name = model["name"]
    lines = [
        "// Generated by porting/gen_native_classes.py from %s. Do not edit by hand." % source_label,
        "// Status: UNVERIFIED reconstruction. Names come from a name map; every TODO_ token must be resolved",
        "// before this file can compile. Not promoted to native/Source (see porting/gen_native_classes.md).",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
        "// TODO: include the header that declares %s" % model["parent_expr"],
        "#include \"%s.generated.h\"" % name,
        "",
    ]
    if model["parent"] is None:
        lines.append("// TODO: parent unresolved (base_fn %s)" % model["parent_fn"])
    lines.append(model["uclass"])
    lines.append("class %s %s : public %s" % (api, name, model["parent_expr"]) if api else
                 "class %s : public %s" % (name, model["parent_expr"]))
    lines.append("{")
    lines.append("\tGENERATED_BODY()")
    lines.append("")
    lines.append("public:")
    rep = [m for m in model["members_table_order"] if m["replicated"]]
    for m in model["members_decl_order"]:
        lines.append("\t// table index %s, flags %s" % (m["table_index"], m["property_flags"]))
        lines.append("\t%s" % uproperty_line(m["specifiers"]))
        lines.append("\t%s" % m["decl"])
        lines.append("")
    for notify in model["rep_notify_missing"]:
        lines.append("\t// TODO: declare %s (RepNotify function absent from the function table)" % notify)
    for f in model["functions"]:
        if f["specifiers"]:
            lines.append("\tUFUNCTION(%s)" % ", ".join(f["specifiers"]))
        else:
            lines.append("\tUFUNCTION()")
        for t in f["todo"]:
            lines.append("\t// TODO: %s" % t)
        lines.append("\t%s" % f["decl"])
    if rep:
        lines.append("")
        lines.append("\tvirtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;")
    lines.append("};")
    lines.append("")
    for t in sorted(set(model["todos"])):
        lines.append("// TODO-LIST: %s" % t)
    return "\n".join(lines) + "\n"


def render_cpp(model, source_label):
    name = model["name"]
    rep = [m for m in model["members_table_order"] if m["replicated"]]
    lines = [
        "// Generated by porting/gen_native_classes.py from %s. Do not edit by hand." % source_label,
        "#include \"%s.h\"" % name,
        "#include \"Net/UnrealNetwork.h\"",
        "",
    ]
    if not rep:
        lines.append("// No replicated properties in the reflection table; GetLifetimeReplicatedProps is inherited.")
        return "\n".join(lines) + "\n"
    lines.append("void %s::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const" % name)
    lines.append("{")
    if model["parent"] is None:
        lines.append("\t// TODO: parent unresolved; call the parent's GetLifetimeReplicatedProps here")
    else:
        lines.append("\tSuper::GetLifetimeReplicatedProps(OutLifetimeProps);")
    lines.append("")
    for m in rep:
        lines.append("\tDOREPLIFETIME(%s, %s);" % (name, m["name"]))
    lines.append("}")
    lines.append("")
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------- validation against a UE header

def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return "\n".join(line.split("//", 1)[0] for line in text.splitlines())


def parse_header_class(path, class_name):
    """Return the class's UPROPERTY and UFUNCTION declarations at member depth.

    Each item: {"kind": "prop"|"func", ...}. Items under any #if/#else block are flagged conditional."""
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        lines = strip_comments(fh.read()).splitlines()
    class_re = re.compile(r"^\s*class\s+(\w+_API\s+)?%s\b" % re.escape(class_name))
    cond = []
    depth = 0
    class_depth = None
    entered = False
    items = []
    state = None
    spec = ""
    spec_is_prop = False
    spec_cond = False
    stmt = ""
    for raw in lines:
        s = raw.strip()
        if s.startswith("#"):
            if s.startswith("#if"):
                cond.append(s)
            elif s.startswith("#endif"):
                if cond:
                    cond.pop()
            elif s.startswith(("#else", "#elif")) and cond:
                cond[-1] += " else"
            continue
        if class_depth is None and class_re.match(raw) and ";" not in raw:
            class_depth = depth
        in_member = class_depth is not None and depth == class_depth + 1
        if in_member:
            entered = True
        if state is None and in_member and s.startswith(("UPROPERTY(", "UFUNCTION(")):
            spec = s
            spec_is_prop = s.startswith("UPROPERTY(")
            spec_cond = bool(cond)
            state = "spec" if spec.count("(") > spec.count(")") else "stmt"
            stmt = ""
        elif state == "spec":
            spec += " " + s
            if spec.count("(") <= spec.count(")"):
                state = "stmt"
                stmt = ""
        elif state == "stmt" and s:
            stmt += " " + s
            if ";" in stmt or "{" in stmt:
                body = re.split(r"[;{]", stmt)[0].strip()
                if spec_is_prop:
                    items.append(parse_prop(spec, body, spec_cond, stmt))
                else:
                    items.append(parse_func(spec, body, spec_cond, stmt))
                state = None
        depth += s.count("{") - s.count("}")
        if entered and depth <= class_depth:
            break
    return items


def parse_prop(spec, body, conditional, line):
    bitfield_m = re.match(r"^(?P<type>[\w:]+)\s+(?P<name>\w+)\s*:\s*(?P<w>\d+)$", body)
    if bitfield_m:
        ctype = "bool"
        name = bitfield_m.group("name")
        bitfield = True
    else:
        m = re.match(r"^(?P<type>.+?)\s*(?P<name>\w+)\s*(\[\d+\])?$", body)
        if not m:
            return {"kind": "prop", "name": body, "type": body, "bitfield": False, "replicated": False,
                    "rep_notify": None, "specifiers": [], "conditional": conditional, "line": line,
                    "parse_error": True}
        ctype = m.group("type").strip()
        name = m.group("name")
        bitfield = False
    rep_notify = re.search(r"ReplicatedUsing\s*=\s*(\w+)", spec, re.I)
    replicated = bool(re.search(r"\bReplicated\b", spec, re.I)) or bool(rep_notify)
    return {"kind": "prop", "name": name, "type": normalize_type(ctype), "bitfield": bitfield,
            "replicated": replicated, "rep_notify": rep_notify.group(1) if rep_notify else None,
            "specifiers": spec, "conditional": conditional, "line": line}


def parse_func(spec, body, conditional, line):
    m = re.search(r"(\w+)\s*\(", body)
    name = m.group(1) if m else body
    net = {
        "Server": bool(re.search(r"\bServer\b", spec)),
        "Client": bool(re.search(r"\bClient\b", spec)),
        "NetMulticast": bool(re.search(r"\bNetMulticast\b", spec, re.I)),
    }
    return {"kind": "func", "name": name, "net": net, "specifiers": spec, "conditional": conditional,
            "line": line}


def longest_common_subsequence(a, b):
    """Return one longest common subsequence of two name lists (quadratic; class sizes are small)."""
    rows = len(a) + 1
    cols = len(b) + 1
    table = [[0] * cols for _ in range(rows)]
    for i in range(len(a) - 1, -1, -1):
        for j in range(len(b) - 1, -1, -1):
            table[i][j] = table[i + 1][j + 1] + 1 if a[i] == b[j] else max(table[i + 1][j], table[i][j + 1])
    out = []
    i = j = 0
    while i < len(a) and j < len(b):
        if a[i] == b[j]:
            out.append(a[i])
            i += 1
            j += 1
        elif table[i + 1][j] >= table[i][j + 1]:
            i += 1
        else:
            j += 1
    return out


def compare_with_header(model, header_path, report_lines):
    items = parse_header_class(header_path, model["name"])
    props_h = [i for i in items if i["kind"] == "prop"]
    funcs_h = [i for i in items if i["kind"] == "func"]
    excluded = [p["name"] for p in props_h if p["conditional"]]
    hdr = [p for p in props_h if not p["conditional"]]
    gen = model["members_decl_order"]
    gen_names = [m["name"] for m in gen]
    hdr_names = [p["name"] for p in hdr]
    report_lines.append("== %s vs %s" % (model["name"], header_path))
    report_lines.append("header UPROPERTY declarations: %d (non-conditional), conditional excluded: %d (%s)" % (
        len(hdr), len(excluded), ", ".join(excluded) or "none"))
    report_lines.append("generated members: %d (%s order)" % (len(gen), "declaration" if gen == list(reversed(model["members_table_order"])) else "table"))
    missing = [n for n in hdr_names if n not in gen_names]
    extra = [n for n in gen_names if n not in hdr_names]
    report_lines.append("names only in header (absent from reflection table): %s" % (", ".join(missing) or "none"))
    report_lines.append("names only in reflection table (absent from header): %s" % (", ".join(extra) or "none"))
    common_h = [n for n in hdr_names if n in gen_names]
    common_g = [n for n in gen_names if n in hdr_names]
    # Longest common subsequence of the two orders; names outside it are out of place.
    agreed = longest_common_subsequence(common_h, common_g)
    displaced = [n for n in common_g if n not in agreed]
    report_lines.append("order of common names: %d of %d in agreed order; out of place in generated order: %s" % (
        len(agreed), len(common_h), ", ".join(displaced) or "none"))
    by_name = {p["name"]: p for p in hdr}
    type_mismatch = []
    rep_mismatch = []
    for m in gen:
        h = by_name.get(m["name"])
        if h is None:
            continue
        gtype = "bool" if m["bitfield"] else normalize_type(m["ctype"])
        if h["type"] != gtype:
            type_mismatch.append("%s: header %s, generated %s" % (m["name"], h["type"], gtype))
        if h["replicated"] != m["replicated"] or (h["rep_notify"] or None) != (m["rep_notify"] or None):
            rep_mismatch.append("%s: header replicated=%s notify=%s; generated replicated=%s notify=%s" % (
                m["name"], h["replicated"], h["rep_notify"], m["replicated"], m["rep_notify"]))
    report_lines.append("type mismatches on common names: %d" % len(type_mismatch))
    for t in type_mismatch:
        report_lines.append("  " + t)
    report_lines.append("replication mismatches on common names: %d" % len(rep_mismatch))
    for t in rep_mismatch:
        report_lines.append("  " + t)
    unresolved = [m["name"] for m in gen if "TODO_" in m["ctype"]]
    report_lines.append("generated members with unresolved TODO types: %d (%s)" % (
        len(unresolved), ", ".join(unresolved) or "none"))
    # UFUNCTION names and net roles
    gen_funcs = {f["name"]: f for f in model["functions"]}
    hdr_func_names = {f["name"] for f in funcs_h if not f["conditional"]}
    report_lines.append("UFUNCTION declarations: header %d, generated %d; header-only %s; generated-only %s" % (
        len(hdr_func_names), len(gen_funcs),
        sorted(hdr_func_names - set(gen_funcs))[:20], sorted(set(gen_funcs) - hdr_func_names)[:20]))
    net_mismatch = []
    for f in funcs_h:
        g = gen_funcs.get(f["name"])
        if g is None or f["conditional"]:
            continue
        gspec = set(g["specifiers"])
        for role, label in (("Server", "Server"), ("Client", "Client"), ("NetMulticast", "NetMulticast")):
            if f["net"][role] != (label in gspec):
                net_mismatch.append("%s: header %s=%s generated=%s" % (f["name"], role, f["net"][role], label in gspec))
    report_lines.append("net-role mismatches on common UFUNCTIONs: %d" % len(net_mismatch))
    for t in net_mismatch[:40]:
        report_lines.append("  " + t)
    return {
        "type_mismatches": len(type_mismatch),
        "replication_mismatches": len(rep_mismatch),
        "missing_from_reflection": missing,
        "extra_in_reflection": extra,
        "order_match": common_h == common_g,
        "net_role_mismatches": len(net_mismatch),
    }


# ---------------------------------------------------------------- self test

SELF_TEST_BLOCK = {
    "block_va": "0x1000",
    "config_name": "Engine",
    "class_flags": "0x80",
    "base_fn": "0x2000",
    "dependency_fns": ["0x3000", "0x2000"],
    "properties": [
        {"index": 0, "name": "Tags", "kind": "Array", "property_flags": "0x10040000000005", "replicated": False,
         "rep_notify": None, "array_dim": 1, "offset": 0},
        {"index": 1, "name": "Tags", "kind": "Name", "property_flags": "0x0", "replicated": False,
         "rep_notify": None, "array_dim": 1, "array_inner_of": "Tags", "offset": 0},
        {"index": 2, "name": "Owner", "kind": "Object", "property_flags": "0x40000100000020", "replicated": True,
         "rep_notify": "OnRep_Owner", "array_dim": 1, "offset": 8, "type_refs": {"class": "0x2100"}},
        {"index": 3, "name": "Role", "kind": "Byte", "property_flags": "0x10000000000020", "replicated": True,
         "rep_notify": None, "array_dim": 1, "offset": 16, "type_refs": {"enum": "0x2200"}},
        {"index": 4, "name": "bHidden", "kind": "Bool", "property_flags": "0x10000200000035", "replicated": True,
         "rep_notify": None, "array_dim": 1, "bitfield": True, "offset": None},
        {"index": 5, "name": "Mode", "kind": "Enum", "property_flags": "0x10000000000005", "replicated": False,
         "rep_notify": None, "array_dim": 1, "offset": 24, "type_refs": {"enum": "0x2300"}},
        {"index": 6, "name": "UnderlyingType", "kind": "Byte", "property_flags": "0x0", "replicated": False,
         "rep_notify": None, "array_dim": 1, "enum_underlying_of": "Mode", "offset": None},
        {"index": 7, "name": "Pos", "kind": "Struct", "property_flags": "0x40000000000021", "replicated": True,
         "rep_notify": None, "array_dim": 1, "offset": 32, "type_refs": {"struct": "0x2400"}},
        {"index": 8, "name": "Subs", "kind": "Class", "property_flags": "0x10000000000005", "replicated": False,
         "rep_notify": None, "array_dim": 1, "offset": 40, "type_refs": {"meta_class": "0x2500", "class": "0x2600"}},
        {"index": 9, "name": "Cache", "kind": "Map", "property_flags": "0x10000000000005", "replicated": False,
         "rep_notify": None, "array_dim": 1, "offset": 48},
        {"index": 10, "name": "Cache_Key", "kind": "Name", "property_flags": "0x0", "replicated": False,
         "rep_notify": None, "array_dim": 1, "offset": None},
        {"index": 11, "name": "Cache", "kind": "Int", "property_flags": "0x0", "replicated": False,
         "rep_notify": None, "array_dim": 1, "offset": None},
        {"index": 12, "name": "Hits", "kind": "Int", "property_flags": "0x40000000000021", "replicated": True,
         "rep_notify": None, "array_dim": 3, "offset": 64},
    ],
    "functions": [
        {"name": "ServerDo", "create_func": "0x1", "flags": ["0x2000c0"], "replicated_function": True,
         "net_roles": ["Server"], "params": [
             {"index": 0, "name": "Amount", "kind": "Int", "property_flags": "0x80", "array_dim": 1,
              "offset": 0, "replicated": False}]},
        {"name": "ClientTell", "create_func": "0x2", "flags": ["0x1000040"], "replicated_function": True,
         "net_roles": ["Client"], "params": [
             {"index": 0, "name": "Text", "kind": "Str", "property_flags": "0x82", "array_dim": 1,
              "offset": 0, "replicated": False}]},
        {"name": "MulticastBoom", "create_func": "0x3", "flags": ["0x4040"], "replicated_function": True,
         "net_roles": ["Multicast"], "params": []},
        {"name": "Unknown", "create_func": "0x4", "flags": [], "note": "no FFunctionParams joined by name pointer"},
    ],
}
SELF_TEST_TYPES = {"0x2000": "UObject", "0x2100": "AActor", "0x2200": "ENetRole", "0x2300": "EMode",
                   "0x2400": "FVector", "0x2500": "UClass", "0x2600": "UClass"}


def types_self_test(check):
    """Checks for the struct, enum and delegate machinery (synthetic records; no files are read)."""
    # Include spelling from a header path.
    check("include path after Public", include_path("/e/Source/Runtime/Core/Public/Math/Vector.h") == "Math/Vector.h")
    check("include path after Classes", include_path("/e/Source/Runtime/Engine/Classes/Engine/X.h") == "Engine/X.h")
    check("include path needs a module directory", include_path("/e/Source/Other/X.h") is None)
    # Struct rendering: declaration order, parent, USTRUCT, UPROPERTY on every member.
    member = lambda decl, table, off: {"decl": decl, "table_index": table, "offset": off, "property_flags": "0x0"}
    struct_model = {"cpp_name": "FSelfTest", "parent": "FParentStruct", "parent_include": "Engine/Parent.h",
                    "includes": ["Math/Vector.h", "Engine/Parent.h"], "forward_decls": ["AActor"],
                    "members": [member("int32 A;", 1, 0), member("FVector B;", 0, 8)]}
    text = render_struct_header(struct_model, "self-test")
    check("struct: USTRUCT and parent", "USTRUCT()\nstruct FSelfTest : public FParentStruct" in text)
    check("struct: declaration order kept", text.index("int32 A;") < text.index("FVector B;"))
    check("struct: UPROPERTY per member", text.count("UPROPERTY()") == 2)
    check("struct: includes and generated header", '#include "Math/Vector.h"' in text and "FSelfTest.generated.h" in text)
    check("struct: forward declaration of a class reference", "class AActor;" in text)
    # Enum rendering: enum class, namespaced (Namespace::Type) and regular.
    enum_class = render_enum_header({"cpp_name": "ELabel", "form": "EnumClass", "values": [("A", 0), ("B", 3)]}, "t")
    check("enum class with uint8", "enum class ELabel : uint8" in enum_class and "B = 3," in enum_class)
    namespaced = render_enum_header({"cpp_name": "ENs::Type", "form": "Namespaced", "values": [("X", 1)]}, "t")
    check("namespaced enum", "namespace ENs" in namespaced and "enum Type" in namespaced and "X = 1," in namespaced)
    check("namespaced enum includes its namespace header", '#include "ENs.generated.h"' in namespaced)
    regular = render_enum_header({"cpp_name": "ERegular", "form": "Regular", "values": [("R", 0)]}, "t")
    check("regular enum", "enum ERegular\n{" in regular)
    # Delegate rendering: macro by arity and kind.
    params = [("int32", "A"), ("FString", "B")]
    text = render_delegate_header({"cpp_name": "FOnSelfTest", "params": params, "includes": []}, "multicast", "t")
    check("delegate macro: two params",
          "DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelfTest, int32, A, FString, B);" in text)
    text = render_delegate_header({"cpp_name": "FSingle", "params": [], "includes": []}, "single", "t")
    check("delegate macro: no params", "DECLARE_DYNAMIC_DELEGATE(FSingle);" in text)
    text = render_delegate_header({"cpp_name": "FSparse", "params": [("int32", "A")], "includes": []}, "sparse", "t")
    check("delegate macro: sparse one param", "DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FSparse, int32, A);" in text)
    check("delegate kind: single unless multicast flag",
          delegate_macro_kind({"multicast": False}, {"InlineMulticastDelegate"}) == "single")
    check("delegate kind: sparse when a sparse property uses it",
          delegate_macro_kind({"multicast": True}, {"SparseMulticastDelegate"}) == "sparse")
    check("delegate kind: multicast by default", delegate_macro_kind({"multicast": True}, {"InlineMulticastDelegate"}) == "multicast")
    # Gate categories for the new skip tokens.
    check("category: struct unavailable", category_of(["TODO_StructUnavailable_1234"]) == "struct_not_available")
    check("category: struct not emittable", category_of(["TODO_StructNotEmittable_1234"]) == "struct_member_unresolved")
    check("category: delegate params", category_of(["TODO_DelegateParams_1234"]) == "delegate_params_unsupported")
    check("category: enum other module", category_of(["TODO_EnumOtherModule_1"]) == "enum_other_module")
    check("category: plain struct unchanged", category_of(["TODO_Struct_1234"]) == "struct")
    # Wire-custom propagation: a Fort struct with a custom serializer taints the structs that contain it; an engine
    # struct with a custom serializer does not (its NetSerialize is engine code).
    def fake_info(kind, fn, name, package, traits=None, props=()):
        rec = {"name": name, "properties": list(props), "struct_ops": {"traits": traits or {}}}
        return TypeInfo(kind, fn, rec, "F" + name, package, "F%s.h" % name)
    tables = TypeTables.__new__(TypeTables)
    custom = fake_info("struct", "0x1", "Custom", "/Script/FortniteGame", {"HasNetSerializer": True})
    outer = fake_info("struct", "0x2", "Outer", "/Script/FortniteGame", {},
                      [{"type_refs": {"struct": "0x1"}, "kind": "Struct", "name": "C"}])
    engine = fake_info("struct", "0x3", "EngineCustom", "/Script/Engine", {"HasNetSerializer": True})
    plain = fake_info("struct", "0x4", "Plain", "/Script/FortniteGame", {})
    tables.by_fn = {i.fn: i for i in (custom, outer, engine, plain)}
    gate = TypeGate(tables, EngineTypeIndex(os.devnull), None)
    check("wire: custom serializer", gate.wire_custom(custom) is True)
    check("wire: containing struct inherits", gate.wire_custom(outer) is True)
    check("wire: engine custom serializer not counted", gate.wire_custom(engine) is False)
    check("wire: plain struct", gate.wire_custom(plain) is False)
    # Net serializers: an FFastArraySerializer over its FFastArraySerializerItem array is emitted natively; any other
    # custom serializer is a declared fallback; no custom trait means no net serializer.
    fa_parent = fake_info("struct", "0x10", "FastArraySerializer", "/Script/Engine")
    fa_item_parent = fake_info("struct", "0x12", "FastArraySerializerItem", "/Script/Engine")
    fa_item = fake_info("struct", "0x11", "Item", "/Script/FortniteGame", {})
    fa_item.rec["super_fn"] = "0x12"
    fa_holder = fake_info("struct", "0x13", "Holder", "/Script/FortniteGame", {"HasNetDeltaSerializer": True},
                          [{"name": "Items", "kind": "Array", "array_dim": 1},
                           {"name": "Items", "kind": "Struct", "array_inner_of": "Items", "type_refs": {"struct": "0x11"}}])
    fa_holder.rec["super_fn"] = "0x10"
    fa_holder.rec["struct_ops"]["vtable_va"] = "0x1459e0000"
    for extra in (fa_parent, fa_item_parent, fa_item, fa_holder):
        tables.by_fn[extra.fn] = extra
    holder_net = gate.net_serializer(fa_holder, [{"name": "Items"}])
    check("net: FastArraySerializer is a delta serializer", holder_net and holder_net["kind"] == "delta"
          and holder_net["member"] == "Items" and holder_net["item"] == "FItem")
    check("net: custom serializer without the array pattern is a fallback",
          gate.net_serializer(custom, [])["kind"] == "fallback")
    check("net: no custom trait, no net serializer", gate.net_serializer(plain, []) is None)
    text = render_struct_header({"cpp_name": "FHolder", "parent": "FFastArraySerializer", "parent_include": None,
                                 "includes": [], "forward_decls": [], "members": [], "net": holder_net}, "self-test")
    check("net: delta header declares NetDeltaSerialize and the trait",
          "FFastArraySerializer::FastArrayDeltaSerialize<FItem, FHolder>(Items, DeltaParms, *this);" in text
          and "struct TStructOpsTypeTraits<FHolder>" in text and "WithNetDeltaSerializer = true" in text
          and "Engine/NetSerialization.h" in text)
    text = render_struct_header({"cpp_name": "FCustom", "parent": None, "parent_include": None, "includes": [],
                                 "forward_decls": [], "members": [],
                                 "net": {"kind": "fallback", "traits": ["HasNetSerializer"], "vtable": "0x1",
                                         "reason": "native HasNetSerializer not recovered"}}, "self-test")
    check("net: fallback header is marked TODO and declares no trait",
          "TODO: the native net serializer of FCustom is not recovered" in text
          and "TStructOpsTypeTraits" not in text)
    # struct_net (native): the body is declared inline one level deeper, UPackageMap is forward declared, and the traits
    # follow the binary (WithNetSharedSerialization only when the binary has HasNetSharedSerialization).
    native = {"kind": "native", "body": "bOutSuccess = true;\nreturn true;", "inc_path": "porting/behavior/FNative/x.inc",
              "traits": ["HasNetSerializer", "HasNetSharedSerialization"], "vtable": "0x1", "includes": [], "uses": []}
    native_model = {"cpp_name": "FNative", "parent": None, "parent_include": None, "includes": [], "forward_decls": [],
                    "members": [], "net": native}
    text = render_struct_header(native_model, "self-test")
    check("net: native header declares NetSerialize inline with the shared trait",
          "\t%s\n\t{\n\t\tbOutSuccess = true;\n\t\treturn true;\n\t}" % NATIVE_NET_SIGNATURE in text
          and "class UPackageMap;" in text
          and "struct TStructOpsTypeTraits<FNative> : public TStructOpsTypeTraitsBase2<FNative>" in text
          and "enum { WithNetSerializer = true, WithNetSharedSerialization = true };" in text
          and "TODO" not in text)
    verify_struct_net(native_model, text)   # raises SystemExit on a mismatch: the self-test fails with it
    native["traits"] = ["HasNetSerializer"]
    text = render_struct_header(native_model, "self-test")
    check("net: native header without the shared trait declares WithNetSerializer only",
          "enum { WithNetSerializer = true };" in text and "WithNetSharedSerialization" not in text)
    # Replication conditions: a recovered condition is emitted as its macro, an unknown one as COND_None with a TODO.
    REPLICATION_CONDITIONS.clear()
    REPLICATION_CONDITIONS.update({"AFakeClass": {"properties": {
        "Owned": {"condition": "COND_OwnerOnly", "confidence": "high"},
        "Notify": {"condition": "COND_None", "rep_notify_condition": "REPNOTIFY_Always", "confidence": "medium"},
        "Unknown": {"condition": "COND_None", "confidence": "unknown"}}}})
    check("cond: owner only", replication_line("AFakeClass", "Owned")
          == "\tDOREPLIFETIME_CONDITION(AFakeClass, Owned, COND_OwnerOnly);")
    check("cond: notify", replication_line("AFakeClass", "Notify").startswith(
        "\tDOREPLIFETIME_CONDITION_NOTIFY(AFakeClass, Notify, COND_None, REPNOTIFY_Always);"))
    check("cond: unknown is COND_None with TODO", "TODO: replication condition not recovered" in
          replication_line("AFakeClass", "Unknown"))
    REPLICATION_CONDITIONS.clear()
    check("cond: no data is plain DOREPLIFETIME", replication_line("AFakeClass", "Owned") == "\tDOREPLIFETIME(AFakeClass, Owned);")
    # Resolver: without a gate a known type name is used as is; an unknown one is a TODO token.
    res = Resolver({"0x9": "ENetRole"})
    check("type_ref without gate", res.type_ref("enum", "0x9", "Enum") == "ENetRole")
    check("type_ref unknown is a token", res.type_ref("struct", "0x77", "Struct").startswith("TODO_Struct_"))
    check("engine type index on a missing root is empty", EngineTypeIndex("/nonexistent/engine").lookup("struct", "FVector") is None)
    # Parameter order: the table lists parameters in reverse declaration order (ascending offsets in declaration
    # order). The "offset" order spells them in declaration order; "table" keeps the listed order.
    global PARAM_ORDER
    saved_order = PARAM_ORDER
    try:
        listed = [{"name": "Second", "kind": "Int", "property_flags": "0x10000000000080", "offset": 4},
                  {"name": "First", "kind": "Int", "property_flags": "0x10000000000080", "offset": 0}]
        PARAM_ORDER = "offset"
        check("params: declaration order from the table's reverse order",
              decode_params(listed, Resolver({}))[1] == ["int32 First", "int32 Second"])
        PARAM_ORDER = "table"
        check("params: table order kept when selected",
              decode_params(listed, Resolver({}))[1] == ["int32 Second", "int32 First"])
        # The Enum's UnderlyingType record (Int kind here) is paired with the enum, not emitted as a member.
        enum_rows = [{"name": "E", "kind": "Enum", "property_flags": "0x10000000000080", "offset": 8,
                      "type_refs": {"enum": "0x1"}},
                     {"name": "UnderlyingType", "kind": "Int", "property_flags": "0x0", "offset": 0},
                     {"name": "M", "kind": "Int", "property_flags": "0x10000000000080", "offset": 0}]
        check("enum underlying Int record is not a member",
              [e["name"] for e, _c in split_properties(enum_rows)[0]] == ["E", "M"])
        # A delegate with a const reference parameter: const T& is emitted, the out flag alone is rejected.
        PARAM_ORDER = "offset"
        const_ref = [{"name": "Reason", "kind": "Text", "property_flags": "0x10000008000182", "offset": 8},
                     {"name": "bSuccess", "kind": "Bool", "property_flags": "0x10000000000080", "offset": None,
                      "bitfield": True}]
        dinfo = fake_info("delegate", "0x7", "FOnSelfTestRef", "/Script/FortniteGame")
        dinfo.rec.update({"params": const_ref, "multicast": True})
        tables.by_fn[dinfo.fn] = dinfo
        gate = TypeGate(tables, EngineTypeIndex(os.devnull), Resolver({}))
        model, reason = gate.build_delegate(dinfo)
        check("delegate: const reference accepted", reason is None and model is not None)
        check("delegate: const reference spelled const T&, parameters in declaration order",
              model is not None and model["params"] == [("bool", "bSuccess"), ("const FText&", "Reason")])
        out_only = [{"name": "Out", "kind": "Int", "property_flags": "0x100", "offset": 0}]
        dinfo2 = fake_info("delegate", "0x8", "FOnSelfTestOut", "/Script/FortniteGame")
        dinfo2.rec.update({"params": out_only, "multicast": True})
        tables.by_fn[dinfo2.fn] = dinfo2
        check("delegate: non-const out parameter rejected", gate.build_delegate(dinfo2)[1] is not None)
    finally:
        PARAM_ORDER = saved_order


def helper_self_test(check):
    """Helper kinds on synthetic input: signature forms, type rules, dependency order, attach and verify."""
    def rejects(sig, static):
        try:
            parse_helper_signature(sig, static)
        except ValueError:
            return True
        return False

    check("helper signature: return type, name, const",
          parse_helper_signature("UFortPlaylistAthena* GetActivePlaylist() const", False)
          == ("UFortPlaylistAthena*", "GetActivePlaylist", [], True))
    check("helper signature: parameters in order, template commas kept",
          parse_helper_signature("bool Find(const FGuid& Id, TMap<FName, int32> Vals)", False)
          == ("bool", "Find", ["const FGuid& Id", "TMap<FName, int32> Vals"], False))
    check("static helper cannot be const", rejects("int32 F() const", True))
    check("static helper cannot spell static", rejects("static int32 F()", True))
    check("helper parameter needs a name", rejects("void F(int32)", False))
    check("helper signature needs parentheses", rejects("int32 F", False))
    check("qualified enum: only the enum name is a type",
          helper_type_names("EFortCustomPartType::Type", [])[0] == {"EFortCustomPartType"})
    ordered, left = static_order(["A", "B", "C", "D"], {"A": set(), "B": {"A"}, "C": {"B", "D"}, "D": {"C"}})
    check("static order: dependencies first, cycles left", ordered == ["A", "B"] and left == {"C", "D"})

    class FakeEngine:
        module_of_inc = {"Engine/World.h": "Engine"}

    class FakeGate:
        engine = FakeEngine()

        def include_of(self, fn):
            return None

    ctx = {"class_pkg": {"ASelfTest": "/Script/FortniteGame", "UFortPlaylistAthena": "/Script/FortniteGame"},
           "struct_index": {}, "gate": FakeGate(), "other_promoted": set()}
    entries = [
        {"kind": "static_helper", "function": "StaticB", "signature": "int32 StaticB()",
         "body": "return StaticA() + 1;"},
        {"kind": "static_helper", "function": "StaticA", "signature": "int32 StaticA()", "body": "return 1;"},
        {"kind": "helper", "function": "Pick", "access": "protected",
         "signature": "UFortPlaylistAthena* Pick() const", "body": "return nullptr;"},
        {"kind": "helper", "function": "Bad", "access": "private", "signature": "void Bad()", "body": ";"},
        {"kind": "static_helper", "function": "UsesMember", "signature": "int32 UsesMember()",
         "body": "return Pick() ? 1 : 0;"},
        {"kind": "static_helper", "function": "CycleX", "signature": "int32 CycleX()", "body": "return CycleY();"},
        {"kind": "static_helper", "function": "CycleY", "signature": "int32 CycleY()", "body": "return CycleX();"},
        {"kind": "helper", "function": "NoBody", "access": "public", "signature": "void NoBody()", "body": None,
         "reject": "snippet file missing: x"},
    ]
    res = attach_snippets("ASelfTest", [], entries, ctx, "/Script/FortniteGame")
    check("helper accepted: Pick, StaticA, StaticB",
          sorted(res["accepted"]) == ["ASelfTest::Pick", "ASelfTest::StaticA", "ASelfTest::StaticB"])
    check("static helpers emitted dependencies first",
          [s["name"] for s in res["static_helpers"]] == ["StaticA", "StaticB"])
    reasons = {r["entry"]: r["reason"] for r in res["rejected"]}
    check("helper with private access rejected", "public or protected" in reasons.get("ASelfTest::Bad", ""))
    check("static helper naming a member helper rejected",
          "member helper Pick" in reasons.get("ASelfTest::UsesMember", ""))
    check("static helper cycle rejected",
          "cycle" in reasons.get("ASelfTest::CycleX", "") and "cycle" in reasons.get("ASelfTest::CycleY", ""))
    check("helper without a usable body rejected", "snippet file missing" in reasons.get("ASelfTest::NoBody", ""))
    check("helper forward declares its class type", res["forward_decls"] == {"UFortPlaylistAthena"})
    model = {"name": "ASelfTest", "parent": "AActor", "uclass": "UCLASS()",
             "forward_decls": sorted(res["forward_decls"]), "includes": sorted(res["header_includes"]),
             "cpp_includes": sorted(res["includes"]), "members_table_order": [], "members_decl_order": [],
             "functions": [], "helpers": res["helpers"], "static_helpers": res["static_helpers"]}
    header = render_promoted_header(model, "SELFTEST_API", "AActor.h", "self-test")
    cpp = render_promoted_cpp(model, "self-test")
    check("protected helper declared under its label, outside UFUNCTION",
          "protected:\n\tUFortPlaylistAthena* Pick() const;" in header)
    check("static helper has no header declaration", "StaticA" not in header)
    check("static helpers precede the namespace close and member definitions",
          cpp.index("static int32 StaticB()") < cpp.index("}  // namespace")
          < cpp.index("UFortPlaylistAthena* ASelfTest::Pick() const"))

    def verifies(h, c):
        try:
            verify_spliced(model, h, c)
        except SystemExit:
            return False
        return True

    check("rendered helpers verify", verifies(header, cpp))
    check("verify: access label removed is detected", not verifies(header.replace("protected:\n", "", 1), cpp))
    check("verify: body edit is detected", not verifies(header, cpp.replace("return 1;", "return 2;", 1)))
    check("verify: static outside the namespace is detected",
          not verifies(header, cpp.replace("namespace\n{", "{", 1)))
def field_self_test(check):
    """member_field kind on synthetic input: signature, rules, placement by offset, rendering and verification."""
    def field_rejects(sig):
        try:
            parse_field_signature(sig)
        except ValueError:
            return True
        return False

    check("field signature: template and pointer types, then the name",
          parse_field_signature("TMulticastDelegate<void(AFortPlayerStateAthena*, int32)> ScoreDel")
          == ("TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>", "ScoreDel"))
    check("field signature: plain type and name", parse_field_signature("int32 KillPointsCache")
          == ("int32", "KillPointsCache"))
    check("field signature: a name alone is rejected", field_rejects("int32"))
    check("field signature: static is rejected", field_rejects("static int32 X"))
    check("field offset: hex, decimal, absent",
          field_offset("0xe88") == 0xe88 and field_offset(3720) == 3720 and field_offset(None) is None)

    class FakeEngine:
        module_of_inc = {"Delegates/Delegate.h": "Core"}

    class FakeGate:
        engine = FakeEngine()

        def include_of(self, fn):
            return None

    ctx = {"class_pkg": {"ASelfTest": "/Script/FortniteGame", "AFortPlayerStateAthena": "/Script/FortniteGame"},
           "struct_index": {}, "gate": FakeGate(), "other_promoted": set()}
    entries = [
        {"kind": "member_field", "function": "PointsA", "signature": "int32 PointsA", "access": "public",
         "offset": "0x100", "default": "0"},
        {"kind": "member_field", "function": "ScoreDel", "access": "protected", "offset": 0x150,
         "signature": "TMulticastDelegate<void(AFortPlayerStateAthena*, int32)> ScoreDel",
         "includes": ["Delegates/Delegate.h"]},
        {"kind": "member_field", "function": "NoOffset", "signature": "int32 NoOffset", "access": "private",
         "default": 3},
        {"kind": "member_field", "function": "Reflected", "signature": "int32 Reflected", "access": "public"},
        {"kind": "member_field", "function": "NoType", "signature": "UWorld* NoType", "access": "public",
         "offset": 0x200},
        {"kind": "member_field", "function": "BadAccess", "signature": "int32 BadAccess", "access": "package"},
        {"kind": "member_field", "function": "HasBody", "signature": "int32 HasBody", "access": "public",
         "inc_path": "x.inc"},
    ]
    res = attach_snippets("ASelfTest", [], entries, ctx, "/Script/FortniteGame", (), {"Reflected"})
    check("fields accepted: PointsA, ScoreDel, NoOffset",
          sorted(res["accepted"]) == ["ASelfTest::NoOffset", "ASelfTest::PointsA", "ASelfTest::ScoreDel"])
    check("fields in offset order, no offset last",
          [f["name"] for f in res["fields"]] == ["PointsA", "ScoreDel", "NoOffset"])
    reasons = {r["entry"]: r["reason"] for r in res["rejected"]}
    check("field clashing with a reflected member rejected", "already a member" in reasons.get("ASelfTest::Reflected", ""))
    check("field with an unknown type rejected", "not known" in reasons.get("ASelfTest::NoType", ""))
    check("field with an unknown access rejected", "access" in reasons.get("ASelfTest::BadAccess", ""))
    check("field with a body rejected", "no body" in reasons.get("ASelfTest::HasBody", ""))
    check("field default number rendered as text",
          [f["decl"] for f in res["fields"] if f["name"] == "NoOffset"] == ["int32 NoOffset = 3;"])
    check("field includes reach the header", "Delegates/Delegate.h" in res["header_includes"])
    members = [
        {"name": "A", "decl": "int32 A;", "specifiers": ["Transient"], "table_index": 1, "property_flags": "0x1",
         "replicated": False, "offset": 0xF0},
        {"name": "B", "decl": "int32 B;", "specifiers": ["Transient"], "table_index": 0, "property_flags": "0x1",
         "replicated": False, "offset": 0x200},
    ]
    model = {"name": "ASelfTest", "parent": "AActor", "uclass": "UCLASS()",
             "forward_decls": sorted(res["forward_decls"]), "includes": sorted(res["header_includes"]),
             "cpp_includes": [], "members_table_order": members, "members_decl_order": members,
             "functions": [], "helpers": [], "static_helpers": [], "fields": res["fields"]}
    header = render_promoted_header(model, "SELFTEST_API", "AActor.h", "self-test")
    cpp = render_promoted_cpp(model, "self-test")
    at = [header.index(s) for s in ("int32 A;", "int32 PointsA = 0;", "ScoreDel;", "int32 B;", "int32 NoOffset = 3;")]
    check("fields placed by offset among the reflected members", at == sorted(at))
    check("protected field under its label, public restored after it",
          header.index("protected:") < header.index("ScoreDel;") < header.index("public:", header.index("ScoreDel;")))
    check("private field under its label", "private:\n" in header and header.index("private:") < header.index("NoOffset"))
    check("no field under UPROPERTY", "UPROPERTY()\n\tTMulticast" not in header
          and "UPROPERTY(Transient)\n\tint32 PointsA" not in header)

    def verifies(model_, h, c):
        try:
            verify_spliced(model_, h, c)
        except SystemExit:
            return False
        return True

    check("rendered fields verify", verifies(model, header, cpp))
    check("verify: field access label removed is detected",
          not verifies(model, header.replace("protected:\n", "", 1), cpp))
    check("verify: field under UPROPERTY is detected",
          not verifies(model, header.replace("\tTMulticastDelegate<void(AFortPlayerStateAthena*, int32)> ScoreDel;",
                                             "\tUPROPERTY()\n\tTMulticastDelegate<void(AFortPlayerStateAthena*, int32)> ScoreDel;", 1),
                       cpp))
    moved = dict(model, fields=[dict(f, offset=0x300) if f["name"] == "PointsA" else f for f in res["fields"]])
    check("verify: field placed against its offset is detected", not verifies(moved, header, cpp))
def static_reflected_self_test(check):
    """Staticness comes from reflection and is required by the snippet and declaration."""
    resolver = Resolver({})
    resolver.skipped = []
    fn = {"name": "Find", "flags": ["0x14022401"], "params": []}
    functions = build_functions_promoted([fn], resolver, set(), set(), {"Find"})
    check("static reflected function emitted from flags", len(functions) == 1 and functions[0]["static"])
    ctx = {"class_pkg": {"ASelfTest": "/Script/FortniteGame"}, "struct_index": {},
           "gate": None, "other_promoted": set()}
    entry = {"kind": "getter", "function": "Find", "signature": "void Find()", "static": True,
             "body": "return;"}
    bad = dict(entry, static=False)
    rejected = attach_snippets("ASelfTest", functions, [bad], ctx, "/Script/FortniteGame")
    check("staticness mismatch rejected", any("staticness" in r["reason"] for r in rejected["rejected"]))
    rejected = attach_snippets("ASelfTest", functions, [dict(entry, body="this->Bad();")], ctx,
                               "/Script/FortniteGame")
    check("static reflected function rejects this", any("uses this" in r["reason"] for r in rejected["rejected"]))
    rejected = attach_snippets("ASelfTest", functions, [dict(entry, body="Count = 1;")], ctx,
                               "/Script/FortniteGame", member_names={"Count"})
    check("static reflected function rejects bare instance member", bool(rejected["rejected"]))
    accepted = attach_snippets("ASelfTest", functions, [entry], ctx, "/Script/FortniteGame")
    check("static reflected snippet accepted", accepted["accepted"] == ["ASelfTest::Find"])
    model = {"name": "ASelfTest", "parent": "AActor", "uclass": "UCLASS()", "forward_decls": [],
             "includes": [], "cpp_includes": [], "members_table_order": [], "members_decl_order": [],
             "functions": functions, "helpers": [], "static_helpers": []}
    hdr = render_promoted_header(model, "SELFTEST_API", "AActor.h", "self-test")
    cpp = render_promoted_cpp(model, "self-test")
    check("static declaration carries static", "static void Find();" in hdr)
    check("out-of-class static definition omits static", "void ASelfTest::Find()" in cpp)
    try:
        verify_spliced(model, hdr, cpp)
        valid = True
    except SystemExit:
        valid = False
    check("static declaration and body verify", valid)
    try:
        verify_spliced(model, hdr.replace("static void Find();", "void Find();"), cpp)
        detects_missing_static = False
    except SystemExit:
        detects_missing_static = True
    check("verification detects missing static", detects_missing_static)
    bad_fn = dict(fn, flags=["0x54022401"])
    resolver2 = Resolver({})
    resolver2.skipped = []
    check("static const combination rejected", not build_functions_promoted([bad_fn], resolver2, set(), set(), {"Find"}))

    class FakeEngine:
        module_of_inc = {"Misc/App.h": "Core"}
    class FakeGate:
        engine = FakeEngine()
    ctx["gate"] = FakeGate()
    fn2 = dict(functions[0], body=None)
    good = dict(entry, includes=["Misc/App.h"])
    res = attach_snippets("ASelfTest", [fn2], [good], ctx, "/Script/FortniteGame")
    check("snippet include accepted and emitted", res["includes"] == {"Misc/App.h"})
    res = attach_snippets("ASelfTest", [dict(fn2, body=None)], [dict(entry, includes=["Missing.h"])], ctx,
                          "/Script/FortniteGame")
    check("unknown snippet include rejected", bool(res["rejected"]))


def self_test():
    failures = []
    count = [0]

    def check(label, condition):
        count[0] += 1
        if not condition:
            failures.append(label)

    resolver = Resolver(SELF_TEST_TYPES)
    block = dict(SELF_TEST_BLOCK)
    model = build_class(block, "ASelfTest", resolver, "declaration")
    names = [m["name"] for m in model["members_table_order"]]
    check("array inner and enum underlying records are not members",
          names == ["Tags", "Owner", "Role", "bHidden", "Mode", "Pos", "Subs", "Cache", "Hits"])
    decls = {m["name"]: m["decl"] for m in model["members_table_order"]}
    check("TArray<FName>", decls["Tags"] == "TArray<FName> Tags;")
    check("object pointer via class fn", decls["Owner"] == "AActor* Owner;")
    check("enum byte is TEnumAsByte", decls["Role"] == "TEnumAsByte<ENetRole> Role;")
    check("bitfield bool", decls["bHidden"] == "uint8 bHidden : 1;")
    check("enum class", decls["Mode"] == "EMode Mode;")
    check("struct by name", decls["Pos"] == "FVector Pos;")
    check("TSubclassOf via meta class", decls["Subs"] == "TSubclassOf<UClass> Subs;")
    check("TMap of key/value records", decls["Cache"] == "TMap<FName, int32> Cache;")
    check("static array", decls["Hits"] == "int32 Hits[3];")
    check("ReplicatedUsing from rep notify", "ReplicatedUsing=OnRep_Owner" in model["members_table_order"][1]["specifiers"])
    check("declaration order is reverse of table", [m["name"] for m in model["members_decl_order"]] == list(reversed(names)))
    check("parent resolved via type map", model["parent"] == "UObject")
    mapped = build_class(dict(SELF_TEST_BLOCK, base_fn="0x3000"), "ASelfTest", Resolver(SELF_TEST_TYPES),
                         "declaration", {"parent_name": "AParentFromMap", "parent_va": "0x5000"})
    check("parent map overrides base_fn", mapped["parent"] == "AParentFromMap" and mapped["parent_fn"] == "0x5000")
    check("parent map cpp calls Super", "Super::GetLifetimeReplicatedProps" in render_cpp(mapped, "self-test"))
    empty = build_class(SELF_TEST_BLOCK, "ASelfTest", Resolver(SELF_TEST_TYPES), "declaration", {})
    check("empty parent-map record is unresolved", empty["parent"] is None and empty["parent_expr"] == "TODO_Parent_none")
    fn = {f["name"]: f for f in model["functions"]}
    check("Server + Reliable flags", fn["ServerDo"]["specifiers"] == ["Server", "Reliable"])
    check("Client flag", fn["ClientTell"]["specifiers"] == ["Client"])
    check("NetMulticast flag", fn["MulticastBoom"]["specifiers"] == ["NetMulticast"])
    check("UFUNCTION signature", fn["ServerDo"]["decl"] == "void ServerDo(int32 Amount);")
    # A CPF_ConstParm without CPF_ReferenceParm is emitted by value (as UHT spells it). Promote mode adds
    # const& for replicated non-scalar parameters (rpc_arg).
    check("const value param is by value", fn["ClientTell"]["decl"] == "void ClientTell(FString Text);")
    # Service request out parameters get UPARAM(NotReplicated) in the header only; RPC parameters are unchanged.
    check("service request out parameter is UPARAM(NotReplicated)",
          header_arg({"service": "Request"}, "FClientUrlContext& Context") == "UPARAM(NotReplicated) FClientUrlContext& Context")
    check("service request const parameter unchanged",
          header_arg({"service": "Request"}, "const FString& Code") == "const FString& Code")
    check("RPC out parameter unchanged", header_arg({"service": None}, "FClientUrlContext& Context") == "FClientUrlContext& Context")
    check("undecoded flags reported", bool(fn["Unknown"]["todo"]) and fn["Unknown"]["flags"] is None)
    header = render_header(model, "SELFTEST_API", "self-test")
    cpp = render_cpp(model, "self-test")
    check("header UCLASS config", "UCLASS(config=Engine)" in header)
    check("header ReplicatedUsing UPROPERTY", "UPROPERTY(ReplicatedUsing=OnRep_Owner)" in header)
    check("header GetLifetimeReplicatedProps declared", "GetLifetimeReplicatedProps" in header)
    order = [cpp.index("DOREPLIFETIME(ASelfTest, %s)" % n) for n in ("Owner", "Role", "bHidden", "Pos", "Hits")]
    check("DOREPLIFETIME in table order", order == sorted(order))
    check("cpp calls Super when parent resolved", "Super::GetLifetimeReplicatedProps" in cpp)
    empty = build_class(dict(SELF_TEST_BLOCK, base_fn="0x9999"), "ANoParent", Resolver({}), "declaration")
    check("unresolved parent emits TODO token", empty["parent_expr"] == "TODO_Parent_9999")
    check("unresolved member emits TODO token",
          any("TODO_Class_2100" in m["ctype"] for m in empty["members_table_order"] if m["name"] == "Owner"))
    check("unresolved references recorded", any("not resolved" in t for t in empty["todos"]))
    check("unresolved parent blocks Super call",
          "Super::" not in render_cpp(empty, "self-test"))
    tiny = "\n".join([
        "class SELFTEST_API ASelfTest : public UObject",
        "{",
        "\tGENERATED_BODY()",
        "public:",
        "#if WITH_EDITORONLY_DATA",
        "\tUPROPERTY()",
        "\tint32 EditorOnly;",
        "#endif",
        "\tUPROPERTY(Replicated)",
        "\tuint8 bHidden:1;",
        "\tUPROPERTY(ReplicatedUsing=OnRep_Owner)",
        "\tAActor* Owner;",
        "\tUPROPERTY(Replicated)",
        "\tTEnumAsByte<enum ENetRole> Role;",
        "\tUFUNCTION(Server, Reliable)",
        "\tvoid ServerDo(int32 Amount);",
        "};",
    ])
    import tempfile
    with tempfile.TemporaryDirectory(prefix="gen_native_selftest_") as tmp:
        path = os.path.join(tmp, "tiny.h")
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(tiny)
        report = []
        result = compare_with_header(model, path, report)
    text = "\n".join(report)
    check("conditional property excluded from comparison", "conditional excluded: 1 (EditorOnly)" in text)
    check("no type mismatch on common names", result["type_mismatches"] == 0)
    check("no replication mismatch on common names", result["replication_mismatches"] == 0)
    check("no net-role mismatch on common UFUNCTION", result["net_role_mismatches"] == 0)
    check("reversed declaration order detected as mismatch", result["order_match"] is False)
    types_self_test(check)
    helper_self_test(check)
    field_self_test(check)
    static_reflected_self_test(check)
    if failures:
        print("SELF-TEST FAILED:", *failures, sep="\n  ")
        return 1
    print("self-test passed (%d checks)" % count[0])
    return 0


# ---------------------------------------------------------------- promotion into native/Source (first pass)
# --promote builds the closure listed in porting/native_class_set.txt. It emits only what resolves:
# a property whose type needs a struct, enum, delegate, interface or a class outside the set is skipped
# and recorded. Functions are emitted only when they are RPCs (wire-relevant) or RepNotify targets.

# Packages that are not in the UE 4.25 engine tree (or whose plugin is not enabled). Each one is generated as a runtime
# module named after the package, so the class path /Script/<Name>.<Class> is the original one. The module's Build.cs
# and IMPLEMENT_MODULE file are written by --promote (porting/native_classes_status.md, "Runtime modules").
RUNTIME_MODULE_PACKAGES = (
    "/Script/BlueprintContext", "/Script/MeshNetwork", "/Script/CommonUI", "/Script/AmbientAudio", "/Script/Account",
    "/Script/ModularGameplay", "/Script/McpProfileSys", "/Script/CommonInput", "/Script/FortniteUI",
    "/Script/SocialUMG", "/Script/KairosSceneCapture",   # stage 2 (coverage cause C1); see native_classes_status.md
)
HAND_WRITTEN_MODULES = ("FortniteGame", "PlayspaceFramework")   # Build.cs and IMPLEMENT_* files are not generated
PROMOTE_PACKAGES = ("/Script/FortniteGame", "/Script/PlayspaceFramework") + RUNTIME_MODULE_PACKAGES
# Runtime modules that a FortniteGame item may use the types of (TypeGate.cross_module_allowed). FortniteUI is not
# one: it depends on FortniteGame (its parents), so the reverse edge would be a module cycle.
CROSS_MODULE_TARGETS = tuple(p for p in RUNTIME_MODULE_PACKAGES if p != "/Script/FortniteUI")
# UHT needs an endpoint name on every ServiceRequest/ServiceResponse (HeaderParser.cpp). The binary's UFunction does
# not carry it and no generated code reads it (the request body is not recovered), so this is a placeholder.
NET_SERVICE_ENDPOINT = "UnrecoveredEndpoint"
# RPCResponseId of each ServiceResponse, read from FFunctionParams+70 in the runtime dump (not in the reflection tables).
# Every entry is a UFortGameInstance JS-bridge response. Its endpoint is JSBridge: UHT exempts that endpoint from the
# rule that a response id needs its request in the same class (HeaderParser.cpp, "RPCResponseId > 0"). Requests have
# RPCId 0, so they need no Id. The id is wire-relevant (UFunction::RPCResponseId).
NET_RESPONSE_IDS = {
    "CancelContentInstall": 1, "CancelUninstallAllOptionalContentOnNextBoot": 1,
    "CancelUninstallFortniteBROnNextBoot": 1, "CheckLatestClient": 1, "GetAnalyticsSessionDetails": 1,
    "GetContentState": 1, "GetFeatureRequiredSizes": 1, "GetFeatureRequiredSizesOptionalOnly": 1,
    "GetFeatureRequiredSizesRequiredOnly": 1, "GetHotfixHasApplied": 1, "GetProjectVersion": 1,
    "GetUserSetting": 1, "IsContentUpToDate": 1, "KairosSubmitBugReport": 1, "OpenUpdateApp": 1,
    "PauseContentInstall": 1, "RequestBroadcast": 1, "SetErrorSimulationCommands": 1,
    "SetFeatureStatusCallback": 1, "SetHotfixCompleteCallback": 1, "SetMultipleUserSettings": 1,
    "SetUserSetting": 1, "UninstallAllOptionalContentOnNextBoot": 1, "UninstallFortniteBROnNextBoot": 1,
}
ENGINE_PACKAGES = ("/Script/Engine", "/Script/CoreUObject")
FUNC_NET_ANY = FUNC_NET | FUNC_NET_SERVER | FUNC_NET_CLIENT | FUNC_NET_MULTICAST
ENGINE_CLASS_DECL_RE = re.compile(
    r"^\s*class\s+(?:[A-Z][A-Z0-9_]*_API\s+)?([AU][A-Za-z0-9_]*)\s*(?:final\s*)?:\s*public\b", re.M)


# Gate tokens written by Resolver.type_ref / TypeGate.check: TODO_<Kind><Reason>_<fn>.
GATE_CATEGORIES = (
    ("TODO_StructOtherModule_", "struct_other_module"),
    ("TODO_StructUnavailable_", "struct_not_available"),
    ("TODO_StructNotEmittable_", "struct_member_unresolved"),
    ("TODO_EnumOtherModule_", "enum_other_module"),
    ("TODO_EnumUnavailable_", "enum_not_available"),
    ("TODO_EnumNotEmittable_", "enum_member_unresolved"),
    ("TODO_DelegateOtherModule_", "delegate_other_module"),
    ("TODO_DelegateUnavailable_", "delegate_not_available"),
    ("TODO_DelegateNotEmittable_", "delegate_member_unresolved"),
    ("TODO_DelegateParams_", "delegate_params_unsupported"),
    ("TODO_DelegateSparse_", "delegate_sparse_macro"),
    ("TODO_StructNameClash_", "struct_name_clash"),
    ("TODO_DelegateNameClash_", "delegate_name_clash"),
)


def category_of(tokens):
    """Report category for the first TODO token (sorted), so skips group by what blocked them."""
    for token in sorted(tokens):
        for prefix, category in GATE_CATEGORIES:
            if token.startswith(prefix):
                return category
        if token.startswith("TODO_ClassOutsideNativeSet_"):
            return "class_outside_native_set"
        if token.startswith(("TODO_Class_", "TODO_MetaClass_", "TODO_SoftClass_")):
            return "class_unnamed"
        if token.startswith("TODO_Struct_"):
            return "struct"
        if token.startswith("TODO_Enum_"):
            return "enum"
        if token.startswith("TODO_Delegate_"):
            return "delegate"
        if token.startswith("TODO_Interface_"):
            return "interface"
        if token.startswith(("TODO_ArrayElem_", "TODO_SetElem_", "TODO_MapKey_", "TODO_MapValue_")):
            return "container_element"
        match = re.match(r"TODO_Kind(\w+?)_", token)
        if match:
            return "kind_" + match.group(1)
        return "other"
    return "other"


def skip_record(item, entry, name, category, reason, tokens):
    entry = entry or {}
    flags = hx(entry.get("property_flags")) if entry.get("property_flags") else 0
    return {"item": item, "name": name, "kind": entry.get("kind"), "property_flags": entry.get("property_flags"),
            "replicated": bool(flags & CPF_NET), "category": category, "reason": reason,
            "tokens": sorted(set(tokens))}


def skip_function(fn, category, reason, tokens=()):
    return {"item": "function", "name": fn.get("name"), "category": category, "reason": reason,
            "flags": fn.get("flags"), "net_roles": fn.get("net_roles"), "tokens": sorted(set(tokens))}


def net_service_kind(flags):
    """'Request' for FUNC_NetRequest and 'Response' for FUNC_NetResponse: UE net-service functions (UHT
    ServiceRequest / ServiceResponse). They carry FUNC_Net but no Server/Client/NetMulticast role, so they are not
    RPCs of those kinds. None for any other flags (including a role, which takes precedence)."""
    if flags & (FUNC_NET_SERVER | FUNC_NET_CLIENT | FUNC_NET_MULTICAST):
        return None
    if flags & FUNC_NET_REQUEST:
        return "Request"
    if flags & FUNC_NET_RESPONSE:
        return "Response"
    return None


def function_specifiers(flags):
    specs = []
    if flags & FUNC_NET_SERVER:
        specs.append("Server")
    if flags & FUNC_NET_CLIENT:
        specs.append("Client")
    if flags & FUNC_NET_MULTICAST:
        specs.append("NetMulticast")
    # UHT requires an explicit delivery mode on every replicated function.
    specs.append("Reliable" if flags & FUNC_NET_RELIABLE else "Unreliable")
    return specs


SCALAR_ARG_PREFIXES = ("bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64",
                       "float", "double", "TEnumAsByte<")


def rpc_arg(decl, enum_args=()):
    """UHT: a replicated function's non-scalar parameter must be passed by const reference. Enums are scalars."""
    ctype, name = decl.rsplit(" ", 1)
    if decl in enum_args or ctype.endswith(("&", "*")) or ctype.startswith(SCALAR_ARG_PREFIXES):
        return decl
    # UHT copies a const reference parameter into the Parms struct as a const member, which cannot be assigned
    # (the thunk does Parms.X = X). A class parameter is passed by value; its property type is unchanged.
    if ctype.startswith(("TSubclassOf<", "TSoftClassPtr<")):
        return decl
    return "const %s& %s" % (ctype, name)


def header_arg(f, arg):
    """A service request's out parameter (`T& Context`, reflection: CPF_OutParm + CPF_RepSkip, no CPF_ReferenceParm)
    is a non-const reference on a net function, which UHT rejects ("Replicated ... cannot be passed by non-const
    reference"). UPARAM(NotReplicated) makes UHT treat it as an unreplicated out parameter (UHT's own rule for
    service requests, HeaderParser.cpp "unless marked NotReplicated"), which matches the original flags. Header only."""
    ctype = arg.rsplit(" ", 1)[0]
    if f.get("service") == "Request" and ctype.endswith("&") and not ctype.startswith("const "):
        return "UPARAM(NotReplicated) " + arg
    return arg


def function_view(fn, owner_fns):
    """(flags, params, problem) for one function. An ambiguous name join (fn["candidates"]) is resolved to
    the single candidate whose outer_fn is the construct function of this class (owner_fns)."""
    if fn.get("candidates"):
        owned = [c for c in fn["candidates"] if str(c.get("outer_fn") or "").lower() in owner_fns]
        if len(owned) != 1:
            return None, None, "%d of %d candidate FFunctionParams belong to this class (need 1)" % (
                len(owned), len(fn["candidates"]))
        return hx(owned[0]["flags"]), owned[0].get("params"), None
    flag_values = fn.get("flags") or []
    flags = hx(flag_values[0]) if len(flag_values) == 1 else None
    if flags is None:
        return None, None, "flags not joined (%s)" % (fn.get("note") or "ambiguous")
    if fn.get("params") is None:
        return flags, None, "parameter table not decoded (%s)" % (fn.get("params_note") or "no FFunctionParams")
    return flags, fn["params"], None


# Narrow, observed cooked Blueprint imports. Expand only with matching reflection and runtime evidence.
UI_BLUEPRINT_EVENTS = {"UCommonButton": frozenset({"BP_OnHovered", "BP_OnSelected"})}
UI_BLUEPRINT_EVENT_FLAGS = 0x08080800  # BlueprintEvent | Protected | Event; not Native

# Parameterized events require an individually evidenced owner, function,
# complete serialized signature and explicitly reviewed UHT declaration.
UI_PARAMETERIZED_BLUEPRINT_EVENTS = {
    "UFortSplashScreenWidget": {
        "OnInputMethodChanged": {
            "owner_fn": "0x1452e3dd0", "create_func": "0x1452e3e00", "flags": 0x08080800,
            "params": [
                {"index": 0, "name": "InputType", "kind": "Enum", "kind_raw": "0x1e",
                 "property_flags": "0x10000000000080", "replicated": False, "rep_notify": None,
                 "array_dim": 1, "object_flags": "0x45", "offset": 0, "type_refs": {"enum": "0x14081ee20"}},
                {"index": 1, "name": "UnderlyingType", "kind": "Byte", "kind_raw": "0x0",
                 "property_flags": "0x0", "replicated": False, "rep_notify": None,
                 "array_dim": 1, "object_flags": "0x45", "offset": 0, "enum_underlying_of": "InputType"},
            ],
            "args": ["ECommonInputType InputType"], "specifiers": ["BlueprintImplementableEvent"],
            "access": "protected", "resolve_args": True,
            "allowed_type_packages": {"0x14081ee20": "/Script/CommonInput"},
        },
    },
    "AFortEntryMusicController": {
        "StartMusic": {
            "owner_fn": "0x1428dc600", "create_func": "0x1428dc630", "flags": 0x08020800,
            "params": [], "args": [], "specifiers": ["BlueprintImplementableEvent"], "access": "public",
        },
    },
    "UFortLoginStatus": {
        "SetContinueButtonText": {
            "owner_fn": "0x145282560", "create_func": "0x1452825a0", "flags": 0x08480800,
            "params": [{"index": 0, "name": "ContinueText", "kind": "Text", "kind_raw": "0x1d",
                        "property_flags": "0x10000008000182", "replicated": False, "rep_notify": None,
                        "array_dim": 1, "object_flags": "0x45", "offset": 0}],
            "args": ["const FText& ContinueText"], "specifiers": ["BlueprintImplementableEvent"], "access": "protected",
        },
    },
}


# Original class-link and FFunctionParams evidence: frontend/parameterless_super_events_evidence.md.
OBSERVED_PARAMETERLESS_SUPER_EVENTS = (
    ('UCommonActivatableWidget', 'BP_OnActivated', '0x1408079b0', '0x140805d90', 0x08080800, 'protected'),
    ('UCommonButton', 'BP_OnClicked', '0x140805cb0', '0x140806710', 0x08080800, 'protected'),
    ('UCommonButton', 'BP_OnDisabled', '0x140805cb0', '0x1408067a0', 0x08080800, 'protected'),
    ('UCommonButton', 'BP_OnEnabled', '0x140805cb0', '0x140806800', 0x08080800, 'protected'),
    ('UCommonButton', 'BP_OnUnhovered', '0x140805cb0', '0x140806890', 0x08080800, 'protected'),
    ('UCommonButton', 'OnActionComplete', '0x140805cb0', '0x140806e40', 0x08080800, 'protected'),
    ('UCommonButton', 'OnCurrentTextStyleChanged', '0x140805cb0', '0x140806ea0', 0x08080800, 'protected'),
    ('UEmergencyNoticeBase', 'HideNotice', '0x1451e6f80', '0x1451e7070', 0x08080800, 'protected'),
    ('UFortFriendSubscriptionToastWidget', 'OnPlayCloseAnimation', '0x145235570', '0x145235650', 0x08080800, 'protected'),
    ('UFortFriendSubscriptionToastWidget', 'OnPlayOpenAnimation', '0x145235570', '0x145235680', 0x08080800, 'protected'),
    ('UFortPlayerPowerRatingToast', 'ShowToast', '0x1452a12b0', '0x1452a13d0', 0x08080800, 'protected'),
    ('UFortVoiceChannelChangePopup', 'OnPlayCloseAnimation', '0x14531fef0', '0x1453202a0', 0x08080800, 'protected'),
    ('UFortVoiceChannelChangePopup', 'OnPlayOpenAnimation', '0x14531fef0', '0x145320310', 0x08080800, 'protected'),
    ('UFortWebLoginWidget', 'DismissWidget', '0x1453208a0', '0x1453208d0', 0x0c020800, 'public'),
)
for _owner, _name, _outer, _create, _flags, _access in OBSERVED_PARAMETERLESS_SUPER_EVENTS:
    UI_PARAMETERIZED_BLUEPRINT_EVENTS.setdefault(_owner, {})[_name] = {
        "owner_fn": _outer, "create_func": _create, "flags": _flags, "params": [], "args": [],
        "specifiers": ["BlueprintImplementableEvent"] + (["BlueprintCallable"] if _flags & 0x04000000 else []),
        "access": _access,
    }


# Source-evidenced typed Blueprint supers; exact serialized arrays remain the guard.
OBSERVED_TYPED_SUPER_EVENTS = (
    ('UCommonButton',
     'OnActionProgress',
     '0x140805cb0',
     '0x140806e70',
     134744064,
     'protected',
     [{'index': 0,
       'name': 'HeldPercent',
       'kind': 'Float',
       'kind_raw': '0xa',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0}],
     ['float'],
     ['float HeldPercent'],
     {}),
    ('UCommonButton',
     'OnTriggeredInputActionChanged',
     '0x140805cb0',
     '0x140806fc0',
     138938368,
     'protected',
     [{'index': 0,
       'name': 'NewTriggeredAction',
       'kind': 'Struct',
       'kind_raw': '0x19',
       'property_flags': '0x10000008000182',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'type_refs': {'struct': '0x1446f2880'}}],
     ['FDataTableRowHandle'],
     ['const FDataTableRowHandle& NewTriggeredAction'],
     {}),
    ('UEmergencyNoticeBase',
     'ShowNotice',
     '0x1451e6f80',
     '0x1451e70a0',
     138938368,
     'protected',
     [{'index': 0,
       'name': 'Body',
       'kind': 'Text',
       'kind_raw': '0x1d',
       'property_flags': '0x10000008000182',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 24},
      {'index': 1,
       'name': 'Title',
       'kind': 'Text',
       'kind_raw': '0x1d',
       'property_flags': '0x10000008000182',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0}],
     ['FText', 'FText'],
     ['const FText& Title', 'const FText& Body'],
     {}),
    ('UFortConfirmationWindow',
     'ConfigureConfirmationButton',
     '0x145215cf0',
     '0x145215d90',
     138938368,
     'protected',
     [{'index': 0,
       'name': 'bSimpleConfirm',
       'kind': 'Bool',
       'kind_raw': '0x2c',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'element_size': 1,
       'size_of_outer': 192,
       'bitfield': True,
       'set_bit_fn': '0x140746280',
       'offset': None},
      {'index': 1,
       'name': 'Action',
       'kind': 'Struct',
       'kind_raw': '0x19',
       'property_flags': '0x10000008000182',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 8,
       'type_refs': {'struct': '0x142b25040'}},
      {'index': 2,
       'name': 'Button',
       'kind': 'Object',
       'kind_raw': '0x12',
       'property_flags': '0x10000000080080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'type_refs': {'class': '0x140807800'}}],
     ['bool', 'FConfirmationDialogAction', 'UCommonButton*'],
     ['UCommonButton* Button', 'const FConfirmationDialogAction& Action', 'bool bSimpleConfirm'],
     {'0x142b25040': '/Script/FortniteGame'}),
    ('UFortConfirmationWindow',
     'ConfigureDeclineButton',
     '0x145215cf0',
     '0x145215dc0',
     134744064,
     'protected',
     [{'index': 0,
       'name': 'Button',
       'kind': 'Object',
       'kind_raw': '0x12',
       'property_flags': '0x10000000080080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'type_refs': {'class': '0x140807800'}}],
     ['UCommonButton*'],
     ['UCommonButton* Button'],
     {}),
    ('UFortErrorWindow',
     'OnOverrideCloseButtonText',
     '0x14522c380',
     '0x14522c3b0',
     138938368,
     'protected',
     [{'index': 0,
       'name': 'OverrideText',
       'kind': 'Text',
       'kind_raw': '0x1d',
       'property_flags': '0x10000008000182',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0}],
     ['FText'],
     ['const FText& OverrideText'],
     {}),
    ('UFortLetoLayoutBorder',
     'UpdateBorderWidth',
     '0x1452c3e10',
     '0x1452c3ea0',
     134744064,
     'protected',
     [{'index': 0,
       'name': 'HalfBorderWidth',
       'kind': 'Float',
       'kind_raw': '0xa',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0}],
     ['float'],
     ['float HalfBorderWidth'],
     {}),
    ('UFortRestartClientModal',
     'OnButtonSet',
     '0x1452bfd00',
     '0x1452bfd60',
     134744064,
     'protected',
     [{'index': 0,
       'name': 'bShouldShow',
       'kind': 'Bool',
       'kind_raw': '0x2c',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'element_size': 1,
       'size_of_outer': 2,
       'bitfield': True,
       'set_bit_fn': '0x1407579e0',
       'offset': None},
      {'index': 1,
       'name': 'InbShouldExit',
       'kind': 'Bool',
       'kind_raw': '0x2c',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'element_size': 1,
       'size_of_outer': 2,
       'bitfield': True,
       'set_bit_fn': '0x1406966a0',
       'offset': None}],
     ['bool', 'bool'],
     ['bool InbShouldExit', 'bool bShouldShow'],
     {}),
    ('UFortToastDisplayArea',
     'OnDisplayToast',
     '0x145300cb0',
     '0x145300da0',
     134350848,
     'public',
     [{'index': 0,
       'name': 'NewToast',
       'kind': 'Object',
       'kind_raw': '0x12',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'type_refs': {'class': '0x142b26c60'}}],
     ['UFortUINotification*'],
     ['UFortUINotification* NewToast'],
     {}),
    ('UFortVoiceChannelChangePopup',
     'OnInputModeChanged',
     '0x14531fef0',
     '0x145320240',
     134744064,
     'protected',
     [{'index': 0,
       'name': 'NewInputType',
       'kind': 'Enum',
       'kind_raw': '0x1e',
       'property_flags': '0x10000000000080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'type_refs': {'enum': '0x14081ee20'}},
      {'index': 1,
       'name': 'UnderlyingType',
       'kind': 'Byte',
       'kind_raw': '0x0',
       'property_flags': '0x0',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'enum_underlying_of': 'NewInputType'}],
     ['ECommonInputType'],
     ['ECommonInputType NewInputType'],
     {'0x14081ee20': '/Script/CommonInput'}),
    ('UFortWebLoginWidget',
     'DisplayWidget',
     '0x1453208a0',
     '0x145320900',
     201459712,
     'public',
     [{'index': 0,
       'name': 'WebWidget',
       'kind': 'Object',
       'kind_raw': '0x12',
       'property_flags': '0x10000000080080',
       'replicated': False,
       'rep_notify': None,
       'array_dim': 1,
       'object_flags': '0x45',
       'offset': 0,
       'type_refs': {'class': '0x143dc3650'}}],
     ['UWidget*'],
     ['UWidget* WebWidget'],
     {}),
)
for _owner, _name, _outer, _create, _flags, _access, _params, _types, _args, _allowed in OBSERVED_TYPED_SUPER_EVENTS:
    UI_PARAMETERIZED_BLUEPRINT_EVENTS.setdefault(_owner, {})[_name] = {
        "owner_fn": _outer, "create_func": _create, "flags": _flags, "params": _params, "args": _args,
        "specifiers": ["BlueprintImplementableEvent"] + (["BlueprintCallable"] if _flags & 0x04000000 else []),
        "access": _access, "checked_types": _types, "allowed_type_packages": _allowed,
    }


def build_functions_promoted(functions, resolver, notify_names, owner_fns, splice_names=frozenset(),
                             blueprint_events=frozenset(), parameterized_blueprint_events=None):
    """Emit RPCs, RepNotify targets, snippet functions and narrowly selected observed Blueprint events.
    A function whose flags cannot be classified is recorded, since it might be an RPC."""
    emitted = []
    for fn in functions:
        name = fn.get("name") or "UnnamedFunction"
        flags, params, problem = function_view(fn, owner_fns)
        if flags is None:
            resolver.skipped.append(skip_function(fn, "flags_ambiguous", "RPC status unknown: " + problem))
            continue
        is_rpc = bool(flags & FUNC_NET_ANY)
        is_splice = name in splice_names and not is_rpc
        policy = (parameterized_blueprint_events or {}).get(name)
        is_blueprint_event = name in blueprint_events or policy is not None
        if policy is not None:
            owned = [c for c in fn.get("candidates", []) if str(c.get("outer_fn") or "").lower() in owner_fns]
            joined = owned[0] if len(owned) == 1 else fn if not fn.get("candidates") else None
            if (flags != policy["flags"] or is_splice or name in notify_names or
                    policy["owner_fn"] not in owner_fns or str(fn.get("create_func") or "").lower() != policy["create_func"] or
                    joined is None or str(joined.get("outer_fn") or "").lower() != policy["owner_fn"] or
                    (fn.get("candidates") and joined.get("nprop") != len(policy["params"])) or
                    params != policy["params"] or policy.get("access") not in ("public", "protected")):
                resolver.skipped.append(skip_function(fn, "blueprint_event_mismatch",
                    "original parameterized event owner, flags or exact signature differs"))
                continue
            used = {}
            if policy.get("resolve_args"):
                mark = len(resolver.tokens)
                resolver.wire_used = False
                resolver.used = {}
                resolver.observed_event_type_packages = policy.get("allowed_type_packages", {})
                try:
                    ret, args = decode_params(params, resolver)
                finally:
                    resolver.observed_event_type_packages = {}
                used = resolver.used
                for ref in policy.get("allowed_type_packages", {}):
                    if ref in used:
                        used[ref].add("ObservedBlueprintEventType")
                if resolver.tokens[mark:] or ret != "void" or args != policy["args"]:
                    resolver.skipped.append(skip_function(fn, "blueprint_event_mismatch",
                        "original Blueprint event parameter types could not be resolved exactly"))
                    continue
            if "checked_types" in policy:
                mark = len(resolver.tokens)
                resolver.wire_used = False
                resolver.used = {}
                resolver.observed_event_type_packages = policy.get("allowed_type_packages", {})
                try:
                    pairs, notes = split_properties(params)
                    types = [resolver.ctype(entry, children) for entry, children in pairs]
                finally:
                    resolver.observed_event_type_packages = {}
                used = resolver.used
                for ref in policy.get("allowed_type_packages", {}):
                    if ref in used:
                        used[ref].add("ObservedBlueprintEventType")
                if notes or resolver.tokens[mark:] or types != policy["checked_types"]:
                    resolver.skipped.append(skip_function(fn, "blueprint_event_mismatch",
                        "original Blueprint event parameter types could not be resolved exactly"))
                    continue
            emitted.append({"name": name, "rpc": False, "service": None, "ret": "void",
                            "const": False, "notify": False, "splice": False,
                            "blueprint_event": True, "access": policy["access"], "specifiers": list(policy["specifiers"]),
                            "args": list(policy["args"]), "used": used})
            continue
        if is_blueprint_event:
            # The imported events have no parameters and exactly these flags. Refuse to invent
            # a signature or native implementation if the reflection join changes.
            if flags != UI_BLUEPRINT_EVENT_FLAGS or params != [] or is_splice or name in notify_names:
                resolver.skipped.append(skip_function(fn, "blueprint_event_mismatch",
                    "expected protected non-native Blueprint event with no parameters"))
                continue
            emitted.append({"name": name, "rpc": False, "service": None, "ret": "void",
                            "const": False, "notify": False, "splice": False,
                            "blueprint_event": True, "specifiers": ["BlueprintImplementableEvent"],
                            "args": [], "used": {}})
            continue
        if not is_rpc and name not in notify_names and not is_splice:
            continue
        if name in splice_names and is_rpc:
            resolver.skipped.append(skip_function(fn, "snippet_on_rpc", "a getter/exec snippet names an RPC"))
            continue
        if is_splice and flags & FUNC_STATIC and flags & FUNC_CONST:
            resolver.skipped.append(skip_function(fn, "snippet_static_const", "static function cannot be const"))
            continue
        service = net_service_kind(flags) if is_rpc else None
        if is_rpc and service is None and not flags & (FUNC_NET_SERVER | FUNC_NET_CLIENT | FUNC_NET_MULTICAST):
            resolver.skipped.append(skip_function(fn, "net_flag_without_role",
                                                  "net flag without a Server/Client/NetMulticast or net service role"))
            continue
        if is_rpc and flags & (FUNC_NET_REQUEST | FUNC_NET_RESPONSE) and service is None:
            resolver.skipped.append(skip_function(fn, "net_service_with_role",
                                                  "net service flags with a Server/Client/NetMulticast role"))
            continue
        # A const RPC is emitted: UHT only sets FUNC_Const for it (the flag is part of the function checksum).
        if is_rpc and flags & FUNC_STATIC:
            resolver.skipped.append(skip_function(fn, "static_rpc", "static RPC is not emitted"))
            continue
        if params is None:
            resolver.skipped.append(skip_function(fn, "params_not_decoded", problem))
            continue
        mark = len(resolver.tokens)
        resolver.wire_used = False
        resolver.used = {}
        enum_args = []
        ret, args = decode_params(params, resolver, enum_args)
        used = resolver.used
        new_tokens = resolver.tokens[mark:]
        if new_tokens:
            resolver.skipped.append(skip_function(fn, category_of(new_tokens),
                                                  "unresolved parameter type: " + ", ".join(sorted(set(new_tokens))),
                                                  new_tokens))
            continue
        # An RPC parameter struct with a custom NetSerialize is emitted (see build_members for the fallback).
        # A net service function may return a value (UHT: the replicated-return check excludes net services).
        if ret != "void" and not is_splice and service is None:
            resolver.skipped.append(skip_function(fn, "non_void_return", "return type %s is not emitted" % ret))
            continue
        if service == "Response":
            # UHT treats a response like any replicated function here: no out, reference or delegate parameters.
            if any(a.endswith("&") and not a.startswith("const ") for a in args) or any(
                    "Delegate" in str(p.get("kind")) for p in params):
                resolver.skipped.append(skip_function(fn, "net_response_parameter",
                                                      "net response with an out, reference or delegate parameter"))
                continue
        if is_rpc:
            args = [rpc_arg(arg, enum_args) for arg in args]
        response_id = NET_RESPONSE_IDS.get(name, 0) if service == "Response" else 0
        if service == "Request":
            specifiers = ["ServiceRequest(%s)" % NET_SERVICE_ENDPOINT]
        elif response_id:
            specifiers = ["ServiceResponse(JSBridge, ResponseId=%d)" % response_id]
        elif service:
            specifiers = ["ServiceResponse(%s)" % NET_SERVICE_ENDPOINT]
        else:
            specifiers = function_specifiers(flags) if is_rpc else []
        rec = {"name": name, "rpc": is_rpc, "service": service, "ret": ret,
               "static": bool(flags & FUNC_STATIC),
               "const": bool(flags & FUNC_CONST) and (is_rpc or is_splice),
               "notify": name in notify_names, "splice": is_splice,
               "specifiers": specifiers, "args": args, "used": used}
        if is_splice:
            # A snippet makes a non-RPC function a BlueprintCallable member; BlueprintPure follows the reflection flag.
            rec["specifiers"] = ["BlueprintCallable"] + (["BlueprintPure"] if flags & FUNC_BLUEPRINT_PURE else [])
        emitted.append(rec)
    return emitted


def build_class_promoted(block, name, resolver, parent_entry, owner_fns, snippets=(), splice_ctx=None):
    resolver.todos = []
    resolver.class_refs = set()
    resolver.skipped = []
    members, _notes = build_members(block.get("properties") or [], resolver, promote=True)
    notify_names = {m["rep_notify"] for m in members if m["rep_notify"]}
    splice_names = {e["function"] for e in snippets if e["kind"] in ("getter", "exec") and e.get("body") is not None}
    functions = build_functions_promoted(block.get("functions") or [], resolver, notify_names, owner_fns,
                                         splice_names, UI_BLUEPRINT_EVENTS.get(name, frozenset()),
                                         UI_PARAMETERIZED_BLUEPRINT_EVENTS.get(name, {}))
    splice = (attach_snippets(name, functions, snippets, splice_ctx, parent_entry.get("package"), resolver.skipped,
                              {m["name"] for m in members})
              if snippets else None)
    if splice is not None:
        functions = [f for f in functions if not (f["splice"] and not f.get("accepted"))]
    emitted_onrep = {f["name"] for f in functions if not f["rpc"] and f.get("notify")}
    for m in members:
        if m["rep_notify"] and m["rep_notify"] not in emitted_onrep:
            resolver.skipped.append(skip_record("rep_notify", None, m["name"], "rep_notify_dropped",
                                                "RepNotify %s not emitted; property kept as Replicated" % m["rep_notify"], []))
            m["specifiers"] = ["Replicated" if s.startswith("ReplicatedUsing=") else s for s in m["specifiers"]]
            m["rep_notify"] = None
    parent = parent_entry.get("parent_name")
    return {
        "name": name,
        "block_va": block.get("block_va"),
        "parent": parent,
        "package": parent_entry.get("package"),
        "uclass": uclass_line(block, minimal_api=False),
        "members_table_order": members,
        "members_decl_order": list(reversed(members)),
        "functions": functions,
        "splice": splice or {"accepted": [], "rejected": []},
        "cpp_includes": sorted(splice["includes"]) if splice else [],
        "helpers": (splice or {}).get("helpers", []),
        "static_helpers": (splice or {}).get("static_helpers", []),
        "fields": (splice or {}).get("fields", []),
        "forward_decls": sorted(ref for ref in (set(resolver.class_refs) | (splice or {}).get("forward_decls", set()))
                                if ref not in (name, parent)),
        "used": merge_used([m["used"] for m in members] + [f["used"] for f in functions]),
        "skipped": list(resolver.skipped),
        "functions_in_table": len(block.get("functions") or []),
        "rpc_in_reflection": sum(1 for f in (block.get("functions") or [])
                                 if function_view(f, owner_fns)[0] is not None
                                 and function_view(f, owner_fns)[0] & FUNC_NET_ANY),
    }


# ---------------------------------------------------------------- behaviour snippets (porting/behavior)
# The ported function bodies of porting/behavior/MANIFEST.json (contract: porting/behavior/README.md). Per entry:
#   getter, exec   non-RPC function: declared as UFUNCTION(BlueprintCallable[, BlueprintPure]) and given the body;
#   validate       <Rpc>_Validate: the Server RPC gets WithValidation, <Rpc>_Validate is declared and given the body;
#   impl           <Rpc>_Implementation: the body replaces the stub;
#   onrep          RepNotify function: the body replaces the stub.
# An entry is spliced only when its signature equals the reflection-derived one (squash) and its body needs only
# includes the generator can give. Otherwise it is rejected: nothing is emitted for it and it is listed in
# behavior_splice.json. FN_BEHAVIOR_SPLICE=0 (or --no-behavior) turns the splice off and reproduces the stub output.

BEHAVIOR_DEFAULT = os.path.join(REPO, "porting", "behavior")
FUNC_BLUEPRINT_PURE = 0x10000000
BEHAVIOR_BAD_RE = re.compile(r"\b(TODO|FIXME|TBD|XXX|PLACEHOLDER|placeholder)\b|not recovered|stub body", re.I)
STUB_BODY = "// Body not recovered: reflection-derived stub."


def behavior_enabled(args):
    if getattr(args, "no_behavior", False):
        return False
    return os.environ.get("FN_BEHAVIOR_SPLICE", "1").strip().lower() not in ("0", "off", "false", "no")


def squash(text):
    """Signature key: all whitespace removed. Names, types, const and parameter lists must still agree."""
    return re.sub(r"\s+", "", text or "").rstrip(";")


def read_snippet(entry):
    """(body, None) for a usable .inc, or (None, reason). The body is the text between the braces (comments kept).
    inc_path in MANIFEST.json is relative to the repository root."""
    rel = entry.get("inc_path") or ""
    path = os.path.join(REPO, rel) if rel else ""
    if not rel or not os.path.isfile(path):
        return None, "snippet file missing: %s" % (rel or "(no inc_path)")
    with open(path, "r", encoding="utf-8") as fh:
        text = fh.read()
    if "\r" in text:
        return None, "CRLF line endings in snippet"
    if any(line.lstrip().startswith("#") for line in text.splitlines()):
        return None, "preprocessor line in snippet (the generator adds includes)"
    if BEHAVIOR_BAD_RE.search(text):
        return None, "placeholder text in snippet"
    code = strip_comments(text)
    if code.count("{") != code.count("}"):
        return None, "unbalanced braces in snippet"
    if not re.search(r"CONFIDENCE:\s*(high|medium|low)\b", text, re.I):
        return None, "snippet has no CONFIDENCE tag"
    return text.strip("\n"), None


def load_behavior(behavior_dir):
    """Ported entries of MANIFEST.json, each with its body (or the reason its body is unusable)."""
    path = os.path.join(behavior_dir, "MANIFEST.json")
    if not os.path.isfile(path):
        raise SystemExit("behaviour manifest not found: %s" % path)
    out = []
    for entry in load_json(path):
        if entry.get("status") != "ported":
            continue
        entry = dict(entry)
        entry["body"], entry["reject"] = read_snippet(entry)
        out.append(entry)
    return out


def body_lines(body):
    """Function body lines as emitted: one tab of indent, or the stub line when no snippet is spliced."""
    if body is None:
        return ["\t" + STUB_BODY]
    return [("\t" + line.rstrip()) if line.strip() else "" for line in body.split("\n")]


def body_refs(body, name, package, ctx):
    """(includes, problems) for the names a snippet body uses. A class or type of another module cannot be included
    from this module, so a body that uses one is rejected."""
    includes, problems = set(), []
    for tok in sorted(set(re.findall(r"[A-Za-z_]\w*", strip_comments(body)))):
        if tok == name:
            continue
        if tok in ctx["class_pkg"]:
            if ctx["class_pkg"][tok] != package:
                problems.append("body uses %s, a class of another module" % tok)
            else:
                includes.add("%s.h" % tok)
        elif tok in ctx["struct_index"]:
            info = ctx["struct_index"][tok]
            if info.package in PROMOTE_PACKAGES:
                if info.package != package:
                    problems.append("body uses %s, a type of another module" % tok)
                else:
                    includes.add(info.header_name)
            else:
                inc = ctx["gate"].include_of(info.fn)
                if inc is None:
                    problems.append("body uses engine type %s with no include" % tok)
                else:
                    includes.add(inc)
        elif tok in ctx["other_promoted"]:
            problems.append("body uses %s, which is not in the native class set" % tok)
    return includes, problems


def not_emitted_reason(skipped, fname):
    """Why a function of the reflection table is not emitted (its skip record), for a rejection message."""
    for rec in skipped:
        if rec.get("item") == "function" and rec.get("name") == fname:
            return "%s (%s)" % (rec.get("category"), rec.get("reason"))
    return "not in the reflection table"


# ---------------------------------------------------------------- helpers (non-reflected functions, porting/behavior)
# kind "helper": a plain C++ member function. Its declaration goes into the header's "Non-reflected helpers" block,
# after the reflected functions and GetLifetimeReplicatedProps and outside any UFUNCTION declaration, with the
# entry's access. Its definition goes into the .cpp after the reflected functions.
# kind "static_helper": a static free function in an anonymous namespace at the top of the class .cpp. It has no header
# declaration and every member definition comes after it. Contract: porting/behavior/README.md, "helper".
HELPER_KINDS = ("helper", "static_helper")
HELPER_ACCESS = ("public", "protected")
HELPER_SIG_RE = re.compile(r"^(?P<ret>.+?)\s*\b(?P<name>[A-Za-z_]\w*)\s*\((?P<args>[^()]*)\)\s*(?P<const>const)?$")
HELPER_TYPE_WORDS = frozenset(("void", "bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32",
                               "uint64", "float", "double", "const", "unsigned", "signed", "char", "TArray", "TMap",
                               "TSet", "TSubclassOf", "TWeakObjectPtr", "TSoftObjectPtr", "TSoftClassPtr",
                               "TEnumAsByte", "FString", "FName", "FText", "FGuid"))
TYPE_NAME_RE = re.compile(r"(?<![\w:])[A-Za-z_]\w*")   # an identifier that is not the member part of A::B


def split_params(text):
    """Top-level comma split of a parameter list; commas inside <...> do not split."""
    parts, depth, cur = [], 0, ""
    for ch in text:
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur.strip())
    return parts


def parse_helper_signature(sig, static):
    """(ret, name, params, const) of a helper signature: a C++ declaration without ';' and without 'static'.
    params are 'type name' strings in declaration order. Raises ValueError when the text is not one."""
    text = re.sub(r"\s+", " ", sig or "").strip().rstrip(";").strip()
    m = HELPER_SIG_RE.match(text)
    if not m:
        raise ValueError("signature is not 'ret name(params) [const]': %r" % (sig,))
    ret, name = m.group("ret").strip(), m.group("name")
    const = m.group("const") is not None
    if re.match(r"(static|virtual|inline|FORCEINLINE)\b", ret):
        raise ValueError("signature must not spell %s: the kind sets the linkage" % ret.split()[0])
    params = split_params(m.group("args"))
    for p in params:
        if "=" in p or not re.match(r"^.+[\s*&]\s*[A-Za-z_]\w*$", p):
            raise ValueError("parameter needs a type and a name: %r" % (p,))
    if static and const:
        raise ValueError("a static helper is a free function and cannot be const")
    return ret, name, params, const


def param_type(param):
    """The type text of a 'type name' parameter (the name is removed)."""
    return re.sub(r"(?<=[\s*&>])[A-Za-z_]\w*$", "", param).strip()


def helper_type_names(ret, params):
    """(names, texts): the identifiers of the return and parameter types, and the type texts."""
    texts = [ret] + [param_type(p) for p in params]
    names = set()
    for t in texts:
        names |= set(TYPE_NAME_RE.findall(t))
    return names, texts


def usable_include(inc, ctx):
    """True for an include spelling the engine header index knows (headers of usable modules)."""
    return isinstance(inc, str) and inc.endswith(".h") and inc in ctx["gate"].engine.module_of_inc


def type_include(tok, package, ctx):
    """(include, problem) for a struct or enum name of ctx["struct_index"] used by a helper signature."""
    info = ctx["struct_index"][tok]
    if info.package in PROMOTE_PACKAGES:
        if info.package != package:
            return None, "uses %s, a type of another module" % tok
        return info.header_name, None
    inc = ctx["gate"].include_of(info.fn)
    if inc is None:
        return None, "uses engine type %s with no include" % tok
    return inc, None


def helper_type_refs(names, texts, package, ctx, explicit):
    """(includes, forward_decls, problems) for the names of a helper signature. A class of this module is forward
    declared and must be used by pointer or reference. A struct or enum of a promoted module is included. A name the
    generator does not know needs an explicit includes entry in the manifest entry."""
    joined = " ".join(texts)
    includes, fwd, problems, unknown = set(), set(), [], []
    for tok in sorted(names):
        if tok in HELPER_TYPE_WORDS:
            continue
        if tok in ctx["class_pkg"]:
            if ctx["class_pkg"][tok] != package:
                problems.append("signature uses %s, a class of another module" % tok)
            elif not re.search(r"\b%s\b\s*[*&]" % re.escape(tok), joined):
                problems.append("signature uses class %s by value (pointer or reference needed)" % tok)
            else:
                fwd.add(tok)
        elif tok in ctx["struct_index"]:
            inc, problem = type_include(tok, package, ctx)
            if problem:
                problems.append("signature " + problem)
            else:
                includes.add(inc)
        else:
            unknown.append(tok)
    if unknown and not explicit:
        problems.append("signature type(s) %s are not known: list their headers in includes" % ", ".join(unknown))
    return includes, fwd, problems


def static_order(nodes, deps):
    """Dependencies-first order of nodes (deps: node -> set of names). Returns (ordered, left): left holds the nodes
    that are in or behind a dependency cycle."""
    ordered, remaining = [], set(nodes)
    while True:
        ready = sorted(n for n in remaining if not (deps[n] & remaining))
        if not ready:
            return ordered, remaining
        ordered.extend(ready)
        remaining -= set(ready)


def check_helper_entry(e, name, package, ctx, by_name, all_names, taken):
    """(candidate, None) for a helper entry whose signature, types, includes and body are usable, else (None, reason).
    taken: the helper names already accepted for this class."""
    kind, fname = e["kind"], e.get("function") or ""
    if e.get("body") is None:
        return None, e.get("reject") or "no body"
    try:
        ret, sname, params, const = parse_helper_signature(e.get("signature"), kind == "static_helper")
    except ValueError as exc:
        return None, str(exc)
    if sname != fname:
        return None, "signature names %s, the entry names %s" % (sname, fname)
    if fname in by_name or fname in taken:
        return None, "%s is already a function of this class" % fname
    access = e.get("access")
    if kind == "helper" and access not in HELPER_ACCESS:
        return None, "a helper needs access public or protected, not %r" % (access,)
    if kind == "static_helper" and access is not None:
        return None, "a static helper is not a member and takes no access"
    explicit = list(e.get("includes") or [])
    bad = [inc for inc in explicit if not usable_include(inc, ctx)]
    if bad:
        return None, "includes not in the engine header index: %s" % ", ".join(str(b) for b in bad)
    names, texts = helper_type_names(ret, params)
    inc_sig, fwd, problems = helper_type_refs(names, texts, package, ctx, bool(explicit))
    if problems:
        return None, "; ".join(problems)
    inc_body, problems = body_refs(e["body"], name, package, ctx)
    if problems:
        return None, "; ".join(problems)
    refs = (set(re.findall(r"[A-Za-z_]\w*", strip_comments(e["body"]))) & all_names) - {fname}
    return {"name": fname, "kind": kind, "tag": "%s::%s" % (name, fname), "ret": ret, "args": params,
            "const": const, "access": access, "body": e["body"], "refs": refs,
            "includes": inc_body | inc_sig | set(explicit), "header_includes": inc_sig | set(explicit),
            "forward": fwd}, None


def attach_helpers(name, entries, ctx, package, by_name):
    """Helper entries (kinds helper and static_helper) of one class. Each one is checked by check_helper_entry. Then
    a helper is dropped when its body names a helper that is not emitted, a static helper when its body names a member
    helper, and static helpers in a dependency cycle are dropped. Returns {"members", "statics", "accepted",
    "rejected", "includes", "header_includes", "forward_decls", "missing"}: members public first, statics dependencies
    first, missing the names of the class's helper entries that are not emitted."""
    out = {"members": [], "statics": [], "accepted": [], "rejected": [], "includes": set(), "header_includes": set(),
           "forward_decls": set(), "missing": set()}
    helper_entries = [e for e in entries if e.get("kind") in HELPER_KINDS]
    all_names = {e.get("function") or "" for e in helper_entries}
    cands, taken = {}, set()
    for e in helper_entries:
        cand, reason = check_helper_entry(e, name, package, ctx, by_name, all_names, taken)
        if cand is None:
            out["rejected"].append({"entry": "%s::%s" % (name, e.get("function")), "kind": e.get("kind"),
                                    "reason": reason})
            continue
        cands[cand["name"]] = cand
        taken.add(cand["name"])
    members = {n for n in cands if cands[n]["kind"] == "helper"}
    statics = {n for n in cands if cands[n]["kind"] == "static_helper"}
    accepted, dropped = set(cands), {}
    changed = True
    while changed:
        changed = False
        for n in sorted(accepted):
            for r in sorted(cands[n]["refs"]):
                if r not in accepted:
                    dropped[n] = "uses helper %s, which is not emitted" % r
                elif cands[n]["kind"] == "static_helper" and r in members:
                    dropped[n] = "static helper uses member helper %s" % r
                if n in dropped:
                    break
            if n in dropped:
                accepted.discard(n)
                changed = True
        _ordered, cyclic = static_order(accepted & statics, {n: cands[n]["refs"] for n in statics})
        for n in sorted(cyclic):
            accepted.discard(n)
            dropped[n] = "static helper is in or behind a dependency cycle"
            changed = True
    ordered, _left = static_order(accepted & statics, {n: cands[n]["refs"] for n in statics})
    member_order = sorted((n for n in cands if n in accepted and n in members),
                          key=lambda n: cands[n]["access"] != "public")
    out["members"] = [cands[n] for n in member_order]
    out["statics"] = [cands[n] for n in ordered]
    out["accepted"] = [cands[n]["tag"] for n in cands if n in accepted]
    out["rejected"].extend({"entry": cands[n]["tag"], "kind": cands[n]["kind"], "reason": why}
                           for n, why in sorted(dropped.items()))
    for n in accepted:
        cand = cands[n]
        out["includes"] |= cand["includes"]
        out["forward_decls"] |= cand["forward"]
        if cand["kind"] == "helper":
            out["header_includes"] |= cand["header_includes"]
    out["missing"] = all_names - accepted
    return out


# ---------------------------------------------------------------- member fields (non-reflected data members)
# kind "member_field": a plain data member of the class, declared in the header outside any UPROPERTY, under the
# entry's access, with an optional default initialiser. It is not replicated and has no body (no inc_path). A field
# with an offset is placed in declaration order among the reflected members: before the first member whose offset is
# larger. A field without an offset goes after the reflected members. Contract: porting/behavior/README.md, "member_field".
FIELD_KIND = "member_field"
FIELD_ACCESS = ("public", "protected", "private")
FIELD_SIG_RE = re.compile(r"^(?P<type>.*?[\s*&>])\s*(?P<name>[A-Za-z_]\w*)$")


def parse_field_signature(sig):
    """(type, name) of a member_field signature: 'type name', without ';' and without the initialiser."""
    text = re.sub(r"\s+", " ", sig or "").strip().rstrip(";").strip()
    m = FIELD_SIG_RE.match(text)
    if not m or not m.group("type").strip():
        raise ValueError("field signature is not 'type name': %r" % (sig,))
    typ, name = m.group("type").strip(), m.group("name")
    if re.match(r"(static|mutable|volatile|virtual|inline|extern|typedef|using|UPROPERTY)\b", typ):
        raise ValueError("a member field takes no %s" % typ.split()[0])
    return typ, name


def field_offset(value):
    """The offset of a member_field entry: an int, or a decimal or 0x string. None when the entry has none."""
    if value is None:
        return None
    if isinstance(value, int) and not isinstance(value, bool):
        return value
    if isinstance(value, str):
        return int(value, 0)
    raise ValueError("offset must be a number or a string such as 0xe88")


def check_field_entry(e, name, package, ctx, taken_names):
    """(field, None) for a usable member_field entry, else (None, reason). taken_names: names already in the class."""
    fname = e.get("function") or ""
    if e.get("inc_path"):
        return None, "a member field has no body: inc_path must be absent"
    try:
        typ, sname = parse_field_signature(e.get("signature"))
        offset = field_offset(e.get("offset"))
    except ValueError as exc:
        return None, str(exc)
    if sname != fname:
        return None, "signature names %s, the entry names %s" % (sname, fname)
    if fname in taken_names:
        return None, "%s is already a member, helper or function of this class" % fname
    access = e.get("access")
    if access not in FIELD_ACCESS:
        return None, "a member field needs access public, protected or private, not %r" % (access,)
    default = e.get("default")
    if isinstance(default, (int, float)) and not isinstance(default, bool):
        default = str(default)
    if default is not None and (not isinstance(default, str) or re.search(r"[;{}\n]", default)):
        return None, "default must be one expression without ; { } or a newline"
    if default == "":
        default = None
    explicit = list(e.get("includes") or [])
    bad = [inc for inc in explicit if not usable_include(inc, ctx)]
    if bad:
        return None, "includes not in the engine header index: %s" % ", ".join(str(b) for b in bad)
    inc_sig, fwd, problems = helper_type_refs(set(TYPE_NAME_RE.findall(typ)), [typ], package, ctx, bool(explicit))
    if problems:
        return None, "; ".join(problems)
    decl = ("%s %s = %s;" % (typ, fname, default)) if default is not None else ("%s %s;" % (typ, fname))
    return {"name": fname, "tag": "%s::%s" % (name, fname), "type": typ, "decl": decl, "access": access,
            "default": default, "offset": offset, "includes": inc_sig | set(explicit), "forward": fwd}, None


def attach_fields(name, entries, ctx, package, taken_names):
    """The member_field entries of one class. taken_names: the reflected member, helper and function names of the class.
    Returns {"fields", "accepted", "rejected", "header_includes", "forward_decls"}; fields in offset order (no offset
    last, manifest order within a tie)."""
    out = {"fields": [], "accepted": [], "rejected": [], "header_includes": set(), "forward_decls": set()}
    cands, taken = [], set(taken_names)
    for e in entries:
        if e.get("kind") != FIELD_KIND:
            continue
        field, reason = check_field_entry(e, name, package, ctx, taken)
        if field is None:
            out["rejected"].append({"entry": "%s::%s" % (name, e.get("function")), "kind": FIELD_KIND,
                                    "reason": reason})
            continue
        cands.append(field)
        taken.add(field["name"])
    cands.sort(key=lambda f: (f["offset"] is None, f["offset"] or 0))
    out["fields"] = cands
    for f in cands:
        out["accepted"].append(f["tag"])
        out["header_includes"] |= f["includes"]
        out["forward_decls"] |= f["forward"]
    return out


def field_lines(fd):
    """Header lines of one member_field: a plain declaration under its access label, outside any UPROPERTY. A label
    other than public is restored to public afterwards, since the reflected members that follow are public."""
    where = "" if fd.get("offset") is None else ", offset 0x%x" % fd["offset"]
    lines = [] if fd["access"] == "public" else ["%s:" % fd["access"]]
    lines.append("\t// Non-reflected field (behaviour kind \"member_field\"%s): not a UPROPERTY, not replicated." % where)
    lines.append("\t%s" % fd["decl"])
    if fd["access"] != "public":
        lines.append("public:")
    lines.append("")
    return lines


def attach_snippets(name, functions, entries, ctx, package, skipped=(), member_names=()):
    """Attach one class's snippets to its emitted functions, in place. Sets body/validate/accepted keys and returns
    {"accepted": [tag], "rejected": [{entry, kind, reason}], "includes": set}. Rejected getter/exec records are removed
    by the caller. skipped is the class's skip list (why an RPC was not emitted)."""
    by_name = {f["name"]: f for f in functions}
    helpers = attach_helpers(name, entries, ctx, package, by_name)
    helper_names = {e.get("function") for e in entries if e.get("kind") in HELPER_KINDS}
    fields = attach_fields(name, entries, ctx, package, set(member_names) | set(by_name) | helper_names)
    out = {"accepted": list(helpers["accepted"]) + fields["accepted"],
           "rejected": list(helpers["rejected"]) + fields["rejected"],
           "includes": set(helpers["includes"]),
           "header_includes": set(helpers["header_includes"]) | fields["header_includes"],
           "forward_decls": set(helpers["forward_decls"]) | fields["forward_decls"], "helpers": helpers["members"],
           "static_helpers": helpers["statics"], "fields": fields["fields"]}
    for e in entries:
        if e.get("kind") in HELPER_KINDS or e.get("kind") == FIELD_KIND:
            continue
        tag = "%s::%s" % (name, e["function"])
        kind, fname = e.get("kind"), e.get("function") or ""

        def reject(reason):
            out["rejected"].append({"entry": tag, "kind": kind, "reason": reason})

        if e.get("body") is None:
            reject(e.get("reject") or "no body")
            continue
        if kind in ("getter", "exec"):
            target = by_name.get(fname)
            if target is None or not target.get("splice"):
                reject("function not emitted: " + not_emitted_reason(skipped, fname))
                continue
            want = "%s %s(%s)%s" % (target["ret"], fname, ", ".join(target["args"]), " const" if target["const"] else "")
        elif kind == "impl" and fname.endswith("_Implementation"):
            base = fname[:-len("_Implementation")]
            target = by_name.get(base)
            if target is None or not target["rpc"] or target.get("service"):
                reject("RPC %s not emitted: %s" % (base, not_emitted_reason(skipped, base)))
                continue
            want = "void %s_Implementation(%s)%s" % (base, ", ".join(target["args"]),
                                                    " const" if target.get("const") else "")
        elif kind == "validate" and fname.endswith("_Validate"):
            base = fname[:-len("_Validate")]
            target = by_name.get(base)
            if target is None or not target["rpc"]:
                reject("RPC %s not emitted: %s" % (base, not_emitted_reason(skipped, base)))
                continue
            if "Server" not in target["specifiers"]:
                reject("WithValidation needs a Server RPC; %s is not one" % base)
                continue
            want = "bool %s_Validate(%s)" % (base, ", ".join(target["args"]))
        elif kind == "onrep":
            target = by_name.get(fname)
            if target is None or target["rpc"] or not target["notify"]:
                reject("RepNotify function %s is not emitted" % fname)
                continue
            want = "void %s()" % fname
        else:
            reject("unknown kind %r or name %r" % (kind, fname))
            continue
        if squash(e.get("signature")) != squash(want):
            reject("manifest signature %r differs from the reflection-derived %r" % (e.get("signature"), want))
            continue
        if kind in ("getter", "exec") and e.get("static", False) != target.get("static", False):
            reject("manifest staticness differs from reflection")
            continue
        if kind in ("getter", "exec") and target.get("static"):
            code = strip_comments(e["body"])
            if re.search(r"\bthis\b", code):
                reject("static reflected function uses this")
                continue
            instance_names = set(member_names) | {f["name"] for f in fields["fields"]}
            instance_names |= {f["name"] for f in functions if not f.get("static")}
            instance_names |= {h["name"] for h in helpers["members"]}
            used_members = sorted(set(re.findall(r"(?<![\w.:>])[A-Za-z_]\w*", code)) & instance_names)
            if used_members:
                reject("static reflected function uses instance member %s" % used_members[0])
                continue
        if kind == "validate":
            if target.get("validate"):
                reject("second validate snippet for %s" % base)
                continue
        elif target.get("body") is not None:
            reject("second snippet for %s" % fname)
            continue
        hit = sorted(set(re.findall(r"[A-Za-z_]\w*", strip_comments(e["body"]))) & helpers["missing"])
        if hit:
            reject("body uses helper %s, which is not emitted" % hit[0])
            continue
        includes, problems = body_refs(e["body"], name, package, ctx)
        if problems:
            reject("; ".join(problems))
            continue
        explicit_includes = e.get("includes") or []
        if not isinstance(explicit_includes, list) or any(not usable_include(inc, ctx) for inc in explicit_includes):
            reject("snippet includes not in the engine header index")
            continue
        includes.update(explicit_includes)
        if kind == "validate":
            target["validate"] = True
            target["validate_body"] = e["body"]
            target["specifiers"] = target["specifiers"] + ["WithValidation"]
        else:
            target["body"] = e["body"]
        if kind in ("getter", "exec"):
            target["accepted"] = True
        out["includes"] |= includes
        out["accepted"].append(tag)
    return out


def verify_spliced(model, header, cpp):
    """Declaration/definition check of every spliced function in the rendered text. Raises SystemExit on a mismatch."""
    name = model["name"]
    hlines = header.splitlines()
    hsq = [squash(line) for line in hlines]
    clines = [line.rstrip() for line in cpp.splitlines()]
    problems = []

    def count(lines_sq, key):
        return sum(1 for line in lines_sq if line == key)

    def check_def(def_text, expected, tag):
        key = squash(def_text)
        starts = [i for i, line in enumerate(clines) if squash(line) == key]
        if len(starts) != 1:
            problems.append("%s: definition found %d times in cpp" % (tag, len(starts)))
            return
        i = starts[0]
        if i + 1 >= len(clines) or clines[i + 1] != "{":
            problems.append("%s: definition not followed by '{'" % tag)
            return
        end = next((j for j in range(i + 2, len(clines)) if clines[j] == "}"), None)
        if end is None:
            problems.append("%s: no closing brace" % tag)
            return
        if clines[i + 2:end] != expected:
            problems.append("%s: cpp body differs from the snippet" % tag)
            return None
        return i

    for f in model["functions"]:
        if f.get("body") is None and not f.get("validate"):
            continue
        ret, args = f.get("ret", "void"), ", ".join(f["args"])
        const = " const" if f.get("const") else ""
        tag = "%s::%s" % (name, f["name"])
        ufunc = squash("UFUNCTION(%s)" % ", ".join(f["specifiers"]))
        decl = squash("%s%s %s(%s)%s;" % ("static " if f.get("static") else "", ret, f["name"], args, const))
        if count(hsq, decl) != 1:
            problems.append("%s: declaration found %d times in header" % (tag, count(hsq, decl)))
        else:
            idx = hsq.index(decl)
            if idx == 0 or hsq[idx - 1] != ufunc:
                problems.append("%s: UFUNCTION specifiers differ from %s" % (tag, ufunc))
        if f["rpc"] and not f.get("service"):
            impl = "%s_Implementation" % f["name"]
            if count(hsq, squash("void %s(%s);" % (impl, args))) != 1:
                problems.append("%s: _Implementation declaration missing or repeated" % tag)
            check_def("void %s::%s(%s)" % (name, impl, args), body_lines(f.get("body")), tag)
        else:
            check_def("%s %s::%s(%s)%s" % (ret, name, f["name"], args, const), body_lines(f.get("body")), tag)
        if f.get("validate"):
            if "WithValidation" not in f["specifiers"]:
                problems.append("%s: validate without WithValidation" % tag)
            vdecl = squash("bool %s_Validate(%s);" % (f["name"], args))
            if count(hsq, vdecl) != 1:
                problems.append("%s: _Validate declaration missing or repeated" % tag)
            check_def("bool %s::%s_Validate(%s)" % (name, f["name"], args),
                      body_lines(f.get("validate_body")), tag + "_Validate")
    ns_open = next((i for i, line in enumerate(clines) if line == "namespace" and i + 1 < len(clines)
                    and clines[i + 1] == "{"), None)
    ns_close = next((i for i, line in enumerate(clines) if line == "}  // namespace"), None)
    member_defs = [i for i, line in enumerate(clines) if re.match(r"^\S.*\b%s::" % re.escape(name), line)]
    for h in model.get("helpers") or []:
        ret, args = h["ret"], ", ".join(h["args"])
        const = " const" if h["const"] else ""
        tag = "%s::%s" % (name, h["name"])
        decl = squash("%s %s(%s)%s;" % (ret, h["name"], args, const))
        if count(hsq, decl) != 1:
            problems.append("%s: declaration found %d times in header" % (tag, count(hsq, decl)))
        else:
            idx = hsq.index(decl)
            if idx == 0 or hsq[idx - 1].startswith("UFUNCTION"):
                problems.append("%s: helper declared inside a UFUNCTION declaration" % tag)
            access = next((hsq[j] for j in range(idx - 1, -1, -1)
                           if hsq[j] in ("public:", "protected:", "private:")), None)
            if access != h["access"] + ":":
                problems.append("%s: declared under %s, not %s:" % (tag, access, h["access"]))
        check_def("%s %s::%s(%s)%s" % (ret, name, h["name"], args, const), body_lines(h["body"]), tag)
    for s in model.get("static_helpers") or []:
        ret, args = s["ret"], ", ".join(s["args"])
        tag = "%s::%s" % (name, s["name"])
        if count(hsq, squash("%s %s(%s);" % (ret, s["name"], args))) != 0:
            problems.append("%s: static helper declared in the header" % tag)
        i = check_def("static %s %s(%s)" % (ret, s["name"], args), body_lines(s["body"]), tag)
        if i is None:
            continue
        if ns_open is None or ns_close is None or not ns_open < i < ns_close:
            problems.append("%s: static helper outside the anonymous namespace" % tag)
        elif member_defs and i > min(member_defs):
            problems.append("%s: static helper defined after a member function" % tag)
    members = model.get("members_decl_order") or []
    for fd in model.get("fields") or []:
        tag = "%s::%s" % (name, fd["name"])
        decl = squash(fd["decl"])
        if count(hsq, decl) != 1:
            problems.append("%s: field declaration found %d times in header" % (tag, count(hsq, decl)))
            continue
        idx = hsq.index(decl)
        if idx > 0 and hsq[idx - 1].startswith(("UPROPERTY", "UFUNCTION")):
            problems.append("%s: field declared under a UPROPERTY or UFUNCTION" % tag)
        access = next((hsq[j] for j in range(idx - 1, -1, -1) if hsq[j] in ("public:", "protected:", "private:")), None)
        if access != fd["access"] + ":":
            problems.append("%s: field declared under %s, not %s:" % (tag, access, fd["access"]))
        if fd.get("offset") is not None:
            pos = {squash(m["decl"]): hsq.index(squash(m["decl"])) for m in members
                   if m.get("offset") is not None and squash(m["decl"]) in hsq}
            before = [pos[squash(m["decl"])] for m in members if m.get("offset") is not None
                      and m["offset"] < fd["offset"] and squash(m["decl"]) in pos]
            after = [pos[squash(m["decl"])] for m in members if m.get("offset") is not None
                     and m["offset"] > fd["offset"] and squash(m["decl"]) in pos]
            if (before and idx < max(before)) or (after and idx > min(after)):
                problems.append("%s: field out of offset order among the reflected members" % tag)
    if problems:
        raise SystemExit("behaviour splice verification failed for %s:\n  %s" % (name, "\n  ".join(problems)))


def render_helper_declarations(model):
    """Header lines of the helper entries: plain member declarations after the reflected members, outside any UFUNCTION
    declaration. The access label is written when it changes (the class body starts public)."""
    helpers = model.get("helpers") or []
    if not helpers:
        return []
    lines = ["", "\t// Non-reflected helpers (behaviour kind \"helper\", porting/behavior/README.md): plain C++ member",
             "\t// functions, not UFUNCTIONs and not part of any UFUNCTION declaration."]
    current = "public"
    for h in helpers:
        if h["access"] != current:
            lines.append("%s:" % h["access"])
            current = h["access"]
        lines.append("\t%s %s(%s)%s;" % (h["ret"], h["name"], ", ".join(h["args"]), " const" if h["const"] else ""))
    return lines


def render_promoted_header(model, api, parent_include, source_label):
    name = model["name"]
    rep = [m for m in model["members_table_order"] if m["replicated"]]
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// Do not edit by hand: run porting/promote_native_classes.sh. Names, types, property order, replication",
        "// and RPC roles come from the shipping exe. Bodies are not recovered; see porting/native_classes_status.md.",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
        "#include \"%s\"" % parent_include,
        "",
    ]
    for inc in model.get("includes", []):
        lines.append("#include \"%s\"" % inc)
    if model.get("includes"):
        lines.append("")
    # A forward-declared class whose header we include (forward_decl_includes) needs no bare decl: the
    # included header carries the full definition (and the UHT-generated code relies on it).
    declared = set("%s.h" % ref for ref in model["forward_decls"]) & set(model.get("forward_decl_includes") or [])
    decls = [ref for ref in model["forward_decls"] if "%s.h" % ref not in declared]
    lines.extend("class %s;" % ref for ref in decls)
    if decls:
        lines.append("")
    lines.append("#include \"%s.generated.h\"" % name)
    lines.append("")
    lines.append(model["uclass"])
    lines.append("class %s %s : public %s" % (api, name, model["parent"]))
    lines.append("{")
    lines.append("\tGENERATED_BODY()")
    lines.append("")
    lines.append("public:")
    pending = sorted(model.get("fields") or [], key=lambda f: (f.get("offset") is None, f.get("offset") or 0))
    for m in model["members_decl_order"]:
        while (pending and pending[0].get("offset") is not None and m.get("offset") is not None
               and pending[0]["offset"] < m["offset"]):
            lines.extend(field_lines(pending.pop(0)))
        lines.append("\t// table index %s, flags %s" % (m["table_index"], m["property_flags"]))
        lines.append("\t%s" % uproperty_line(m["specifiers"]))
        lines.append("\t%s" % m["decl"])
        lines.append("")
    for fd in pending:
        lines.extend(field_lines(fd))
    for f in model["functions"]:
        if f.get("blueprint_event"):
            lines.append(f.get("access", "protected") + ":")
        lines.append("\tUFUNCTION(%s)" % ", ".join(f["specifiers"]))
        lines.append("\t%s%s %s(%s)%s;" % ("static " if f.get("static") else "", f.get("ret", "void"), f["name"],
                                       ", ".join(header_arg(f, a) for a in f["args"]),
                                       " const" if f.get("const") else ""))
        if f.get("blueprint_event"):
            lines.append("public:")
        if f["rpc"] and not f.get("service"):
            lines.append("\tvoid %s_Implementation(%s)%s;" % (f["name"], ", ".join(f["args"]),
                                                          " const" if f.get("const") else ""))
        if f.get("validate"):
            lines.append("\tbool %s_Validate(%s);" % (f["name"], ", ".join(f["args"])))
        lines.append("")
    if rep:
        lines.append("\tvirtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;")
    lines.extend(render_helper_declarations(model))
    lines.append("};")
    lines.append("")
    return "\n".join(lines) + "\n"


def render_interface_header(name, source_label, api):
    """UINTERFACE U<Name> (parent UInterface) and its native interface I<Name>, in one header as UHT expects."""
    iname = "I" + name[1:]
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// Do not edit by hand: run porting/promote_native_classes.sh. The interface's UFUNCTIONs are not recovered.",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
        "#include \"UObject/Interface.h\"",
        "",
        "#include \"%s.generated.h\"" % name,
        "",
        "UINTERFACE()",
        "class %s %s : public UInterface" % (api, name),
        "{",
        "\tGENERATED_BODY()",
        "};",
        "",
        "class %s %s" % (api, iname),
        "{",
        "\tGENERATED_IINTERFACE_BODY()",
        "};",
        "",
    ]
    return "\n".join(lines) + "\n"


def render_interface_cpp(name, source_label):
    return "\n".join([
        "// Generated by porting/gen_native_classes.py --promote from %s. Do not edit by hand." % source_label,
        "#include \"%s.h\"" % name,
        "",
    ])


def replication_line(class_name, prop_name):
    """The DOREPLIFETIME line of one replicated property. With REPLICATION_CONDITIONS loaded (--promote), the
    recovered condition is used. A property whose registration was not recovered gets COND_None and a TODO."""
    if not REPLICATION_CONDITIONS:
        return "\tDOREPLIFETIME(%s, %s);" % (class_name, prop_name)
    cls = REPLICATION_CONDITIONS.get(class_name) or {}
    rec = (cls.get("properties") or {}).get(prop_name) or {}
    cond = rec.get("condition", "COND_None")
    notify = rec.get("rep_notify_condition")
    conf = rec.get("confidence", "unknown")
    if conf == "unknown" or not rec:
        return "\tDOREPLIFETIME(%s, %s);  // TODO: replication condition not recovered (COND_None assumed)" % (
            class_name, prop_name)
    note = "" if conf == "high" else "  // replication condition confidence: %s" % conf
    if notify:
        return "\tDOREPLIFETIME_CONDITION_NOTIFY(%s, %s, %s, %s);%s" % (class_name, prop_name, cond, notify, note)
    if cond != "COND_None":
        return "\tDOREPLIFETIME_CONDITION(%s, %s, %s);%s" % (class_name, prop_name, cond, note)
    return "\tDOREPLIFETIME(%s, %s);%s" % (class_name, prop_name, note)


def load_replication_conditions(path):
    """Fill REPLICATION_CONDITIONS from the data file. A missing file leaves it empty (plain DOREPLIFETIME)."""
    REPLICATION_CONDITIONS.clear()
    if not path or not os.path.exists(path):
        return 0
    with open(path, encoding="utf-8") as fh:
        data = json.load(fh)
    REPLICATION_CONDITIONS.update(data.get("classes") or {})
    return len(REPLICATION_CONDITIONS)


def render_promoted_cpp(model, source_label):
    name = model["name"]
    rep = [m for m in model["members_table_order"] if m["replicated"]]
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s. Do not edit by hand." % source_label,
        "#include \"%s.h\"" % name,
    ]
    for inc in model.get("cpp_includes", []):
        lines.append("#include \"%s\"" % inc)
    if rep:
        lines.append("#include \"Net/UnrealNetwork.h\"")
    lines.append("")
    statics = model.get("static_helpers") or []
    if statics:
        lines.append("namespace")
        lines.append("{")
        for h in statics:
            lines.append("static %s %s(%s)" % (h["ret"], h["name"], ", ".join(h["args"])))
            lines.append("{")
            lines.extend(body_lines(h["body"]))
            lines.append("}")
            lines.append("")
        lines.append("}  // namespace")
        lines.append("")
    for f in model["functions"]:
        # UHT generates the ProcessEvent forwarder for BlueprintImplementableEvent.
        if f.get("blueprint_event"):
            continue
        # A service request has no definition here: UHT generates its Event thunk (a ProcessEvent forwarder, one per
        # request: 138 of 138 in the scratch build), and a second definition is a duplicate symbol at link time.
        if f.get("service") == "Request":
            if f.get("body") is not None:
                raise ValueError("%s: a behaviour body cannot be spliced into a service request (UHT generates it)" % f["name"])
            continue
        # A net service response is the programmer's own body (UHT: CppImplName = the name), so it has no _Implementation.
        impl = "%s_Implementation" % f["name"] if f["rpc"] and not f.get("service") else f["name"]
        args = ", ".join(f["args"])
        lines.append("%s %s::%s(%s)%s" % (f.get("ret", "void"), name, impl, args, " const" if f.get("const") else ""))
        lines.append("{")
        body = body_lines(f.get("body"))
        if f.get("body") is None and f.get("service") and f.get("ret", "void") != "void":
            body.append("\treturn {};")  # the stub of a net service function that returns a value still returns one
        lines.extend(body)
        lines.append("}")
        lines.append("")
        if f.get("validate"):
            lines.append("bool %s::%s_Validate(%s)" % (name, f["name"], args))
            lines.append("{")
            lines.extend(body_lines(f.get("validate_body")))
            lines.append("}")
            lines.append("")
    for h in model.get("helpers") or []:
        lines.append("%s %s::%s(%s)%s" % (h["ret"], name, h["name"], ", ".join(h["args"]),
                                          " const" if h["const"] else ""))
        lines.append("{")
        lines.extend(body_lines(h["body"]))
        lines.append("}")
        lines.append("")
    if rep:
        lines.append("void %s::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const" % name)
        lines.append("{")
        lines.append("\tSuper::GetLifetimeReplicatedProps(OutLifetimeProps);")
        lines.append("")
        for m in rep:
            lines.append(replication_line(name, m["name"]))
        lines.append("}")
        lines.append("")
    return "\n".join(lines) + "\n"


def build_engine_header_index(root):
    """Map engine class name -> header path relative to the Classes directory (the include root)."""
    if not os.path.isdir(root):
        raise SystemExit("engine Classes directory not found: %s" % root)
    index = {}
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        for filename in sorted(filenames):
            if not filename.endswith(".h"):
                continue
            path = os.path.join(dirpath, filename)
            with open(path, "r", encoding="utf-8", errors="replace") as fh:
                text = fh.read()
            rel = os.path.relpath(path, root).replace(os.sep, "/")
            for match in ENGINE_CLASS_DECL_RE.finditer(text):
                index.setdefault(match.group(1), rel)
    return index


def count_by(records, key):
    out = {}
    for record in records:
        value = str(record.get(key))
        out[value] = out.get(value, 0) + 1
    return dict(sorted(out.items(), key=lambda kv: (-kv[1], kv[0])))


def engine_macro_names(engine_root):
    """Global typedef names the engine's party interface creates through PARTY_DECLARE_DELEGATETYPE(X): F<X>Delegate.
    The token pasting hides them from the name search, and a generated type of the same name clashes with them."""
    path = os.path.join(engine_root, "Plugins", "Online", "OnlineSubsystem", "Source", "Public", "Interfaces",
                        "OnlinePartyInterface.h")
    if not os.path.isfile(path):
        return set()
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        found = re.findall(r"PARTY_DECLARE_DELEGATETYPE\((\w+)\)", fh.read())
    return {"F%sDelegate" % name for name in found if name != "Type"}


def cross_module_allowed(current_package, target_package):
    """True when an item of current_package may use a promoted type of target_package. Only the game module does, and
    only for the runtime modules in CROSS_MODULE_TARGETS: its Build.cs lists them and they do not depend on it. Other
    pairs stay refused (TypeGate: OtherModule), which keeps the module graph acyclic; promote also refuses a cycle."""
    return current_package == "/Script/FortniteGame" and target_package in CROSS_MODULE_TARGETS


def own_includes(gate, used, package):
    """Headers the items of one package include: engine headers, the package's own promoted types, and promoted
    types of another package that cross_module_allowed admits (their module is a dependency, see cross_module_modules)."""
    out = set()
    for fn in used:
        info = gate.info(fn)
        if (info is not None and info.package in PROMOTE_PACKAGES and info.package != package
                and not cross_module_allowed(package, info.package)
                and "ObservedBlueprintEventType" not in used[fn]):
            continue
        inc = gate.include_of(fn)
        if inc:
            out.add(inc)
    return sorted(out)


def cross_module_modules(gate, used, package):
    """Modules of the promoted types of other packages that the items of one package use (module dependencies)."""
    out = set()
    for fn in used:
        info = gate.info(fn)
        if info is not None and info.package in PROMOTE_PACKAGES and info.package != package:
            out.add(info.module)
    return out


def find_module_cycle(module_deps, promoted_modules):
    """A cycle among the modules that the generator writes (engine modules are not part of the graph), or None."""
    graph = {m: sorted(d for d in deps if d in promoted_modules) for m, deps in module_deps.items()}
    state = {}
    stack = []

    def visit(node):
        state[node] = 1
        stack.append(node)
        for nxt in graph.get(node, []):
            if state.get(nxt) == 1:
                return stack[stack.index(nxt):] + [nxt]
            if state.get(nxt) is None:
                found = visit(nxt)
                if found:
                    return found
        state[node] = 2
        stack.pop()
        return None

    for node in sorted(graph):
        if state.get(node) is None:
            found = visit(node)
            if found:
                return found
    return None


def render_module_build_cs(module, deps, source_label):
    base = ["Core", "CoreUObject", "Engine", "Slate"]   # Slate: FSlateApplication is used by widgets (CommonUI)
    names = ", ".join('"%s"' % d for d in base + [d for d in deps if d not in base])
    return "\n".join([
        "using System.IO;",
        "using UnrealBuildTool;",
        "",
        "// Runtime module generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// It is named after its original package (/Script/%s): the class path is part of the wire format, and the" % module,
        "// package is not in the UE 4.25 engine tree, so the module stands in for it. Do not edit by hand: run",
        "// porting/promote_native_classes.sh. See porting/native_classes_status.md, \"Runtime modules\".",
        "public class %s : ModuleRules" % module,
        "{",
        "    public %s(ReadOnlyTargetRules Target) : base(Target)" % module,
        "    {",
        "        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;",
        "        PublicDependencyModuleNames.AddRange(new[] { %s });" % names,
        "        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, \"Generated\"));",
        "    }",
        "}",
        "",
    ])


def render_module_cpp(module, source_label):
    return "\n".join([
        "// Generated by porting/gen_native_classes.py --promote from %s. Do not edit by hand." % source_label,
        "#include \"Modules/ModuleManager.h\"",
        "",
        "IMPLEMENT_MODULE(FDefaultModuleImpl, %s);" % module,
        "",
    ])


def promote(args):
    loaded = load_replication_conditions(args.replication_conditions)
    print("replication conditions: %d classes from %s" % (loaded, args.replication_conditions or "(none)"),
          file=sys.stderr)
    missing = [flag for flag, value in (("--class-list", args.class_list), ("--evidence", args.evidence),
                                        ("--engine-classes", args.engine_classes), ("--source-root", args.source_root),
                                        ("--report-dir", args.report_dir), ("--name-map", args.name_map),
                                        ("--parent-map", args.parent_map),
                                        ("--class-ref-map", args.class_ref_map)) if not value]
    if missing:
        raise SystemExit("--promote needs: %s" % ", ".join(missing))
    data = load_json(args.reflection)
    blocks = data.get("classes_in_groups_with_fort_names") or data.get("classes") or []
    blocks_by_va = {str(b.get("block_va", "")).lower(): b for b in blocks}
    names = {k.lower(): v for k, v in load_json(args.name_map).items()}
    parents = lower_keys(load_json(args.parent_map))
    evidence = load_json(args.evidence)
    evidence_fns = {}
    for rec in evidence.get("blocks", []):
        fns = []
        for fn in rec.get("construct_fns") or []:
            fn = str(fn).lower()
            fns.append(fn if fn.startswith("0x") else "0x" + fn)
        evidence_fns[str(rec.get("block_va", "")).lower()] = fns

    with open(args.class_list, "r", encoding="utf-8") as fh:
        class_list = [line.split("#", 1)[0].strip() for line in fh]
    class_list = [name for name in class_list if name]
    if len(set(class_list)) != len(class_list):
        raise SystemExit("duplicate entries in %s" % args.class_list)
    listed = set(class_list)
    name_to_block = {v: k for k, v in names.items()}
    registry = ModuleRegistry(args.engine_root, PROJECT_NATIVE_DIR, UPROJECT_DEFAULT)
    blocklist = load_blocklist(args.blocklist)

    def is_interface(block):
        return bool(hx((blocks_by_va.get(block) or {}).get("class_flags")) & CLASS_INTERFACE)

    def engine_like(package):
        # A class outside the project that the project may depend on: its module is usable (see ModuleRegistry).
        return package not in PROMOTE_PACKAGES and registry.usable_package(package)

    # Closure check: every class is in a promote package, is not blocked or an interface, and each parent is listed
    # or in a usable module.
    errors = []
    for name in class_list:
        block = name_to_block.get(name)
        entry = parents.get(block or "") or {}
        if name in blocklist:
            errors.append("%s: listed in the blocklist (%s)" % (name, blocklist[name]))
        elif block is None or not entry:
            errors.append("%s: no named block with a parent record" % name)
        elif entry.get("package") not in PROMOTE_PACKAGES:
            errors.append("%s: package %s is not a promote package" % (name, entry.get("package")))
        elif is_interface(block):
            # A UINTERFACE is generated as U<Name> (parent UInterface) with its I<Name> class. An interface that
            # extends another interface is not generated (the parent is not a UInterface).
            if entry.get("parent_name") != "UInterface":
                errors.append("%s: UINTERFACE extending %s, not generated" % (name, entry.get("parent_name")))
        else:
            parent = entry.get("parent_name")
            if parent not in listed:
                parent_entry = parents.get(str(entry.get("parent_va") or "").lower()) or {}
                if not engine_like(parent_entry.get("package")):
                    errors.append("%s: parent %s (package %s) is neither listed nor in a usable module"
                                  % (name, parent, parent_entry.get("package")))
    if errors:
        raise SystemExit("closure check failed:\n  " + "\n  ".join(errors))

    # Type map: the native set and the classes of usable modules (forward declared). Any other named class becomes a
    # TODO token. Interfaces have no UCLASS spelling here, so they stay outside the map.
    type_map = {}
    known = set()
    for block, name in names.items():
        fns = evidence_fns.get(block, [])
        known.update(fns)
        package = (parents.get(block) or {}).get("package")
        if name in listed or (engine_like(package) and not is_interface(block)):
            for fn in fns:
                type_map[fn] = name
    # Class references in the tables point at jmp thunks to a class's NoRegister function (resolve_class_refs.py).
    class_refs = load_json(args.class_ref_map).get("resolved", {})
    for ref, block in class_refs.items():
        name = names.get(block.lower())
        if not name:
            continue
        known.add(ref.lower())
        package = (parents.get(block.lower()) or {}).get("package")
        if name in listed or (engine_like(package) and not is_interface(block.lower())):
            type_map[ref.lower()] = name
    # Struct, enum and delegate types (porting/extract_types.py): names for the type map, and the gate.
    tables = TypeTables(args.types_dir or os.path.dirname(args.reflection))
    for fn, info in tables.by_fn.items():
        type_map.setdefault(fn, info.cpp_name)
    iface_map = {}
    for block, name in names.items():
        if name in listed and is_interface(block):
            for fn in evidence_fns.get(block, []):
                iface_map[fn] = "I" + name[1:]
    resolver = Resolver(type_map, known_fns=known, iface_map=iface_map)
    gate = TypeGate(tables, EngineTypeIndex(args.engine_root, registry), resolver)
    gate.name_clashes = engine_macro_names(args.engine_root)
    resolver.gate = gate
    header_index = ModuleHeaderIndex(registry)
    source_label = os.path.relpath(args.reflection, REPO)

    # Pass 1: build every model. Nothing is written until the module dependencies are checked.
    index = []
    all_skipped = []
    class_models = []
    class_outputs = []
    module_deps = {}   # module -> modules its emitted items need (engine modules and promoted modules)
    package_of = {n: parents[name_to_block[n]].get("package") for n in listed if n in name_to_block}
    # Behaviour snippets (see attach_snippets). Disabled with FN_BEHAVIOR_SPLICE=0 or --no-behavior.
    splice_on = behavior_enabled(args)
    snippets_by_class = {}
    struct_entries = []   # kind struct_net: spliced into a struct header, not into a class (TypeGate.native_net_serializer)
    if splice_on:
        for entry in load_behavior(args.behavior_dir):
            if entry.get("kind") == STRUCT_NET_KIND:
                struct_entries.append(entry)
            else:
                snippets_by_class.setdefault(entry["class"], []).append(entry)
    struct_index = {}
    for info in tables.by_fn.values():
        struct_index.setdefault(info.cpp_name.split("::")[0], info)
    splice_ctx = {
        "class_pkg": package_of, "struct_index": struct_index, "gate": gate,
        "other_promoted": {n for b, n in names.items()
                           if n not in listed and (parents.get(b) or {}).get("package") in PROMOTE_PACKAGES},
    }
    splice_done, splice_rejects = [], []
    # struct_net entries reach the gate before pass 1 builds any struct. A second entry for one struct is refused (below).
    gate.struct_ctx = splice_ctx
    for entry in struct_entries:
        gate.struct_net.setdefault(entry["class"], entry)

    def add_deps(module, mods):
        module_deps.setdefault(module, set()).update(m for m in mods if m and m != module)

    for name in class_list:
        block = name_to_block[name]
        package = parents[block].get("package")
        module = package.rsplit("/", 1)[-1]
        module_deps.setdefault(module, set())
        if is_interface(block):
            # UINTERFACE: no properties or functions are recovered for it; it only has to exist for TScriptInterface.
            class_outputs.append((name, module, render_interface_header(name, source_label, "%s_API" % module.upper()),
                                  render_interface_cpp(name, source_label)))
            continue
        owner_fns = set(evidence_fns.get(block, []))
        gate.current_package = package
        model = build_class_promoted(blocks_by_va[block], name, resolver, parents[block], owner_fns,
                                     snippets_by_class.get(name, []), splice_ctx)
        model["includes"] = sorted(set(own_includes(gate, model["used"], package))
                                   | set(model["splice"].get("header_includes", set())))
        # Forward-declared classes of another promoted module: their full definition must be visible to the
        # UHT-generated code (the class's Z_Construct and API macro), which a bare forward declaration does not
        # provide. Include the header (the module dependency is added below); the bare decl is dropped from the
        # rendered header (see render_promoted_header).
        fwd_includes = sorted("%s.h" % ref for ref in model["forward_decls"]
                              if (package_of.get(ref) or "") in PROMOTE_PACKAGES
                              and package_of.get(ref) != package)
        if fwd_includes:
            model["includes"] = sorted(set(model["includes"]) | set(fwd_includes))
            model["forward_decl_includes"] = fwd_includes
        class_models.append(model)
        splice_done.extend(model["splice"]["accepted"])
        splice_rejects.extend(model["splice"]["rejected"])
        parent = model["parent"]
        if parent in listed:
            parent_include = "%s.h" % parent
            parent_pkg = package_of.get(parent)
        else:
            parent_pkg = (parents.get(str(parents[block].get("parent_va") or "").lower()) or {}).get("package")
            parent_module = parent_pkg.rsplit("/", 1)[-1] if parent_pkg else None
            parent_include = header_index.find(parent_module, parent)
            if parent_include is None:
                raise SystemExit("%s: no header for parent %s in a usable module (%s)" % (name, parent, parent_pkg))
            add_deps(module, [parent_module])
        if parent_pkg and parent_pkg != package:
            add_deps(module, [parent_pkg.rsplit("/", 1)[-1]])
        add_deps(module, [gate.engine.module_of_inc.get(inc) for inc in model["includes"] + model["cpp_includes"]])
        add_deps(module, cross_module_modules(gate, model["used"], package))
        # Forward-declared classes of another generated package need that module (a cycle is refused below).
        add_deps(module, [(package_of.get(ref) or "").rsplit("/", 1)[-1] for ref in model["forward_decls"]
                          if (package_of.get(ref) or "") in PROMOTE_PACKAGES and package_of.get(ref) != package])
        api = "%s_API" % module.upper()
        header = render_promoted_header(model, api, parent_include, source_label)
        cpp = render_promoted_cpp(model, source_label)
        if re.search(r"TODO_", header + cpp):
            raise SystemExit("%s: unresolved TODO token reached the output" % name)
        class_outputs.append((name, module, header, cpp))
        kept = model["members_table_order"]
        skipped_props = [r for r in model["skipped"] if r["item"] == "property"]
        for record in model["skipped"]:
            record["class"] = name
            all_skipped.append(record)
        index.append({
            "name": name, "block_va": model["block_va"], "module": module, "package": model["package"],
            "parent": parent, "parent_include": parent_include,
            "properties_total": len(kept) + len(skipped_props),
            "properties_emitted": len(kept),
            "properties_skipped": len(skipped_props),
            "replicated_total": sum(1 for m in kept if m["replicated"]) + sum(1 for r in skipped_props if r["replicated"]),
            "replicated_emitted": sum(1 for m in kept if m["replicated"]),
            "replicated_skipped": sum(1 for r in skipped_props if r["replicated"]),
            "rep_notify_downgraded": sum(1 for r in model["skipped"] if r["item"] == "rep_notify"),
            "functions_in_table": model["functions_in_table"],
            "rpc_in_reflection": model["rpc_in_reflection"],
            "rpc_emitted": sum(1 for f in model["functions"] if f["rpc"]),
            "rep_notify_functions_emitted": sum(1 for f in model["functions"] if not f["rpc"] and f.get("notify")),
            "functions_skipped": sum(1 for r in model["skipped"] if r["item"] == "function"),
            "forward_declarations": len(model["forward_decls"]),
        })

    # Closure: every promoted struct, enum and delegate used by an emitted member or RPC, transitively.
    seeds = {}
    for model in class_models:
        for fn, kinds in model["used"].items():
            seeds.setdefault(fn, set()).update(kinds)
    models, texts, all_used = emit_types(gate, seeds, source_label)
    # struct_net (behaviour kind): each native NetSerialize is checked as text in its header, and every entry is reported
    # as spliced or rejected with its reason. Nothing is written before this check.
    native_structs = {}
    for fn, model in models.items():
        info = gate.info(fn)
        native_structs[info.cpp_name] = model
        if (model.get("net") or {}).get("kind") == "native":
            verify_struct_net(model, texts[info.header_name])
    for entry in struct_entries:
        cls = entry["class"]
        tag = "%s::NetSerialize" % cls
        if gate.struct_net.get(cls) is not entry:
            splice_rejects.append({"entry": tag, "kind": STRUCT_NET_KIND,
                                   "reason": "second struct_net entry for this struct"})
        elif (native_structs.get(cls) or {}).get("net", {}).get("kind") == "native":
            splice_done.append(tag)
        else:
            if cls not in native_structs:
                reason = "struct is not emitted (not reached from the native class set)"
            else:
                reason = gate.struct_net_reasons.get(cls) or \
                    "the struct has no native NetSerialize in the binary (ops traits)"
            splice_rejects.append({"entry": tag, "kind": STRUCT_NET_KIND, "reason": reason})
    engine_used = []
    for fn, kinds in sorted(all_used.items()):
        info = gate.info(fn)
        if info is not None and info.package not in PROMOTE_PACKAGES:
            inc = gate.include_of(fn)
            engine_used.append({"type": info.cpp_name, "kind": info.kind, "package": info.package, "include": inc})
    for fn, model in models.items():
        info = gate.info(fn)
        module_deps.setdefault(info.module, set())
        add_deps(info.module, [gate.engine.module_of_inc.get(inc) for inc in model["includes"]])
        add_deps(info.module, cross_module_modules(gate, model["used"], info.package))

    # FortniteUI classes include headers of FortniteGame's promoted classes; the include map covers engine modules only.
    if "FortniteUI" in module_deps:
        add_deps("FortniteUI", ["FortniteGame"])

    # Module check: the game module lists every module its items need. The runtime modules depend on engine
    # modules only, and on each other only through parents and forward declarations; a cycle is refused.
    deps = build_cs_modules(FORTNITE_BUILD_CS)
    needed = sorted(m for m in module_deps.get("FortniteGame", ()) if m != "FortniteGame")
    missing_deps = [m for m in needed if m not in deps]
    promoted_modules = {p.rsplit("/", 1)[-1] for p in PROMOTE_PACKAGES}
    cycle = find_module_cycle(module_deps, promoted_modules)
    if cycle:
        raise SystemExit("module dependency cycle: %s" % " -> ".join(cycle))
    # Hand-written modules that hold generated classes (FortniteGame, PlayspaceFramework) list their needs in Build.cs.
    hand_missing = {}
    for module in HAND_WRITTEN_MODULES:
        if module not in module_deps:
            continue
        # Hand-written Build.cs files are never generated, so they are read from the native tree, not --source-root.
        build_cs = FORTNITE_BUILD_CS if module == "FortniteGame" else os.path.join(PROJECT_NATIVE_DIR, "Source", module, module + ".Build.cs")
        listed_deps = build_cs_modules(build_cs)
        absent = sorted(m for m in module_deps[module] if m != module and m not in listed_deps)
        if absent:
            hand_missing[module] = absent
    os.makedirs(args.report_dir, exist_ok=True)
    with open(os.path.join(args.report_dir, "required_modules.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"listed_in_build_cs": sorted(m for m in needed if m in deps),
                   "missing_from_build_cs": missing_deps,
                   "hand_written_missing": hand_missing,
                   "modules": {m: sorted(v) for m, v in sorted(module_deps.items())}}, fh, indent=1)
        fh.write("\n")
    if missing_deps or hand_missing:
        raise SystemExit("Build.cs must add these modules: %s" % json.dumps({"FortniteGame": missing_deps, **hand_missing}))

    # Behaviour snippets of classes that are not generated are not spliced. Each spliced class is verified as text
    # (declaration and definition of every spliced function) before anything is written.
    generated_names = {m["name"] for m in class_models}
    for cls, entries in sorted(snippets_by_class.items()):
        if cls not in generated_names:
            for e in entries:
                splice_rejects.append({"entry": "%s::%s" % (cls, e["function"]), "kind": e["kind"],
                                       "reason": "class is not generated (not in the native class set, or an interface)"})
    outputs = {n: (h, c) for n, _module, h, c in class_outputs}
    for model in class_models:
        if model["splice"]["accepted"]:
            verify_spliced(model, *outputs[model["name"]])

    # Pass 2: write the class files.
    for name, module, header, cpp in class_outputs:
        out_dir = os.path.join(args.source_root, module, "Generated")
        os.makedirs(out_dir, exist_ok=True)
        with open(os.path.join(out_dir, name + ".h"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(header)
        with open(os.path.join(out_dir, name + ".cpp"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(cpp)
    os.makedirs(args.report_dir, exist_ok=True)
    with open(os.path.join(args.report_dir, "behavior_splice.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"enabled": splice_on, "manifest": os.path.join(args.behavior_dir, "MANIFEST.json"),
                   "spliced": len(splice_done), "rejected": len(splice_rejects),
                   "accepted": splice_done, "rejections": splice_rejects}, fh, indent=1)
        fh.write("\n")
    print("behaviour snippets: %s, %d spliced, %d rejected" % ("on" if splice_on else "off (disabled)",
                                                              len(splice_done), len(splice_rejects)), file=sys.stderr)

    public_deps = build_cs_public_modules(FORTNITE_BUILD_CS)
    engine_shared = sorted(m for m in public_deps if m in registry.modules and m not in promoted_modules)
    for module in sorted(module_deps):
        if module in HAND_WRITTEN_MODULES:
            continue
        # Engine modules the game module lists (they cover forward declarations of engine classes), plus this
        # module's own needs: engine modules of its headers and promoted modules of its parents.
        # Keep dependencies on FortniteGame when actually needed; the cycle check
        # above already refuses a reverse dependency from FortniteGame.
        own = sorted((set(engine_shared) | module_deps[module]) - {module})
        module_dir = os.path.join(args.source_root, module)
        os.makedirs(os.path.join(module_dir, "Private"), exist_ok=True)
        with open(os.path.join(module_dir, module + ".Build.cs"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(render_module_build_cs(module, own, source_label))
        with open(os.path.join(module_dir, "Private", module + ".cpp"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(render_module_cpp(module, source_label))

    type_files = []
    for fn, model in sorted(models.items(), key=lambda kv: (kv[1]["kind"], kv[1]["cpp_name"])):
        info = gate.info(fn)
        text = texts[info.header_name]
        if re.search(r"TODO_", text):
            raise SystemExit("%s: unresolved TODO token reached the output" % info.cpp_name)
        out_dir = os.path.join(args.source_root, info.module, "Generated")
        os.makedirs(out_dir, exist_ok=True)
        with open(os.path.join(out_dir, info.header_name), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        type_files.append({"type": info.cpp_name, "kind": info.kind, "package": info.package,
                           "header": info.header_name, "members_or_values": len(model.get("members") or model.get("values")
                           or model.get("params") or []),
                           "wire_custom": bool(info.kind == "struct" and gate.wire_custom(info)),
                           "struct_flags": model.get("flags"), "struct_ops": model.get("struct_ops")})
    not_emittable = []
    for fn, reason in sorted(gate.reasons.items()):
        info = gate.info(fn)
        if info is not None and info.package in PROMOTE_PACKAGES:
            not_emittable.append({"type": info.cpp_name, "kind": info.kind, "package": info.package, "reason": reason})
    type_summary = {
        "structs_emitted": sum(1 for t in type_files if t["kind"] == "struct"),
        "enums_emitted": sum(1 for t in type_files if t["kind"] == "enum"),
        "delegates_emitted": sum(1 for t in type_files if t["kind"] == "delegate"),
        "wire_custom_structs_emitted": sum(1 for t in type_files if t["wire_custom"]),
        "engine_types_included": len(engine_used),
        "promoted_types_not_emittable": len(not_emittable),
    }

    props = [r for r in all_skipped if r["item"] == "property"]
    funcs = [r for r in all_skipped if r["item"] == "function"]
    notify = [r for r in all_skipped if r["item"] == "rep_notify"]
    for record in props:
        record["property"] = record.pop("name")
    summary = {
        "types": type_summary,
        "classes": len(class_list),
        "properties_total": sum(r["properties_total"] for r in index),
        "properties_emitted": sum(r["properties_emitted"] for r in index),
        "properties_skipped": len(props),
        "replicated_total": sum(r["replicated_total"] for r in index),
        "replicated_emitted": sum(r["replicated_emitted"] for r in index),
        "replicated_skipped": sum(1 for r in props if r["replicated"]),
        "skipped_by_category": count_by(props, "category"),
        "skipped_by_kind": count_by(props, "kind"),
        "functions_skipped": len(funcs),
        "functions_skipped_by_category": count_by(funcs, "category"),
        "rep_notify_downgraded": len(notify),
        "rpc_in_reflection": sum(r["rpc_in_reflection"] for r in index),
        "rpc_emitted": sum(r["rpc_emitted"] for r in index),
    }
    os.makedirs(args.report_dir, exist_ok=True)
    with open(os.path.join(args.report_dir, "skipped_properties.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"summary": summary, "properties": props, "functions": funcs, "rep_notify": notify},
                  fh, indent=1)
        fh.write("\n")
    with open(os.path.join(args.report_dir, "native_classes_index.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"summary": summary, "classes": index}, fh, indent=1)
        fh.write("\n")
    with open(os.path.join(args.report_dir, "type_closure.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"summary": type_summary, "emitted": type_files, "engine_types": engine_used,
                   "not_emittable": not_emittable}, fh, indent=1)
        fh.write("\n")
    print(json.dumps(summary, indent=1))
    print("promoted %d classes into %s (param order: %s)" % (len(index), args.source_root, PARAM_ORDER),
          file=sys.stderr)
    if PARAM_ORDER_BREAKS:
        print("warning: %d parameter list(s) with offsets not in reverse table order kept in table order"
              % len(PARAM_ORDER_BREAKS), file=sys.stderr)
    return 0


# ---------------------------------------------------------------- struct, enum and delegate types (promote)
# The closure of types used by the promoted classes. Inputs: porting/extract_types.py output (struct_blocks.json,
# enum_blocks.json, delegate_blocks.json, construct_fn_to_block.json). Engine types (not in PROMOTE_PACKAGES) are
# referenced by name and included from the engine tree (EngineTypeIndex). Promoted types are emitted into the
# module's Generated directory, one header per type.

WIRE_CUSTOM_TRAITS = ("HasNetSerializer", "HasNetDeltaSerializer", "HasNetSharedSerialization")
# Behaviour kind "struct_net" (porting/behavior/README.md, "Native NetSerialize"): the recovered NetSerialize of a struct,
# declared and defined inline in the struct header (as the delta NetDeltaSerialize is). Only this signature is accepted,
# and only for a struct whose binary traits are a plain NetSerialize (HasNetSerializer, HasNetSharedSerialization optional).
STRUCT_NET_KIND = "struct_net"
NATIVE_NET_SIGNATURE = "bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)"
NATIVE_NET_HAS_SHARED = "HasNetSharedSerialization"
NATIVE_NET_SERIALIZATION_HEADER = "Serialization/Archive.h"   # FArchive is complete in the inline body
DELEGATE_MACRO = {"single": "DECLARE_DYNAMIC_DELEGATE", "multicast": "DECLARE_DYNAMIC_MULTICAST_DELEGATE",
                  "sparse": "DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE"}
PARAM_ORDINALS = ("", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine")
TYPE_DECL_RE = {
    "struct": re.compile(r"^\s*struct\s+(?:[A-Z][A-Z0-9_]*_API\s+)?(F[A-Za-z0-9_]+)\b[^;]*$", re.M),
    "enum": re.compile(r"^\s*enum\s+(?:class\s+|struct\s+)?(E[A-Za-z0-9_]+)\b[^;]*$", re.M),
    "delegate": re.compile(r"\bDECLARE_(?:DYNAMIC_)?(?:MULTICAST_)?(?:SPARSE_)?DELEGATE\w*\(\s*(F[A-Za-z0-9_]+)"),
}
ENGINE_ROOT_DEFAULT = os.environ.get("FN_ENGINE_ROOT", "/opt/fortnite/UnrealEngine/Engine")
# OnlineSubsystemUtils declares the party and spectator beacon state types (PartyBeaconState.h, SpectatorBeaconState.h)
# that the beacon RPCs use. Only that plugin is scanned: the rest of Plugins/Online would add headers of modules that
# the project's Build.cs does not list (WebBrowser, LoginFlow). Only usable modules are indexed (ModuleRegistry).
ENGINE_SCAN_DIRS = ("Source/Runtime", "Plugins/Runtime", "Plugins/Developer", "Source/Developer",
                    "Plugins/Online/OnlineSubsystemUtils")
# A namespaced enum (UE: `namespace ECollisionEnabled { enum Type {...}; }`) is named ECollisionEnabled::Type in C++.
# TYPE_DECL_RE only sees `enum E...` at the start of a line, so the namespaced form is indexed by this pattern.
NAMESPACE_ENUM_RE = re.compile(r"^\s*namespace\s+(E[A-Za-z0-9_]+)\s*\{?\s*enum\s+Type\b", re.M)
KIND_LABEL = {"struct": "Struct", "enum": "Enum", "delegate": "Delegate"}


def include_path(path):
    """Include spelling for a header: the part after its module's Public or Classes directory."""
    best = None
    for marker in ("/Public/", "/Classes/"):
        at = path.rfind(marker)
        if at >= 0 and (best is None or at > best[0]):
            best = (at, marker)
    if best is None:
        return None
    return path[best[0] + len(best[1]):].replace(os.sep, "/")


MODULE_SCAN_SKIP = ("Intermediate", "Binaries", "Content", "ThirdParty", "Saved", "DerivedDataCache", "Config")
PROJECT_NATIVE_DIR = os.path.join(REPO, "porting", "native")
UPROJECT_DEFAULT = os.path.join(PROJECT_NATIVE_DIR, "FortniteLinux.uproject")
BLOCKLIST_DEFAULT = os.path.join(REPO, "porting", "native_class_blocklist.txt")
FORTNITE_BUILD_CS = os.path.join(PROJECT_NATIVE_DIR, "Source", "FortniteGame", "FortniteGame.Build.cs")


class ModuleRegistry:
    """Build.cs modules of the engine and of the project, and which of them a promoted class may depend on.

    A module is usable when it is a Runtime module of the engine (Source/Runtime), a module of a plugin enabled in
    the project, or a project module outside a plugin. Only usable modules supply parents, included headers and
    forward-declared class names. A module that is not usable (for example a plugin the project does not enable)
    makes every reference to it a skip, not a compile error."""

    def __init__(self, engine_root, project_dir, uproject):
        enabled = set()
        if uproject and os.path.isfile(uproject):
            for plugin in load_json(uproject).get("Plugins", []):
                if plugin.get("Enabled"):
                    enabled.add(plugin.get("Name"))
        self.enabled_plugins = enabled
        self.engine_root = os.path.abspath(engine_root)
        self.modules = {}   # name -> {"dir", "usable", "why"}
        self.dirs = {}      # module directory -> name
        for root, origin in ((project_dir, "project"), (engine_root, "engine")):
            root = os.path.abspath(root)
            for sub in ("Source", "Plugins"):
                base = os.path.join(root, sub)
                if not os.path.isdir(base):
                    continue
                for dirpath, dirnames, filenames in os.walk(base):
                    dirnames[:] = sorted(d for d in dirnames if d not in MODULE_SCAN_SKIP)
                    for filename in filenames:
                        if not filename.endswith(".Build.cs"):
                            continue
                        name = filename[:-len(".Build.cs")]
                        if name in self.modules:
                            continue
                        usable, why = self._usable(dirpath, root, origin)
                        self.modules[name] = {"dir": dirpath, "usable": usable, "why": why}
                        self.dirs.setdefault(dirpath, name)

    def _usable(self, mdir, root, origin):
        cur = mdir
        while cur.startswith(root) and len(cur) > len(root):
            if any(f.endswith(".uplugin") for f in os.listdir(cur)):
                plugin = os.path.basename(cur)
                if plugin in self.enabled_plugins:
                    return True, "enabled plugin %s" % plugin
                return False, "plugin %s is not enabled in the project" % plugin
            cur = os.path.dirname(cur)
        if origin == "project":
            return True, "project module"
        rel = os.path.relpath(mdir, root).replace(os.sep, "/")
        if rel.startswith("Source/Runtime/"):
            return True, "engine runtime module"
        return False, "engine module outside Source/Runtime (%s)" % rel

    def module_of_path(self, path):
        cur = os.path.dirname(os.path.abspath(path))
        while True:
            if cur in self.dirs:
                return self.dirs[cur]
            parent = os.path.dirname(cur)
            if parent == cur:
                return None
            cur = parent

    def usable(self, module):
        info = self.modules.get(module)
        return bool(info and info["usable"])

    def usable_package(self, package):
        return bool(package) and self.usable(package.rsplit("/", 1)[-1])


class ModuleHeaderIndex:
    """Class name -> include spelling (relative to the module's Classes or Public directory), per usable module."""

    def __init__(self, registry):
        self.registry = registry
        self.cache = {}

    def find(self, module, class_name):
        if module not in self.registry.modules or not self.registry.usable(module):
            return None
        if module not in self.cache:
            index = {}
            base = self.registry.modules[module]["dir"]
            for sub in ("Classes", "Public"):
                root = os.path.join(base, sub)
                if os.path.isdir(root):
                    for name, rel in build_engine_header_index(root).items():
                        index.setdefault(name, rel)
            self.cache[module] = index
        return self.cache[module].get(class_name)


def load_blocklist(path):
    """Class name -> reason for porting/native_class_blocklist.txt (one 'Name<TAB>reason' line each)."""
    out = {}
    if not path or not os.path.isfile(path):
        return out
    with open(path, "r", encoding="utf-8") as fh:
        for line in fh:
            text = line.split("#", 1)[0].rstrip("\n")
            if not text.strip():
                continue
            name, _, reason = text.partition("\t")
            out[name.strip()] = reason.strip() or "no reason given"
    return out


def build_cs_modules(path):
    """Module names listed in a Build.cs (every quoted identifier; the file only lists dependencies this way)."""
    with open(path, "r", encoding="utf-8") as fh:
        return set(re.findall(r'"([A-Za-z0-9_]+)"', fh.read()))


def build_cs_public_modules(path):
    """Extract only literal public dependency additions; reject unsupported syntax.

    The hand-written game Build.cs may also have conditional private dependencies
    that must not become unconditional public dependencies of generated modules.
    """
    with open(path, "r", encoding="utf-8") as fh:
        text = fh.read()
    # Preserve quoted strings while removing comments, so a quoted module name in
    # a comment cannot be mistaken for a dependency. Keep offsets stable.
    token = re.compile(r'"(?:\\.|[^"\\])*"|//[^\r\n]*|/\*[\s\S]*?\*/')
    clean = token.sub(lambda m: m.group(0) if m.group(0).startswith('"') else
                      ''.join('\n' if c == '\n' else ' ' for c in m.group(0)), text)
    masked = token.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m.group(0)), clean)
    mentions = list(re.finditer(r'\bPublicDependencyModuleNames\b', masked))
    add = re.compile(r'PublicDependencyModuleNames\s*\.\s*Add\s*\(\s*"([A-Za-z0-9_]+)"\s*\)\s*;')
    add_range = re.compile(r'PublicDependencyModuleNames\s*\.\s*AddRange\s*\(\s*new\s*(?:string\s*)?\[\s*\]\s*\{([^{}]*)\}\s*\)\s*;', re.S)
    entries = re.compile(r'\s*(?:"[A-Za-z0-9_]+"\s*(?:,\s*"[A-Za-z0-9_]+"\s*)*,?)?\s*', re.S)
    modules = set()
    for mention in mentions:
        match = add.match(clean, mention.start())
        if match:
            modules.add(match.group(1))
            continue
        match = add_range.match(clean, mention.start())
        if not match or not entries.fullmatch(match.group(1)):
            raise ValueError("unsupported PublicDependencyModuleNames syntax in %s" % path)
        modules.update(re.findall(r'"([A-Za-z0-9_]+)"', match.group(1)))
    if not mentions:
        raise ValueError("no literal PublicDependencyModuleNames in %s" % path)
    return modules


class EngineTypeIndex:
    """First declaration of each struct (F...), enum (E...) and delegate type in the engine tree, Runtime first.

    With a ModuleRegistry, only headers of usable modules are indexed. module_of_inc maps each include spelling
    to its module, so the generator can check that the module is a dependency of the project."""

    def __init__(self, root, registry=None):
        self.index = {"struct": {}, "enum": {}, "delegate": {}}
        self.module_of_inc = {}
        self.files = 0
        if not os.path.isdir(root):
            return
        for sub in ENGINE_SCAN_DIRS:
            base = os.path.join(root, sub)
            if not os.path.isdir(base):
                continue
            for dirpath, dirnames, filenames in os.walk(base):
                dirnames[:] = sorted(d for d in dirnames if d not in ("Intermediate", "Binaries", "Content"))
                for filename in sorted(filenames):
                    if not filename.endswith(".h"):
                        continue
                    path = os.path.join(dirpath, filename)
                    module = None
                    if registry is not None:
                        module = registry.module_of_path(path)
                        if module is None or not registry.usable(module):
                            continue
                    inc = include_path(path)
                    if inc is None:
                        continue
                    with open(path, "r", encoding="utf-8", errors="replace") as fh:
                        text = fh.read()
                    self.files += 1
                    if module is not None:
                        self.module_of_inc.setdefault(inc, module)
                    for kind, rx in TYPE_DECL_RE.items():
                        for match in rx.finditer(text):
                            self.index[kind].setdefault(match.group(1), inc)
                    for match in NAMESPACE_ENUM_RE.finditer(text):
                        self.index["enum"].setdefault(match.group(1) + "::Type", inc)

    def lookup(self, kind, cpp_name):
        return self.index[kind].get(cpp_name)


class TypeInfo:
    def __init__(self, kind, fn, rec, cpp_name, package, header_name):
        self.kind = kind
        self.fn = fn
        self.rec = rec
        self.cpp_name = cpp_name
        self.package = package
        self.header_name = header_name
        self.module = package.rsplit("/", 1)[-1] if package else None


class TypeTables:
    """Struct, enum and delegate records keyed by their construct function (lower-case 0x address)."""

    def __init__(self, types_dir):
        cmap = load_json(os.path.join(types_dir, "construct_fn_to_block.json"))["map"]
        recs = {
            "struct": {r["block_va"].lower(): r for r in load_json(os.path.join(types_dir, "struct_blocks.json"))["structs"]},
            "enum": {r["block_va"].lower(): r for r in load_json(os.path.join(types_dir, "enum_blocks.json"))["enums"]},
            "delegate": {r["block_va"].lower(): r for r in load_json(os.path.join(types_dir, "delegate_blocks.json"))["delegates"]},
        }
        self.by_fn = {}
        for fn, hit in cmap.items():
            kind, block = hit["kind"], hit["block_va"].lower()
            fn = fn.lower()
            rec = recs.get(kind, {}).get(block)
            if rec is None:
                continue
            if kind == "struct":
                cpp, package = "F" + rec["name"], rec.get("package")
            elif kind == "enum":
                # A namespaced enum is named Namespace::Type in C++; its header is named for the namespace.
                cpp, package = rec["name"], rec.get("package")
                header = cpp.split("::")[0] + ".h"
            else:
                cpp, package = rec["type_name"], rec.get("package")
            if kind != "enum":
                header = cpp + ".h"
            self.by_fn[fn] = TypeInfo(kind, fn, rec, cpp, package, header)


def observed_confirmation_action(info):
    """Validate the complete original Blueprint-visible struct before enabling its UHT representation."""
    expected = [
        ('ActionName', 'Name', '0x14', 168, None),
        ('Icon', 'Struct', '0x19', 32, {'struct': '0x142fdb590'}),
        ('ResultName', 'Name', '0x14', 24, None),
        ('DisplayName', 'Text', '0x1d', 0, None),
    ]
    rec = info.rec
    if (info.kind != 'struct' or info.fn != '0x142b25040' or info.cpp_name != 'FConfirmationDialogAction' or
            info.package != '/Script/FortniteGame' or rec.get('name') != 'ConfirmationDialogAction' or
            rec.get('size') != 176 or rec.get('align') != 8 or rec.get('super_fn') is not None or
            len(rec.get('properties', [])) != len(expected)):
        return False
    for index, (prop, (name, kind, kind_raw, offset, refs)) in enumerate(zip(rec['properties'], expected)):
        if (prop.get('index') != index or prop.get('name') != name or prop.get('kind') != kind or
                prop.get('kind_raw') != kind_raw or prop.get('offset') != offset or
                hx(prop.get('property_flags')) != 0x0010000000000004 or prop.get('array_dim') != 1 or
                prop.get('object_flags') != '0x45' or prop.get('replicated') is not False or
                prop.get('type_refs') != refs):
            return False
    return True


class TypeGate:
    """Decides whether a struct, enum or delegate type reference can be emitted, and builds its model.

    check() is called by Resolver.type_ref. A promoted type is usable when it is in the current module and its
    model builds (every member type resolves). An engine type is usable when its header is in the engine tree.
    Models are memoized, so a type is built once however many items use it."""

    def __init__(self, tables, engine_index, base):
        self.tables = tables
        self.engine = engine_index
        self.base = base            # the promote-mode Resolver (type map, known functions); forked for checks
        self.current_package = None
        self.models = {}            # fn -> model dict, or None when the type cannot be emitted
        self.reasons = {}           # fn -> reason string when the type cannot be emitted
        self._wire = {}
        self.name_clashes = set()   # promoted type names the engine declares through macros (see engine_macro_names)
        # Behaviour kind struct_net (see native_net_serializer). Set by promote when the splice is on.
        self.struct_net = {}          # struct C++ name -> manifest entry
        self.struct_net_reasons = {}  # struct C++ name -> why its entry is not spliced
        self.struct_ctx = None        # the snippet context of body_refs (names, modules, the gate)
        self._building = set()        # fn of the types whose model is being built (see reason)

    def info(self, fn):
        return self.tables.by_fn.get(fn.lower()) if fn else None

    def include_of(self, fn):
        info = self.info(fn)
        if info is None:
            return None
        if info.package in PROMOTE_PACKAGES:
            return info.header_name
        return self.engine.lookup(info.kind, info.cpp_name)

    def check(self, resolver, kind, fn, name, entry_kind):
        info = self.info(fn)
        label = KIND_LABEL[kind]
        if info is None:
            return resolver.todo(label + "Unknown", fn)
        if kind == "delegate" and entry_kind == "SparseMulticastDelegate":
            # The sparse macro names the owning class and the property, which the reflection does not give.
            return resolver.todo("DelegateSparse", fn)
        resolver.used.setdefault(info.fn, set()).add(entry_kind or "")
        if info.package in PROMOTE_PACKAGES and info.cpp_name in self.name_clashes:
            # The engine declares this global name through a macro (token pasting), so the name cannot be included
            # next to the engine header that declares it. Not emitted.
            return resolver.todo(label + "NameClash", fn)
        if info.package in PROMOTE_PACKAGES:
            if (info.package != self.current_package and not cross_module_allowed(self.current_package, info.package)
                    and getattr(resolver, "observed_event_type_packages", {}).get(info.fn) != info.package):
                return resolver.todo(label + "OtherModule", fn)
            if self.reason(info) is not None:
                return resolver.todo(label + "NotEmittable", fn)
            if kind == "struct" and self.wire_custom(info):
                resolver.wire_used = True
            return info.cpp_name
        if self.include_of(info.fn) is None:
            return resolver.todo(label + "Unavailable", fn)
        return info.cpp_name

    def reason(self, info):
        """None when the type can be emitted, else why not."""
        if info.fn in self.models:
            return self.reasons.get(info.fn)
        if info.fn in self._building:
            # A cycle through the type names of a native NetSerialize body (native_net_serializer). A by-value member
            # cycle is impossible in C++, so the type in progress is taken as emittable; its own check decides.
            return None
        self._building.add(info.fn)
        try:
            if info.kind == "struct":
                model, reason = self.build_struct(info)
            elif info.kind == "enum":
                model, reason = self.build_enum(info)
            else:
                model, reason = self.build_delegate(info)
        finally:
            self._building.discard(info.fn)
        self.models[info.fn] = model
        if reason is not None:
            self.reasons[info.fn] = reason
        return reason

    def model(self, info):
        """The emitted model (only after reason() returned None)."""
        self.reason(info)
        return self.models[info.fn]

    def _child(self, info):
        child = self.base.fork()
        child.gate = self
        return child

    def build_struct(self, info):
        saved = self.current_package
        self.current_package = info.package
        try:
            child = self._child(info)
            reason = None
            observed_blueprint = info.cpp_name == "FConfirmationDialogAction"
            if observed_blueprint and not observed_confirmation_action(info):
                return None, "original ConfirmationDialogAction Blueprint struct identity or fields differ"
            if any("error" in p for p in info.rec["properties"]):
                reason = "unreadable property record"
            parent = None
            if reason is None and info.rec.get("super_fn"):
                parent = child.ctype({"kind": "Struct", "type_refs": {"struct": info.rec["super_fn"]}}, {})
                if child.tokens:
                    reason = "parent struct unresolved (%s)" % ", ".join(sorted(set(child.tokens)))
            members = []
            if reason is None:
                members, _notes = build_members(info.rec["properties"], child, promote=True, struct_mode=True)
                if child.skipped:
                    first = child.skipped[0]
                    reason = "member %s: %s" % (first.get("name"), first.get("reason"))
            if reason is not None:
                return None, reason
            # Declaration order: the table lists members in reverse declaration order (see build_class).
            decl = list(reversed(members))
            # The parent is a use too: its header is included and, when promoted, emitted (see emit_types).
            parent_used = {info.rec["super_fn"].lower(): {""}} if info.rec.get("super_fn") else {}
            model = {"fn": info.fn, "cpp_name": info.cpp_name, "kind": "struct", "package": info.package,
                     "parent": parent, "members": decl,
                     "used": merge_used([m["used"] for m in members] + [parent_used]),
                     "forward_decls": sorted(child.class_refs - {info.cpp_name, parent}),
                     "flags": info.rec.get("struct_flag_names", []), "size": info.rec.get("size"),
                     "align": info.rec.get("align"), "struct_ops": info.rec.get("struct_ops"),
                     "net": self.net_serializer(info, members)}
            if observed_blueprint:
                model["blueprint_type"] = True
                for member in model["members"]:
                    member["blueprint_read_write"] = True
            return model, None
        finally:
            self.current_package = saved

    def net_serializer(self, info, members):
        """How a struct's native net serializer is emitted, or None when it has none (see WIRE_CUSTOM_TRAITS).

        {"kind": "delta", ...}: an FFastArraySerializer whose TArray of FFastArraySerializerItem structs is one of its
        members. Its NetDeltaSerialize is FFastArraySerializer::FastArrayDeltaSerialize over that array (identity:
        the decompiled NetDeltaSerialize of 45 Fort structs, which contains the FastArrayDeltaSerialize log strings,
        porting/decompilation_notes.md "Sixth pass"). Emitted with WithNetDeltaSerializer.
        {"kind": "native", ...}: a struct with a recovered NetSerialize body (struct_net manifest entry, see
        native_net_serializer). The body is declared inline in the header with the binary's traits.
        {"kind": "fallback", ...}: any other struct with a custom NetSerialize or NetDeltaSerialize. The native body is
        not recovered, so the struct keeps the default serialization and the header says so (TODO)."""
        ops = info.rec.get("struct_ops") or {}
        traits = ops.get("traits") or {}
        custom = sorted(t for t in WIRE_CUSTOM_TRAITS if traits.get(t) is True)
        if not custom:
            return None
        vtable = ops.get("vtable_va")
        if traits.get("HasNetDeltaSerializer") is True and info.package in PROMOTE_PACKAGES:
            parent = self.info(info.rec.get("super_fn"))
            member_names = {m["name"] for m in members}
            if parent is not None and parent.cpp_name == "FFastArraySerializer":
                for prop in info.rec.get("properties") or []:
                    if prop.get("kind") != "Array" or int(prop.get("array_dim") or 1) != 1:
                        continue
                    inner = next((q for q in info.rec.get("properties") or []
                                  if q.get("array_inner_of") == prop.get("name") and q.get("kind") == "Struct"), None)
                    if inner is None or prop.get("name") not in member_names:
                        continue
                    item = self.info((inner.get("type_refs") or {}).get("struct"))
                    item_parent = self.info(item.rec.get("super_fn")) if item is not None and item.kind == "struct" else None
                    if item_parent is not None and item_parent.cpp_name == "FFastArraySerializerItem":
                        return {"kind": "delta", "member": prop["name"], "item": item.cpp_name,
                                "parent": "FFastArraySerializer", "traits": custom, "vtable": vtable}
        native = self.native_net_serializer(info, custom, vtable)
        if native is not None:
            return native
        return {"kind": "fallback", "traits": custom, "vtable": vtable,
                "reason": "native net serializer not recovered"}

    def native_net_serializer(self, info, custom, vtable):
        """{"kind": "native", ...} when the struct's struct_net manifest entry (recovered NetSerialize body) can be
        spliced, else None with the reason in struct_net_reasons. The binary must have a plain NetSerialize:
        HasNetSerializer, with HasNetSharedSerialization optional (the trait is read from the binary, not from the
        manifest). The signature must be NATIVE_NET_SIGNATURE, and the body may name only types that the generator can
        include (body_refs) and emit (body_types)."""
        entry = self.struct_net.get(info.cpp_name)
        if entry is None or self.struct_ctx is None:
            return None
        reason, includes = None, set()
        if info.package not in PROMOTE_PACKAGES:
            reason = "struct is not in a promoted package"
        elif entry.get("body") is None:
            reason = entry.get("reject") or "no body"
        elif "HasNetSerializer" not in custom or not set(custom) <= {"HasNetSerializer", NATIVE_NET_HAS_SHARED}:
            reason = "binary traits (%s) are not a plain NetSerialize" % ", ".join(custom)
        elif squash(entry.get("signature")) != squash(NATIVE_NET_SIGNATURE):
            reason = "manifest signature %r differs from %r" % (entry.get("signature"), NATIVE_NET_SIGNATURE)
        else:
            includes, problems = body_refs(entry["body"], info.cpp_name, info.package, self.struct_ctx)
            uses = self.body_types(entry["body"], info)
            for used in uses:
                if self.reason(used) is not None:
                    problems.append("body uses %s, which cannot be emitted (%s)"
                                    % (used.cpp_name, self.reasons.get(used.fn)))
            if problems:
                reason = "; ".join(problems)
        if reason is not None:
            self.struct_net_reasons[info.cpp_name] = reason
            return None
        # "uses": the promoted types the body names. emit_types adds them to the closure, so their headers exist.
        return {"kind": "native", "body": entry["body"], "inc_path": entry.get("inc_path"), "traits": custom,
                "vtable": vtable, "includes": sorted(includes), "uses": [u.fn for u in uses]}

    def body_types(self, body, info):
        """The promoted types (of any promoted package that is in the struct_index) that a native body names, other
        than the struct itself. Each is emitted with the struct when it can be."""
        index = self.struct_ctx["struct_index"]
        out = []
        for tok in sorted(set(re.findall(r"[A-Za-z_]\w*", strip_comments(body)))):
            used = index.get(tok)
            if used is not None and used.package in PROMOTE_PACKAGES and used.fn != info.fn:
                out.append(used)
        return out

    def build_enum(self, info):
        rec = info.rec
        values, seen = [], set()
        for item in rec["enumerators"]:
            ident = (item.get("name") or "").split("::")[-1]
            if not re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", ident) or ident in seen:
                return None, "enumerator name %r" % item.get("name")
            seen.add(ident)
            values.append((ident, int(item["value"])))
        if not values:
            return None, "no enumerators"
        model = {"fn": info.fn, "cpp_name": info.cpp_name, "kind": "enum", "package": info.package,
                 "form": rec.get("cpp_form"), "values": values, "used": {}}
        return model, None

    def build_delegate(self, info):
        saved = self.current_package
        self.current_package = info.package
        try:
            child = self._child(info)
            reason, args = None, []
            pairs, _notes = split_properties(info.rec.get("params") or [])
            offsets = []
            for entry, children in pairs:
                if "error" in entry:
                    reason = "unreadable parameter record"
                    break
                pf = hx(entry.get("property_flags"))
                if PARAM_ORDER == "table":
                    if pf & (CPF_RETURN_PARM | CPF_OUT_PARM | CPF_REFERENCE_PARM):
                        reason = "return, out or reference parameter"
                        break
                    args.append((child.ctype(entry, children), entry.get("name") or "Param%d" % len(args)))
                    continue
                # UHT sets CPF_OutParm and CPF_ReferenceParm on every reference parameter, so a const reference
                # (`const T&`) carries all three flags. It is emitted as `const T&`, the same spelling the source has.
                # A return parameter, or an out or non-const reference parameter, is not a delegate signature.
                const_ref = bool(pf & CPF_REFERENCE_PARM and pf & CPF_OUT_PARM and pf & CPF_CONST_PARM)
                if pf & CPF_RETURN_PARM or (pf & (CPF_OUT_PARM | CPF_REFERENCE_PARM) and not const_ref):
                    reason = "return, out or non-const reference parameter"
                    break
                ctype = child.ctype(entry, children)
                if const_ref:
                    ctype = ctype + " const&" if ctype.endswith("*") else "const " + ctype + "&"
                if "," in ctype:
                    # A comma would split the macro argument.
                    reason = "parameter type with a comma (%s)" % ctype
                    break
                if entry.get("offset") is not None:
                    offsets.append(int(entry["offset"]))
                args.append((ctype, entry.get("name") or "Param%d" % len(args)))
            if reason is None and len(args) > 9:
                reason = "more than nine parameters"
            if reason is None and child.tokens:
                reason = "parameter type unresolved (%s)" % ", ".join(sorted(set(child.tokens)))
            if reason is None and PARAM_ORDER == "offset" and any(b >= a for a, b in zip(offsets, offsets[1:])):
                reason = "parameter offsets not in reverse table order"
            if reason is not None:
                return None, reason
            if PARAM_ORDER == "offset":
                # The table lists the parameters in reverse declaration order (UHT emits PropPointers from the last
                # declared parameter back, and the parameter block offsets ascend in declaration order). The macro
                # takes them in declaration order, so the Parms block lays out at the recorded offsets.
                args.reverse()
            model = {"fn": info.fn, "cpp_name": info.cpp_name, "kind": "delegate", "package": info.package,
                     "params": args, "used": merge_used([child.used]),
                     "multicast": bool(info.rec.get("multicast"))}
            return model, None
        finally:
            self.current_package = saved

    def wire_custom(self, info):
        """A struct whose serialization is custom (NetSerialize, NetDeltaSerialize or shared serialization), or that
        contains one. Engine structs are not counted: their NetSerialize is engine code, not recovered here."""
        if info.fn in self._wire:
            return self._wire[info.fn]
        self._wire[info.fn] = False
        value = False
        if info.package in PROMOTE_PACKAGES and info.kind == "struct":
            traits = (info.rec.get("struct_ops") or {}).get("traits") or {}
            value = any(traits.get(t) is True for t in WIRE_CUSTOM_TRAITS)
            if not value:
                for p in info.rec["properties"]:
                    fn = (p.get("type_refs") or {}).get("struct")
                    sub = self.info(fn)
                    if sub is not None and sub.package in PROMOTE_PACKAGES and self.wire_custom(sub):
                        value = True
                        break
        self._wire[info.fn] = value
        return value


def merge_used(dicts):
    out = {}
    for d in dicts:
        for fn, kinds in (d or {}).items():
            out.setdefault(fn, set()).update(kinds)
    return out


def delegate_macro_kind(model, kinds):
    """single (DECLARE_DYNAMIC_DELEGATE), multicast or sparse, from the flags and the property kinds that use it."""
    if not model["multicast"]:
        return "single"
    return "sparse" if "SparseMulticastDelegate" in kinds else "multicast"


NET_SERIALIZATION_HEADER = "Engine/NetSerialization.h"   # FFastArraySerializer, FNetDeltaSerializeInfo, struct ops traits


def native_net_body_lines(body):
    """A struct_net body as emitted inside the struct: two tabs (the declaration is at one tab)."""
    return [("\t\t" + line.rstrip()) if line.strip() else "" for line in body.split("\n")]


def verify_struct_net(model, header):
    """Text check of a native NetSerialize in its rendered struct header: the declaration once, inside the struct (before
    its closing '};'), the body equal to the snippet, and the trait block once. Raises SystemExit on a mismatch, before
    anything is written."""
    name = model["cpp_name"]
    net = model["net"]
    lines = header.split("\n")
    problems = []
    decl = "\t%s" % NATIVE_NET_SIGNATURE
    where = [i for i, line in enumerate(lines) if line == decl]
    if len(where) != 1:
        problems.append("NetSerialize declared %d times" % len(where))
    else:
        i = where[0]
        expected = ["\t{"] + native_net_body_lines(net["body"]) + ["\t}"]
        if lines[i + 1:i + 1 + len(expected)] != expected:
            problems.append("body differs from the snippet")
        close = lines.index("};") if "};" in lines else None
        if close is None or not i < close:
            problems.append("declared outside the struct")
    trait = "struct TStructOpsTypeTraits<%s> : public TStructOpsTypeTraitsBase2<%s>" % (name, name)
    if sum(1 for line in lines if line == trait) != 1:
        problems.append("trait block missing or repeated")
    if "class UPackageMap;" not in lines:
        problems.append("UPackageMap is not forward declared")
    if problems:
        raise SystemExit("%s: native NetSerialize is not spliced as written: %s" % (name, "; ".join(problems)))


def render_struct_header(model, source_label):
    name = model["cpp_name"]
    net = model.get("net") or {}
    delta = net.get("kind") == "delta"
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// Do not edit by hand: run porting/promote_native_classes.sh. Members are in declaration order (offsets",
        "// ascending). " + ("A struct with a custom NetSerialize is emitted here only when it is not replicated."
                            if not net else
                            "The custom net serializer of this struct is declared below."),
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
    ]
    if model.get("parent_include"):
        lines.append("#include \"%s\"" % model["parent_include"])
    native = net.get("kind") == "native"
    includes = list(model.get("includes", []))
    if delta and NET_SERIALIZATION_HEADER not in includes + [model.get("parent_include")]:
        includes.append(NET_SERIALIZATION_HEADER)
    if native and NATIVE_NET_SERIALIZATION_HEADER not in includes:
        includes.append(NATIVE_NET_SERIALIZATION_HEADER)   # the inline NetSerialize body uses FArchive members
    for inc in includes:
        if inc != model.get("parent_include"):
            lines.append("#include \"%s\"" % inc)
    lines.append("")
    forward = list(model["forward_decls"])
    if native and "UPackageMap" not in forward:
        forward.append("UPackageMap")   # the NetSerialize signature takes class UPackageMap*
    lines.extend("class %s;" % ref for ref in forward)
    if forward:
        lines.append("")
    lines.append("#include \"%s.generated.h\"" % name)
    lines.append("")
    if net.get("kind") == "fallback":
        # TODO: the native NetSerialize is not recovered (ops vtable %s). Replicated uses serialize with the default
        # property serialization, which differs from the original wire format.
        lines.append("// TODO: the native net serializer of %s is not recovered (ops vtable %s; %s). The default"
                     % (name, net.get("vtable"), ", ".join(net.get("traits", []))))
        lines.append("// serialization is used until it is ported.")
    lines.append("USTRUCT(BlueprintType)" if model.get("blueprint_type") else "USTRUCT()")
    lines.append("struct %s%s" % (name, " : public %s" % model["parent"] if model["parent"] else ""))
    lines.append("{")
    lines.append("\tGENERATED_BODY()")
    lines.append("")
    for m in model["members"]:
        lines.append("\t// table index %s, offset %s, flags %s" % (m["table_index"], m["offset"], m["property_flags"]))
        lines.append("\tUPROPERTY(BlueprintReadWrite)" if m.get("blueprint_read_write") else "\tUPROPERTY()")
        lines.append("\t%s" % m["decl"])
        lines.append("")
    if delta:
        item = net["item"]
        lines.append("\t// Native NetDeltaSerialize, the FFastArraySerializer pattern (ops vtable %s, traits %s): the"
                     % (net.get("vtable"), ", ".join(net.get("traits", []))))
        lines.append("\t// decompiled function calls FFastArraySerializer::FastArrayDeltaSerialize over %s."
                     % net["member"])
        lines.append("\tbool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)")
        lines.append("\t{")
        lines.append("\t\treturn FFastArraySerializer::FastArrayDeltaSerialize<%s, %s>(%s, DeltaParms, *this);"
                     % (item, name, net["member"]))
        lines.append("\t}")
        lines.append("")
    if native:
        # Native NetSerialize (behaviour kind struct_net): declared and defined inline, the body one level deeper than the
        # declaration. verify_struct_net checks the text.
        lines.append("\t// Native NetSerialize (behaviour kind %s, body %s; ops traits %s)."
                     % (STRUCT_NET_KIND, net.get("inc_path"), ", ".join(net.get("traits", []))))
        lines.append("\t%s" % NATIVE_NET_SIGNATURE)
        lines.append("\t{")
        lines.extend(native_net_body_lines(net["body"]))
        lines.append("\t}")
        lines.append("")
    lines.append("};")
    lines.append("")
    if delta:
        lines.append("template<>")
        lines.append("struct TStructOpsTypeTraits<%s> : public TStructOpsTypeTraitsBase2<%s>" % (name, name))
        lines.append("{")
        lines.append("\tenum { WithNetDeltaSerializer = true };")
        lines.append("};")
        lines.append("")
    if native:
        flags = ["WithNetSerializer = true"]
        if NATIVE_NET_HAS_SHARED in net.get("traits", []):
            flags.append("WithNetSharedSerialization = true")
        lines.append("template<>")
        lines.append("struct TStructOpsTypeTraits<%s> : public TStructOpsTypeTraitsBase2<%s>" % (name, name))
        lines.append("{")
        lines.append("\tenum { %s };" % ", ".join(flags))
        lines.append("};")
        lines.append("")
    return "\n".join(lines) + "\n"


def render_enum_header(model, source_label):
    """EnumClass: UENUM() enum class E : uint8. Namespaced: UENUM() namespace N { enum Type }. Regular: UENUM() enum E."""
    name = model["cpp_name"]
    namespace, _sep, type_name = name.partition("::")
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// Do not edit by hand: run porting/promote_native_classes.sh. Values come from the exe. Enumerator names",
        "// have the EnumName:: prefix removed. The underlying type of a UENUM is uint8 (UHT rule, not in the exe).",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
        "#include \"%s.generated.h\"" % namespace,
        "",
        "UENUM()",
    ]
    if model["form"] == "Namespaced" and type_name:
        lines += ["namespace %s" % namespace, "{", "\tenum %s" % type_name, "\t{"]
        indent = "\t\t"
        closing = ["\t};", "}"]
    else:
        lines.append("enum class %s : uint8" % name if model["form"] == "EnumClass" else "enum %s" % name)
        lines.append("{")
        indent = "\t"
        closing = ["};"]
    for ident, value in model["values"]:
        lines.append("%s%s = %d," % (indent, ident, value))
    lines.extend(closing)
    lines.append("")
    return "\n".join(lines) + "\n"


def render_delegate_header(model, macro_kind, source_label):
    name = model["cpp_name"]
    args = model["params"]
    n = len(args)
    if n == 0:
        macro = "%s(%s);" % (DELEGATE_MACRO[macro_kind], name)
    else:
        macro = "%s_%sParam%s(%s);" % (DELEGATE_MACRO[macro_kind], PARAM_ORDINALS[n], "" if n == 1 else "s",
                                      ", ".join([name] + ["%s, %s" % (t, a) for t, a in args]))
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// Do not edit by hand: run porting/promote_native_classes.sh. The type name is the signature name without",
        "// its __DelegateSignature suffix, with the F prefix (the name the property table does not give).",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
    ]
    for inc in model.get("includes", []):
        lines.append("#include \"%s\"" % inc)
    # UnrealBuildTool passes a header to UHT only when it has reflection markup at the start of a line
    # (SourceFileMetadataCache: ^\s*U(CLASS|STRUCT|ENUM|INTERFACE|DELEGATE)\b). A delegate header therefore
    # needs UDELEGATE() and its own generated header, as the struct and enum renderers have.
    lines.append("#include \"%s.generated.h\"" % name)
    lines.append("")
    lines.append("UDELEGATE()")
    lines.append(macro)
    lines.append("")
    return "\n".join(lines) + "\n"


def emit_types(gate, seeds, source_label):
    """Models and header texts for every promoted type reachable from seeds (fn -> used kinds). Engine types are
    not emitted. Returns (models by fn, texts by header name, all used kinds by fn)."""
    models = {}
    all_used = {}
    queue = list(seeds.items())
    while queue:
        fn, kinds = queue.pop()
        info = gate.info(fn)
        if info is None:
            continue
        all_used.setdefault(info.fn, set()).update(kinds)
        if info.package not in PROMOTE_PACKAGES or info.fn in models:
            continue
        model = gate.model(info)
        model["includes"] = own_includes(gate, model["used"], info.package)
        if (model.get("net") or {}).get("kind") == "native":
            # Types the native NetSerialize body names (TypeGate.native_net_serializer) are included too.
            model["includes"] = sorted(set(model["includes"]) | set(model["net"]["includes"]))
        if info.kind == "struct" and model["parent"] is not None:
            parent_fn = info.rec.get("super_fn")
            model["parent_include"] = gate.include_of(parent_fn) if parent_fn else None
        models[info.fn] = model
        queue.extend(model["used"].items())
        if (model.get("net") or {}).get("kind") == "native":
            queue.extend((fn, {""}) for fn in model["net"]["uses"])   # types the native body names
    texts = {}
    for fn, model in models.items():
        info = gate.info(fn)
        if info.kind == "struct":
            texts[info.header_name] = render_struct_header(model, source_label)
        elif info.kind == "enum":
            texts[info.header_name] = render_enum_header(model, source_label)
        else:
            macro_kind = delegate_macro_kind(model, all_used.get(info.fn, set()))
            texts[info.header_name] = render_delegate_header(model, macro_kind, source_label)
    return models, texts, all_used


def load_json(path):
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--reflection", default=DEFAULT_REFLECTION)
    ap.add_argument("--name-map", default=None, help="JSON {block_va: class_name}")
    ap.add_argument("--type-map", default=None, help="JSON {fn_va: C++ type name} for Z_Construct functions")
    ap.add_argument("--parent-map", default=None,
                    help="JSON from resolve_class_parents.py {block_va: {parent_name, parent_va, ...}}")
    ap.add_argument("--out", default=DEFAULT_OUT)
    ap.add_argument("--api", default="FORTNITEGAME_API", help="API export macro; empty for none")
    ap.add_argument("--order", choices=("declaration", "table"), default="declaration")
    ap.add_argument("--only", default=None, help="generate only this class name")
    ap.add_argument("--compare-header", default=None)
    ap.add_argument("--compare-class", default=None)
    ap.add_argument("--report", default=None, help="write the comparison report to this path")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--promote", action="store_true",
                    help="write the native set into --source-root/<Module>/Generated (see promote_native_classes.sh)")
    ap.add_argument("--class-list", default=None, help="--promote: closure list, one class per line")
    ap.add_argument("--evidence", default=None, help="--promote: class_name_map.evidence.json (construct functions)")
    ap.add_argument("--engine-classes", default=None, help="--promote: engine Runtime/Engine/Classes directory")
    ap.add_argument("--source-root", default=None, help="--promote: native/Source directory")
    ap.add_argument("--report-dir", default=None, help="--promote: where the skip and index reports are written")
    ap.add_argument("--class-ref-map", default=None, help="--promote: class_ref_map.json from resolve_class_refs.py")
    ap.add_argument("--types-dir", default=None,
                    help="--promote: directory with struct_blocks.json, enum_blocks.json, delegate_blocks.json and "
                         "construct_fn_to_block.json (default: the reflection file's directory)")
    ap.add_argument("--blocklist", default=BLOCKLIST_DEFAULT,
                    help="--promote: 'Name<TAB>reason' lines for classes that must not be generated")
    ap.add_argument("--engine-root", default=ENGINE_ROOT_DEFAULT,
                    help="--promote: UnrealEngine/Engine directory searched for struct, enum and delegate headers")
    ap.add_argument("--replication-conditions", default=CONDITIONS_DEFAULT,
                    help="--promote: JSON of recovered replication conditions (porting/replication_conditions.json); "
                         "a missing file gives plain DOREPLIFETIME")
    ap.add_argument("--behavior-dir", default=BEHAVIOR_DEFAULT,
                    help="--promote: porting/behavior directory (MANIFEST.json and the .inc snippets)")
    ap.add_argument("--no-behavior", action="store_true",
                    help="--promote: do not splice the behaviour snippets (same as FN_BEHAVIOR_SPLICE=0)")
    ap.add_argument("--param-order", choices=PARAM_ORDER_CHOICES, default=None,
                    help="parameter order of functions and delegates: offset (declaration order, the recorded "
                         "offsets; default) or table (the legacy order as listed; FN_PARAM_ORDER=table)")
    args = ap.parse_args(argv)
    global PARAM_ORDER
    if args.param_order:
        PARAM_ORDER = args.param_order
    if PARAM_ORDER not in PARAM_ORDER_CHOICES:
        raise SystemExit("FN_PARAM_ORDER must be one of %s, not %r" % (", ".join(PARAM_ORDER_CHOICES), PARAM_ORDER))

    if args.self_test:
        return self_test()
    if args.promote:
        return promote(args)

    with open(args.reflection, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    blocks = data.get("classes_in_groups_with_fort_names") or data.get("classes") or []
    names = {}
    if args.name_map:
        with open(args.name_map, "r", encoding="utf-8") as fh:
            names = {k.lower(): v for k, v in json.load(fh).items()}
    type_map = {}
    if args.type_map:
        with open(args.type_map, "r", encoding="utf-8") as fh:
            type_map = json.load(fh)
    parents = None
    if args.parent_map:
        with open(args.parent_map, "r", encoding="utf-8") as fh:
            parents = lower_keys(json.load(fh))
    resolver = Resolver(type_map)
    source_label = os.path.relpath(args.reflection, REPO)

    named = [b for b in blocks if b.get("block_va", "").lower() in names]
    if args.only:
        named = [b for b in named if names[b["block_va"].lower()] == args.only]
    if args.compare_header and args.compare_class:
        named = [b for b in named if names[b["block_va"].lower()] == args.compare_class]
    if not named:
        print("no named blocks match (name map entries: %d)" % len(names), file=sys.stderr)
        return 1

    report = []
    index = []
    os.makedirs(args.out, exist_ok=True)
    for block in named:
        name = names[block["block_va"].lower()]
        parent_entry = None
        if parents is not None:
            parent_entry = parents.get(block["block_va"].lower(), {})
        model = build_class(block, name, resolver, args.order, parent_entry)
        if args.compare_header and args.compare_class == name:
            compare_with_header(model, args.compare_header, report)
        if args.compare_header and args.compare_class:
            continue
        with open(os.path.join(args.out, name + ".h"), "w", encoding="utf-8") as fh:
            fh.write(render_header(model, args.api, source_label))
        with open(os.path.join(args.out, name + ".cpp"), "w", encoding="utf-8") as fh:
            fh.write(render_cpp(model, source_label))
        index.append({
            "name": name,
            "block_va": model["block_va"],
            "parent": model["parent"],
            "parent_fn": model["parent_fn"],
            "members": len(model["members_table_order"]),
            "replicated": sum(1 for m in model["members_table_order"] if m["replicated"]),
            "functions": len(model["functions"]),
            "todo_count": len(model["todos"]),
            "todos": model["todos"],
        })
    if index:
        with open(os.path.join(args.out, "index.json"), "w", encoding="utf-8") as fh:
            json.dump(index, fh, indent=1)
    if report:
        text = "\n".join(report) + "\n"
        if args.report:
            with open(args.report, "w", encoding="utf-8") as fh:
                fh.write(text)
        sys.stdout.write(text)
    if args.compare_header and args.compare_class:
        print("compared %d class(es) against %s" % (len(named), args.compare_header), file=sys.stderr)
    else:
        print("generated %d class(es) into %s" % (len(index), args.out), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
