"""Reflection-derived JSON schemas for the MCP structs, and a checker for backend responses.

Python standard library only. Run `python3 -I porting/server/schema_check.py` for a report.

Source of truth. The USTRUCT headers under porting/native/Source/McpProfileSys/Generated are
generated from the 12.60 reflection data (porting/generated/reflection/struct_blocks.json). Each
member is one UPROPERTY line, so the member list and the C++ type are read from there. The
generated tree is tracked by Git, so the tests do not depend on the git-ignored extraction.

JSON names. The engine writes each UPROPERTY under `FJsonObjectConverter::StandardizeCase`
(Engine/Source/Runtime/JsonUtilities/Private/JsonObjectConverter.cpp, UStructToJsonObject):
lower-case the first character, then replace "ID" with "Id". So `ProfileRevision` is written as
`profileRevision` and `bReceiveGifts` stays `bReceiveGifts`. The binary has the PascalCase
property names (ASCII) and the camelCase log text ("Missing new 'profileChangesBaseRevision' key").

Type mapping (JSON types the engine's converter produces):
  int8..int64, uint*        integer
  float, double             number
  bool                      boolean
  FString, FName, FText     string
  FDateTime                 string, ISO 8601 (datetime)
  E<Name> (UENUM)           string (the enumerator name)
  FJsonObjectWrapper        object (any JSON object)
  TArray<X>                 array of X
  F<Name> (USTRUCT)         object with that struct's members
  U<Name>* (UObject*)       not serialized here, skipped

Checks. A response is checked against a struct with `check_object`. Every member must be present
with a compatible JSON type. Keys the struct does not declare are reported as extra, and they fail
unless the caller allows them (`allow_extra`). The MCP envelope names that are not in any McpProfileSys
header (serverTime, responseVersion, multiUpdate) are listed in ENVELOPE_FIELDS with their evidence.
"""

import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
GENERATED_DIR = os.path.normpath(os.path.join(HERE, "..", "native", "Source", "McpProfileSys", "Generated"))
REFLECTION_JSON = os.path.normpath(os.path.join(HERE, "..", "generated", "reflection", "struct_blocks.json"))

INTEGER_TYPES = {"int8", "int16", "int32", "int64", "int", "uint8", "uint16", "uint32", "uint64", "uint"}
NUMBER_TYPES = {"float", "double"}
STRING_TYPES = {"FString", "FName", "FText"}
ISO_8601 = re.compile(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d{1,9})?(Z|[+-]\d{2}:\d{2})?")

# Envelope fields of an MCP command response that no McpProfileSys header declares.
# Each entry gives the JSON type and where the name is seen. The types are a convention
# (the engine writes these as ServerTime, ResponseVersion and MultiUpdate, and the binary has
# those PascalCase names), so they are the lowest-confidence part of the schema.
ENVELOPE_FIELDS = {
    "serverTime": {"type": "datetime", "evidence": "binary: ServerTime (ASCII property name); convention for type"},
    "responseVersion": {"type": "integer", "evidence": "binary: ResponseVersion (ASCII); convention for type"},
    "multiUpdate": {"type": ("array", "FProfileUpdateSingle"),
                    "evidence": "binary: MultiUpdate (ASCII); element type is FProfileUpdateSingle (see note)"},
}

_STRUCT_RE = re.compile(r"USTRUCT\(\)\s*struct\s+(\w+)\s*(?::\s*public\s+(\w+))?\s*\{(.*?)\n\};", re.DOTALL)
_MEMBER_RE = re.compile(r"UPROPERTY\([^)]*\)\s*\n\s*([^;]+?)\s+(\w+)\s*;")


def standardize_case(name):
    """FJsonObjectConverter::StandardizeCase."""
    if not name:
        return name
    fixed = name[0].lower() + name[1:]
    return fixed.replace("ID", "Id")


def _json_type(cpp_type, known):
    cpp_type = cpp_type.strip()
    array = re.fullmatch(r"TArray<\s*(.+?)\s*>", cpp_type)
    if array:
        return ("array", _json_type(array.group(1), known))
    if cpp_type in INTEGER_TYPES:
        return "integer"
    if cpp_type in NUMBER_TYPES:
        return "number"
    if cpp_type == "bool":
        return "boolean"
    if cpp_type in STRING_TYPES:
        return "string"
    if cpp_type == "FDateTime":
        return "datetime"
    if cpp_type == "FJsonObjectWrapper":
        return "object"
    if re.fullmatch(r"E[A-Z]\w*", cpp_type):
        return "enum"
    if re.fullmatch(r"F[A-Z]\w*", cpp_type):
        return ("struct", cpp_type)
    if cpp_type.endswith("*") or cpp_type.startswith("TWeakObjectPtr") or cpp_type.startswith("TSoftObjectPtr"):
        return "unserialized"
    return "unknown:" + cpp_type


def parse_structs(directory=GENERATED_DIR):
    """{struct name: {"parent": str|None, "members": [{"name", "json_name", "cpp_type", "type"}]}}."""
    raw = {}
    for fname in sorted(os.listdir(directory)):
        if not fname.endswith(".h"):
            continue
        with open(os.path.join(directory, fname), encoding="utf-8") as handle:
            text = handle.read()
        for match in _STRUCT_RE.finditer(text):
            name, parent, body = match.group(1), match.group(2), match.group(3)
            members = []
            for member in _MEMBER_RE.finditer(body):
                cpp_type, prop = member.group(1).strip(), member.group(2)
                members.append({"name": prop, "cpp_type": cpp_type})
            raw[name] = {"parent": parent, "members": members, "file": fname}
    structs = {}
    for name, info in raw.items():
        members = []
        for member in info["members"]:
            members.append({
                "name": member["name"],
                "json_name": standardize_case(member["name"]),
                "cpp_type": member["cpp_type"],
                "type": _json_type(member["cpp_type"], raw),
            })
        structs[name] = {"parent": info["parent"], "members": members, "file": info["file"]}
    return structs


def describe(json_type):
    if isinstance(json_type, tuple):
        if json_type[0] == "array":
            return "array of " + describe(json_type[1])
        if json_type[0] == "struct":
            return json_type[1]
    return json_type


def check_value(value, json_type, structs, path, issues, allow_extra):
    if json_type == "unserialized":
        return
    if isinstance(json_type, tuple) and json_type[0] == "array":
        if not isinstance(value, list):
            issues.append(f"{path}: expected array, got {type(value).__name__}")
            return
        for index, item in enumerate(value):
            check_value(item, json_type[1], structs, f"{path}[{index}]", issues, allow_extra)
        return
    if isinstance(json_type, tuple) and json_type[0] == "struct":
        check_object(value, json_type[1], structs, path, issues, allow_extra=allow_extra)
        return
    checks = {
        "integer": lambda v: isinstance(v, int) and not isinstance(v, bool),
        "number": lambda v: isinstance(v, (int, float)) and not isinstance(v, bool),
        "boolean": lambda v: isinstance(v, bool),
        "string": lambda v: isinstance(v, str),
        "enum": lambda v: isinstance(v, str),
        "datetime": lambda v: isinstance(v, str) and ISO_8601.fullmatch(v) is not None,
        "object": lambda v: isinstance(v, dict),
    }
    test = checks.get(json_type)
    if test is None:
        issues.append(f"{path}: no checker for JSON type {json_type}")
    elif not test(value):
        issues.append(f"{path}: expected {json_type}, got {_actual(value)}")


def _actual(value):
    if isinstance(value, bool):
        return "boolean"
    if isinstance(value, int):
        return "integer"
    if isinstance(value, float):
        return "number"
    if isinstance(value, str):
        return "string"
    if isinstance(value, list):
        return "array"
    if isinstance(value, dict):
        return "object"
    return type(value).__name__


def check_object(value, struct_name, structs, path, issues, allow_extra=()):
    """Check one JSON object against a struct. Returns the list of issues (empty when it conforms)."""
    if struct_name not in structs:
        issues.append(f"{path}: unknown struct {struct_name}")
        return issues
    if not isinstance(value, dict):
        issues.append(f"{path}: expected object for {struct_name}, got {_actual(value)}")
        return issues
    info = structs[struct_name]
    members = list(info["members"])
    if info["parent"]:
        parent = structs.get(info["parent"])
        if parent is not None:
            members = parent["members"] + members
    declared = {member["json_name"] for member in members}
    for member in members:
        key = member["json_name"]
        here = f"{path}.{key}"
        if key not in value:
            issues.append(f"{here}: missing (required by {struct_name}.{member['name']})")
            continue
        check_value(value[key], member["type"], structs, here, issues, allow_extra)
    for key in value:
        if key not in declared and key not in allow_extra:
            issues.append(f"{path}.{key}: not declared by {struct_name}")
    return issues


def check_profile_response(body, structs, *, envelope=True):
    """Check one MCP profile command or query response.

    The root carries the FProfileUpdateSingle members (the engine reads one profile update from the
    root) plus the envelope fields. Each entry of `multiUpdate` must be a full FProfileUpdateSingle.
    """
    issues = []
    allowed_root = tuple(ENVELOPE_FIELDS) if envelope else ()
    check_object(body, "FProfileUpdateSingle", structs, "$", issues, allow_extra=allowed_root)
    if envelope:
        for key, spec in ENVELOPE_FIELDS.items():
            if key not in body:
                issues.append(f"$.{key}: missing (envelope field, {spec['evidence']})")
                continue
            if key == "multiUpdate":
                check_value(body[key], ("array", ("struct", "FProfileUpdateSingle")), structs,
                            "$.multiUpdate", issues, allow_extra=())
            else:
                check_value(body[key], spec["type"], structs, "$." + key, issues, allow_extra=())
    return issues



# Reflected structs outside McpProfileSys. reflection_schemas.json (tracked, written by build_evidence.py)
# has the member names and reflected kinds: Bool, Int, Int64, UInt32, Double, Str, Name, Text, Array, Struct, Map.
REFLECTED_SCHEMAS_JSON = os.path.join(HERE, "reflection_schemas.json")
_KIND_TESTS = {
    "Bool": ("boolean", lambda v: isinstance(v, bool)),
    "Int": ("integer", lambda v: isinstance(v, int) and not isinstance(v, bool)),
    "Int64": ("integer", lambda v: isinstance(v, int) and not isinstance(v, bool)),
    "UInt32": ("integer", lambda v: isinstance(v, int) and not isinstance(v, bool)),
    "Double": ("number", lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "Float": ("number", lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "Str": ("string", lambda v: isinstance(v, str)),
    "Name": ("string", lambda v: isinstance(v, str)),
    "Text": ("string", lambda v: isinstance(v, str)),
    "Map": ("object", lambda v: isinstance(v, dict)),
}


def load_reflected_schemas(path=REFLECTED_SCHEMAS_JSON):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)["others"]


def check_reflected_object(value, struct_name, schemas, path="$", issues=None):
    """Check a JSON object against a reflection_schemas.json entry: every member present with its kind.

    Struct members (FDateTime and other structs) accept an object or an ISO 8601 string. An Array member
    with an `element` entry checks each item against that struct.
    """
    issues = [] if issues is None else issues
    if struct_name not in schemas:
        issues.append(f"{path}: no reflected schema for {struct_name}")
        return issues
    if not isinstance(value, dict):
        issues.append(f"{path}: expected object for {struct_name}, got {_actual(value)}")
        return issues
    declared = set()
    for member in schemas[struct_name]["members"]:
        key = member["json_name"]
        declared.add(key)
        here = f"{path}.{key}"
        if key not in value:
            issues.append(f"{here}: missing (required by {struct_name}.{member['name']})")
            continue
        got = value[key]
        kind = member["kind"]
        if kind == "Array":
            if not isinstance(got, list):
                issues.append(f"{here}: expected array, got {_actual(got)}")
            elif member.get("element"):
                for index, item in enumerate(got):
                    check_reflected_object(item, member["element"], schemas, f"{here}[{index}]", issues)
        elif kind == "Struct":
            if not (isinstance(got, dict) or (isinstance(got, str) and ISO_8601.fullmatch(got))):
                issues.append(f"{here}: expected object or ISO 8601 string, got {_actual(got)}")
        elif kind in _KIND_TESTS:
            label, test = _KIND_TESTS[kind]
            if not test(got):
                issues.append(f"{here}: expected {label}, got {_actual(got)}")
    for key in value:
        if key not in declared:
            issues.append(f"{path}.{key}: not declared by {struct_name}")
    return issues


def _report():
    structs = parse_structs(GENERATED_DIR)
    print(f"{len(structs)} structs from {GENERATED_DIR}")
    for name in ("FProfileUpdateSingle", "FMcpProfileChangeRequest", "FMcpLootResult", "FGiftBoxInfo"):
        info = structs.get(name)
        if info is None:
            print(f"{name}: not declared")
            continue
        print(f"{name} ({info['file']}):")
        for member in info["members"]:
            print(f"  {member['json_name']:32s} {describe(member['type'])}  [{member['cpp_type']}]")
    if os.path.exists(REFLECTION_JSON):
        with open(REFLECTION_JSON, encoding="utf-8") as handle:
            blocks = json.load(handle)["structs"]
        by_name = {block["name"]: block for block in blocks if block.get("name")}
        ref = by_name.get("ProfileUpdateSingle")
        if ref is not None:
            # Array elements appear as extra records with the same name; compare name sets.
            names = {prop["name"] for prop in ref["properties"]}
            mine = {member["name"] for member in structs["FProfileUpdateSingle"]["members"]}
            print("struct_blocks.json ProfileUpdateSingle matches header:", names == mine)
    return 0


if __name__ == "__main__":
    sys.exit(_report())
