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

# EPropertyFlags (UObjectMacros/ObjectMacros.h, UE 4.25)
CPF_NET = 0x20
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

    def __init__(self, type_map, known_fns=None):
        self.types = lower_keys(type_map)
        # Construct functions of every named class, including those outside the native set.
        self.known = {str(k).lower() for k in (known_fns or [])}
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
            target = self.name(refs.get("interface_class")) or self.todo("Interface", refs.get("interface_class"))
            return "TScriptInterface<%s>" % target
        if kind == "Array":
            elem = children.get("elem")
            inner = self.ctype(elem, {}) if elem else self.todo_named("ArrayElem", entry.get("name"))
            return "TArray<%s>" % inner
        if kind == "Set":
            elem = children.get("elem")
            inner = self.ctype(elem, {}) if elem else self.todo_named("SetElem", entry.get("name"))
            return "TSet<%s>" % inner
        if kind == "Map":
            key, value = children.get("key"), children.get("value")
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
        elif kind == "Enum" and nxt and nxt.get("enum_underlying_of") == p.get("name"):
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
        bitfield = entry.get("kind") == "Bool" and bool(entry.get("bitfield"))
        dim = int(entry.get("array_dim") or 1)
        name = entry.get("name") or "UnnamedProperty%d" % index
        if promote:
            # Promote mode emits only properties whose every type resolves to a declared native class.
            if new_tokens:
                resolver.skipped.append(skip_record("property", entry, name, category_of(new_tokens),
                                                    "unresolved type: " + ", ".join(sorted(set(new_tokens))), new_tokens))
                continue
            if wire and not struct_mode and hx(entry.get("property_flags")) & CPF_NET:
                # A replicated property whose struct serializes with a custom NetSerialize (body not recovered):
                # the default serializer would write different bits, so the property is not emitted.
                resolver.skipped.append(skip_record("property", entry, name, "struct_custom_netserialize",
                                                    "replicated struct with a custom NetSerialize (body not recovered)",
                                                    []))
                continue
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
            "rep_notify": entry.get("rep_notify"),
            "specifiers": property_specifiers(entry),
            "used": used,
            "offset": entry.get("offset"),
        })
    return members, notes


def decode_params(params, resolver):
    """Return (return_type, [arg decls]) for a decoded parameter table."""
    ret = "void"
    args = []
    pairs, _ = split_properties(params)
    for entry, children in pairs:
        pf = hx(entry.get("property_flags"))
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
        args.append("%s %s" % (ctype, entry.get("name") or "Param%d" % len(args)))
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
    # Resolver: without a gate a known type name is used as is; an unknown one is a TODO token.
    res = Resolver({"0x9": "ENetRole"})
    check("type_ref without gate", res.type_ref("enum", "0x9", "Enum") == "ENetRole")
    check("type_ref unknown is a token", res.type_ref("struct", "0x77", "Struct").startswith("TODO_Struct_"))
    check("engine type index on a missing root is empty", EngineTypeIndex("/nonexistent/engine").lookup("struct", "FVector") is None)


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
    if failures:
        print("SELF-TEST FAILED:", *failures, sep="\n  ")
        return 1
    print("self-test passed (%d checks)" % count[0])
    return 0


# ---------------------------------------------------------------- promotion into native/Source (first pass)
# --promote builds the closure listed in porting/native_class_set.txt. It emits only what resolves:
# a property whose type needs a struct, enum, delegate, interface or a class outside the set is skipped
# and recorded. Functions are emitted only when they are RPCs (wire-relevant) or RepNotify targets.

PROMOTE_PACKAGES = ("/Script/FortniteGame", "/Script/PlayspaceFramework")
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


def rpc_arg(decl):
    """UHT: a replicated function's non-scalar parameter must be passed by const reference."""
    ctype, name = decl.rsplit(" ", 1)
    if ctype.endswith(("&", "*")) or ctype.startswith(SCALAR_ARG_PREFIXES):
        return decl
    return "const %s& %s" % (ctype, name)


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


def build_functions_promoted(functions, resolver, notify_names, owner_fns):
    """RPCs and RepNotify targets only. A function whose flags cannot be classified is recorded, since it might be an RPC."""
    emitted = []
    for fn in functions:
        name = fn.get("name") or "UnnamedFunction"
        flags, params, problem = function_view(fn, owner_fns)
        if flags is None:
            resolver.skipped.append(skip_function(fn, "flags_ambiguous", "RPC status unknown: " + problem))
            continue
        is_rpc = bool(flags & FUNC_NET_ANY)
        if not is_rpc and name not in notify_names:
            continue
        if is_rpc and not flags & (FUNC_NET_SERVER | FUNC_NET_CLIENT | FUNC_NET_MULTICAST):
            resolver.skipped.append(skip_function(fn, "net_flag_without_role",
                                                  "net flag without a Server/Client/NetMulticast role"))
            continue
        if is_rpc and flags & (FUNC_CONST | FUNC_STATIC):
            resolver.skipped.append(skip_function(fn, "static_or_const_rpc", "static or const RPC is not emitted"))
            continue
        if params is None:
            resolver.skipped.append(skip_function(fn, "params_not_decoded", problem))
            continue
        mark = len(resolver.tokens)
        resolver.wire_used = False
        resolver.used = {}
        ret, args = decode_params(params, resolver)
        used = resolver.used
        new_tokens = resolver.tokens[mark:]
        if new_tokens:
            resolver.skipped.append(skip_function(fn, category_of(new_tokens),
                                                  "unresolved parameter type: " + ", ".join(sorted(set(new_tokens))),
                                                  new_tokens))
            continue
        if is_rpc and resolver.wire_used:
            resolver.skipped.append(skip_function(fn, "struct_custom_netserialize",
                                                  "RPC parameter struct with a custom NetSerialize (body not recovered)"))
            continue
        if ret != "void":
            resolver.skipped.append(skip_function(fn, "non_void_return", "return type %s is not emitted" % ret))
            continue
        if is_rpc:
            args = [rpc_arg(arg) for arg in args]
        emitted.append({"name": name, "rpc": is_rpc,
                        "specifiers": function_specifiers(flags) if is_rpc else [], "args": args, "used": used})
    return emitted


def build_class_promoted(block, name, resolver, parent_entry, owner_fns):
    resolver.todos = []
    resolver.class_refs = set()
    resolver.skipped = []
    members, _notes = build_members(block.get("properties") or [], resolver, promote=True)
    notify_names = {m["rep_notify"] for m in members if m["rep_notify"]}
    functions = build_functions_promoted(block.get("functions") or [], resolver, notify_names, owner_fns)
    emitted_onrep = {f["name"] for f in functions if not f["rpc"]}
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
        "forward_decls": sorted(ref for ref in resolver.class_refs if ref not in (name, parent)),
        "used": merge_used([m["used"] for m in members] + [f["used"] for f in functions]),
        "skipped": list(resolver.skipped),
        "functions_in_table": len(block.get("functions") or []),
        "rpc_in_reflection": sum(1 for f in (block.get("functions") or [])
                                 if function_view(f, owner_fns)[0] is not None
                                 and function_view(f, owner_fns)[0] & FUNC_NET_ANY),
    }


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
    lines.extend("class %s;" % ref for ref in model["forward_decls"])
    if model["forward_decls"]:
        lines.append("")
    lines.append("#include \"%s.generated.h\"" % name)
    lines.append("")
    lines.append(model["uclass"])
    lines.append("class %s %s : public %s" % (api, name, model["parent"]))
    lines.append("{")
    lines.append("\tGENERATED_BODY()")
    lines.append("")
    lines.append("public:")
    for m in model["members_decl_order"]:
        lines.append("\t// table index %s, flags %s" % (m["table_index"], m["property_flags"]))
        lines.append("\t%s" % uproperty_line(m["specifiers"]))
        lines.append("\t%s" % m["decl"])
        lines.append("")
    for f in model["functions"]:
        lines.append("\tUFUNCTION(%s)" % ", ".join(f["specifiers"]))
        lines.append("\tvoid %s(%s);" % (f["name"], ", ".join(f["args"])))
        if f["rpc"]:
            lines.append("\tvoid %s_Implementation(%s);" % (f["name"], ", ".join(f["args"])))
        lines.append("")
    if rep:
        lines.append("\tvirtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;")
    lines.append("};")
    lines.append("")
    return "\n".join(lines) + "\n"


def render_promoted_cpp(model, source_label):
    name = model["name"]
    rep = [m for m in model["members_table_order"] if m["replicated"]]
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s. Do not edit by hand." % source_label,
        "#include \"%s.h\"" % name,
    ]
    if rep:
        lines.append("#include \"Net/UnrealNetwork.h\"")
    lines.append("")
    for f in model["functions"]:
        impl = "%s_Implementation" % f["name"] if f["rpc"] else f["name"]
        lines.append("void %s::%s(%s)" % (name, impl, ", ".join(f["args"])))
        lines.append("{")
        lines.append("\t// Body not recovered: reflection-derived stub.")
        lines.append("}")
        lines.append("")
    if rep:
        lines.append("void %s::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const" % name)
        lines.append("{")
        lines.append("\tSuper::GetLifetimeReplicatedProps(OutLifetimeProps);")
        lines.append("")
        for m in rep:
            lines.append("\tDOREPLIFETIME(%s, %s);" % (name, m["name"]))
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


def promote(args):
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

    # Closure check: every class is in a promote package, and each parent is listed or an engine class.
    errors = []
    for name in class_list:
        block = name_to_block.get(name)
        entry = parents.get(block or "") or {}
        if block is None or not entry:
            errors.append("%s: no named block with a parent record" % name)
        elif entry.get("package") not in PROMOTE_PACKAGES:
            errors.append("%s: package %s is not a promote package" % (name, entry.get("package")))
        else:
            parent = entry.get("parent_name")
            if parent not in listed:
                parent_entry = parents.get(str(entry.get("parent_va") or "").lower()) or {}
                if parent_entry.get("package") not in ENGINE_PACKAGES:
                    errors.append("%s: parent %s (package %s) is neither listed nor an engine class"
                                  % (name, parent, parent_entry.get("package")))
    if errors:
        raise SystemExit("closure check failed:\n  " + "\n  ".join(errors))

    # Type map: only the native set and engine classes. Any other named class becomes a TODO token.
    type_map = {}
    known = set()
    for block, name in names.items():
        fns = evidence_fns.get(block, [])
        known.update(fns)
        package = (parents.get(block) or {}).get("package")
        if name in listed or package in ENGINE_PACKAGES:
            for fn in fns:
                type_map[fn] = name
    # Class references in the tables point at jmp thunks to a class's NoRegister function (resolve_class_refs.py).
    class_refs = load_json(args.class_ref_map).get("resolved", {})
    for ref, block in class_refs.items():
        name = names.get(block.lower())
        if not name:
            continue
        known.add(ref.lower())
        if name in listed or (parents.get(block.lower()) or {}).get("package") in ENGINE_PACKAGES:
            type_map[ref.lower()] = name
    # Struct, enum and delegate types (porting/extract_types.py): names for the type map, and the gate.
    tables = TypeTables(args.types_dir or os.path.dirname(args.reflection))
    for fn, info in tables.by_fn.items():
        type_map.setdefault(fn, info.cpp_name)
    resolver = Resolver(type_map, known_fns=known)
    gate = TypeGate(tables, EngineTypeIndex(args.engine_root), resolver)
    resolver.gate = gate
    engine_index = build_engine_header_index(args.engine_classes)
    source_label = os.path.relpath(args.reflection, REPO)

    index = []
    all_skipped = []
    class_models = []
    for name in class_list:
        block = name_to_block[name]
        owner_fns = set(evidence_fns.get(block, []))
        gate.current_package = parents[block].get("package")
        model = build_class_promoted(blocks_by_va[block], name, resolver, parents[block], owner_fns)
        model["includes"] = sorted({gate.include_of(f) for f in model["used"]} - {None})
        class_models.append(model)
        parent = model["parent"]
        if parent in listed:
            parent_include = "%s.h" % parent
        elif parent in engine_index:
            parent_include = engine_index[parent]
        else:
            raise SystemExit("%s: no engine header found for parent %s" % (name, parent))
        module = model["package"].rsplit("/", 1)[-1]
        api = "%s_API" % module.upper()
        header = render_promoted_header(model, api, parent_include, source_label)
        cpp = render_promoted_cpp(model, source_label)
        if "TODO" in header or "TODO" in cpp:
            raise SystemExit("%s: unresolved TODO token reached the output" % name)
        out_dir = os.path.join(args.source_root, module, "Generated")
        os.makedirs(out_dir, exist_ok=True)
        with open(os.path.join(out_dir, name + ".h"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(header)
        with open(os.path.join(out_dir, name + ".cpp"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(cpp)

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
            "rep_notify_functions_emitted": sum(1 for f in model["functions"] if not f["rpc"]),
            "functions_skipped": sum(1 for r in model["skipped"] if r["item"] == "function"),
            "forward_declarations": len(model["forward_decls"]),
        })

    # Closure: every promoted struct, enum and delegate used by an emitted member or RPC, transitively.
    seeds = {}
    for model in class_models:
        for fn, kinds in model["used"].items():
            seeds.setdefault(fn, set()).update(kinds)
    models, texts, all_used = emit_types(gate, seeds, source_label)
    type_files = []
    for fn, model in sorted(models.items(), key=lambda kv: (kv[1]["kind"], kv[1]["cpp_name"])):
        info = gate.info(fn)
        text = texts[info.header_name]
        if "TODO" in text:
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
    engine_used = []
    for fn, kinds in sorted(all_used.items()):
        info = gate.info(fn)
        if info is not None and info.package not in PROMOTE_PACKAGES:
            engine_used.append({"type": info.cpp_name, "kind": info.kind, "package": info.package,
                                "include": gate.include_of(fn)})
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
    print("promoted %d classes into %s" % (len(index), args.source_root), file=sys.stderr)
    return 0


# ---------------------------------------------------------------- struct, enum and delegate types (promote)
# The closure of types used by the promoted classes. Inputs: porting/extract_types.py output (struct_blocks.json,
# enum_blocks.json, delegate_blocks.json, construct_fn_to_block.json). Engine types (not in PROMOTE_PACKAGES) are
# referenced by name and included from the engine tree (EngineTypeIndex). Promoted types are emitted into the
# module's Generated directory, one header per type.

WIRE_CUSTOM_TRAITS = ("HasNetSerializer", "HasNetDeltaSerializer", "HasNetSharedSerialization")
DELEGATE_MACRO = {"single": "DECLARE_DYNAMIC_DELEGATE", "multicast": "DECLARE_DYNAMIC_MULTICAST_DELEGATE",
                  "sparse": "DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE"}
PARAM_ORDINALS = ("", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine")
TYPE_DECL_RE = {
    "struct": re.compile(r"^\s*struct\s+(?:[A-Z][A-Z0-9_]*_API\s+)?(F[A-Za-z0-9_]+)\b[^;]*$", re.M),
    "enum": re.compile(r"^\s*enum\s+(?:class\s+|struct\s+)?(E[A-Za-z0-9_]+)\b[^;]*$", re.M),
    "delegate": re.compile(r"\bDECLARE_(?:DYNAMIC_)?(?:MULTICAST_)?(?:SPARSE_)?DELEGATE\w*\(\s*(F[A-Za-z0-9_]+)"),
}
ENGINE_ROOT_DEFAULT = os.environ.get("FN_ENGINE_ROOT", "/opt/fortnite/UnrealEngine/Engine")
ENGINE_SCAN_DIRS = ("Source/Runtime", "Plugins/Runtime", "Plugins/Developer", "Source/Developer")
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


class EngineTypeIndex:
    """First declaration of each struct (F...), enum (E...) and delegate type in the engine tree, Runtime first."""

    def __init__(self, root):
        self.index = {"struct": {}, "enum": {}, "delegate": {}}
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
                    inc = include_path(path)
                    if inc is None:
                        continue
                    with open(path, "r", encoding="utf-8", errors="replace") as fh:
                        text = fh.read()
                    self.files += 1
                    for kind, rx in TYPE_DECL_RE.items():
                        for match in rx.finditer(text):
                            self.index[kind].setdefault(match.group(1), inc)

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
        resolver.used.setdefault(info.fn, set()).add(entry_kind or "")
        if info.package in PROMOTE_PACKAGES:
            if info.package != self.current_package:
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
        if info.kind == "struct":
            model, reason = self.build_struct(info)
        elif info.kind == "enum":
            model, reason = self.build_enum(info)
        else:
            model, reason = self.build_delegate(info)
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
                     "align": info.rec.get("align"), "struct_ops": info.rec.get("struct_ops")}
            return model, None
        finally:
            self.current_package = saved

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
            for entry, children in pairs:
                if "error" in entry:
                    reason = "unreadable parameter record"
                    break
                pf = hx(entry.get("property_flags"))
                if pf & (CPF_RETURN_PARM | CPF_OUT_PARM | CPF_REFERENCE_PARM):
                    reason = "return, out or reference parameter"
                    break
                args.append((child.ctype(entry, children), entry.get("name") or "Param%d" % len(args)))
            if reason is None and len(args) > 9:
                reason = "more than nine parameters"
            if reason is None and child.tokens:
                reason = "parameter type unresolved (%s)" % ", ".join(sorted(set(child.tokens)))
            if reason is not None:
                return None, reason
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


def render_struct_header(model, source_label):
    name = model["cpp_name"]
    lines = [
        "// Generated by porting/gen_native_classes.py --promote from %s." % source_label,
        "// Do not edit by hand: run porting/promote_native_classes.sh. Members are in declaration order (offsets",
        "// ascending). A struct with a custom NetSerialize is emitted here only when it is not replicated.",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
    ]
    if model.get("parent_include"):
        lines.append("#include \"%s\"" % model["parent_include"])
    for inc in model.get("includes", []):
        if inc != model.get("parent_include"):
            lines.append("#include \"%s\"" % inc)
    lines.append("")
    lines.extend("class %s;" % ref for ref in model["forward_decls"])
    if model["forward_decls"]:
        lines.append("")
    lines.append("#include \"%s.generated.h\"" % name)
    lines.append("")
    lines.append("USTRUCT()")
    lines.append("struct %s%s" % (name, " : public %s" % model["parent"] if model["parent"] else ""))
    lines.append("{")
    lines.append("\tGENERATED_BODY()")
    lines.append("")
    for m in model["members"]:
        lines.append("\t// table index %s, offset %s, flags %s" % (m["table_index"], m["offset"], m["property_flags"]))
        lines.append("\tUPROPERTY()")
        lines.append("\t%s" % m["decl"])
        lines.append("")
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
    lines.append("")
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
        model["includes"] = sorted({gate.include_of(f) for f in model["used"]} - {None})
        if info.kind == "struct" and model["parent"] is not None:
            parent_fn = info.rec.get("super_fn")
            model["parent_include"] = gate.include_of(parent_fn) if parent_fn else None
        models[info.fn] = model
        queue.extend(model["used"].items())
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
    ap.add_argument("--engine-root", default=ENGINE_ROOT_DEFAULT,
                    help="--promote: UnrealEngine/Engine directory searched for struct, enum and delegate headers")
    args = ap.parse_args(argv)

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
