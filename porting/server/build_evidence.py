#!/usr/bin/env python3
"""Build endpoint_evidence.json, reflection_schemas.json and the generated table in EVIDENCE.md.

Python standard library only. Inputs:
  --scan FILE          output of `evidence_scan.py --keys KEYS --out FILE` (default run: see EVIDENCE.md)
  --struct-blocks FILE porting/generated/reflection/struct_blocks.json (git-ignored; only for the
                       non-MCP response structs listed in REFLECTED_STRUCTS)

    python3 -I porting/server/evidence_scan.py --keys KEYS --out SCAN.json
    python3 -I porting/server/build_evidence.py --scan SCAN.json \
        --struct-blocks porting/generated/reflection/struct_blocks.json

Each served route is matched to the client's route format strings found in the runtime image. A
match gives the string's VA, the number of code references (`lea` in the code sections), the
functions that load it, the HTTP verb literals and JSON-key literals inside those functions, and any
config template (DefaultEngine.ini, class `asset`). The evidence class is set per row (see CLASSES).
"""

import argparse
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import schema_check  # noqa: E402

IMAGE_BASE = 0x140000000
GENERATED = os.path.normpath(os.path.join(HERE, "..", "generated"))
DEFAULT_ENGINE_INI = os.path.join(GENERATED, "DefaultEngine.ini")
OUT_JSON = os.path.join(HERE, "endpoint_evidence.json")
OUT_SCHEMAS = os.path.join(HERE, "reflection_schemas.json")
OUT_MD = os.path.join(HERE, "EVIDENCE.md")
MD_BEGIN = "<!-- BEGIN GENERATED TABLE (build_evidence.py) -->"
MD_END = "<!-- END GENERATED TABLE (build_evidence.py) -->"

# Ordered strongest first. A row's primary class is the first one it has.
CLASSES = ("wine_log", "reflection", "binary", "asset", "binary_string_only", "convention")

# Observed requests in server_surface.md, "Requests observed, in order" (the 12.60 Wine login run).
# Keyed by the backend route name. method is the verb seen in that run.
WINE_LOG = {
    "datarouter": ("POST", "step 1 (telemetry, 204)"),
    "oauth.token": ("POST", "steps 2 and 5 (client_credentials, exchange_code)"),
    "versioncheck": ("GET", "step 3 (200 NO_UPDATE)"),
    "cloudstorage.system.list": ("GET", "step 4 (200 [])"),
    "oauth.sessions.kill": ("DELETE", "step 6 (204)"),
    "account.by_id": ("GET", "step 7"),
    "account.external_auths": ("GET", "step 7"),
    "account.by_ids": ("GET", "step 8 (200 array)"),
    "account.user_setting": ("POST", "step 8"),
    "account.try_play_on_platform": ("POST", "step 9 (200 text true)"),
    "lightswitch.bulk": ("GET", "step 10 (200 array, status UP)"),
    "enabled_features": ("GET", "step 11"),
    "account.profile_token_verify": ("POST", "step 11 (204)"),
    "cloudstorage.user.list": ("GET", "step 12 (404 accepted)"),
    "profile.command": ("POST", "step 13 (QueryProfile, fullProfileUpdate; twice)"),
    "storefront.keychain": ("GET", "step 14 (must be an array of guid:base64key:path strings)"),
    "oauth.sessions.kill_token": ("DELETE", "step 15 (failed login)"),
}

# The later frontend refresh pass named in server_surface.md (verb not recorded there).
WINE_LOG_PATH_ONLY = {
    "calendar.timeline": "refresh pass after Frontend (path named, verb not recorded)",
    "storefront.catalog": "refresh pass after Frontend (path named, verb not recorded)",
    "content.page": "refresh pass after Frontend (path named, verb not recorded)",
    "friends.summary": "refresh pass after Frontend (friends/api/v1/* named, verb not recorded)",
    "friends.lists": "refresh pass after Frontend (friends/api/v1/* named, verb not recorded)",
    "friends.settings": "refresh pass after Frontend (friends/api/v1/* named, verb not recorded)",
    "presence.last_online": "refresh pass after Frontend (presence/api/v1/* named, verb not recorded)",
    "presence.subscriptions.list": "refresh pass after Frontend (presence/api/v1/* named, verb not recorded)",
    "presence.subscriptions.add": "refresh pass after Frontend (presence/api/v1/* named, verb not recorded)",
    "presence.subscriptions.remove": "refresh pass after Frontend (presence/api/v1/* named, verb not recorded)",
    "privacy.account": "refresh pass after Frontend (privacy/account/* named, verb not recorded)",
}

# Route formats from DefaultEngine.ini (the cooked config extract). class `asset`.
CONFIG_TEMPLATES = {
    "profile.command": "McpClientCommandUrl=/api/game/v2/profile/`accountId/client/`fname?profileId=`profileId",
    "storefront.catalog": "ListCatalogEntriesUrl=/api/storefront/v2/catalog",
}

# Response structs the backend answers from. MCP ones come from the tracked headers; the
# others are read from the reflection struct_blocks.json (see REFLECTED_STRUCTS).
REFLECTED_STRUCTS = {
    "GeneralChatReturn": "chat.reserve_general_rooms",
    "GeneralChatRoom": "chat.reserve_general_rooms",
    "CatalogDownload": "storefront.catalog",
    "EventsTimeline": "calendar.timeline (channels.states)",
    "CalendarDownload": "calendar.timeline",
}

# Element struct of an array member. struct_blocks gives only an address for the element type that does
# not resolve to a block name, so the element is matched by name here (inferred, not read from the binary).
ELEMENT_TYPES = {
    ("GeneralChatReturn", "FounderChatRooms"): "GeneralChatRoom",
    ("GeneralChatReturn", "GlobalChatRooms"): "GeneralChatRoom",
}

# Backend routes served that no binary string matches, plus notes for them.
NOTES = {
    "storefront.keychain": "Binary has the literal with no code reference (lea 0). Answer shape from the "
                           "login capture (array of strings), not from reflection.",
    "datarouter": "Binary has `datarouter/api/v1/public/data?SessionID=` with no code reference.",
    "content.page": "No matching string in the binary. Path named in the earlier refresh-pass capture.",
    "legacy.get": "Matches the generic template /api/v1/{namespace}/get with namespace Fortnite. Not a known route.",
    "matchmaking.session_key": "Route format is in the binary. The key field name is not confirmed.",
    "party.meta": "Route format is not in the binary (PATCH .../parties/{id}/meta has no matching literal).",
}


def expand_regex(pattern):
    """Backend router regex -> list of segment templates ('*' for captures). Alternations are expanded."""
    text = re.sub(r"^\(\?:/fortnite\)\?", "", pattern)
    text = re.sub(r"^\[/fortnite\]", "", text)
    text = text.replace("[^/]+", "*").replace("[A-Za-z]+", "*").replace("\\.", ".").replace("\\-", "-")
    text = text.split("?")[0]
    group = re.search(r"\(([^()]*\|[^()]*)\)", text)
    if group:
        options = []
        for alt in group.group(1).split("|"):
            options.extend(expand_regex(text[:group.start()] + alt + text[group.end():]))
        return options
    text = re.sub(r"\((?:[^()]|\([^()]*\))*\)", "*", text)
    return [text.split("/")]


def norm_regex(pattern):
    return expand_regex(pattern)[0]


def norm_binary(template):
    """Client route format -> segment template."""
    text = template.split("?")[0]
    text = re.sub(r"^(%s|\*)", "", text)  # a leading base placeholder is the service host, not a segment
    text = re.sub(r"`\w+", "*", text)
    text = re.sub(r"\{\w+\}", "*", text)
    text = re.sub(r"%[sdl]+", "*", text)
    if not text.startswith("/"):
        text = "/" + text
    text = text.rstrip("/")
    return text.split("/")


def matches(backend_segs, binary_segs):
    """True when the client format's segments equal a contiguous run of the backend route's segments.

    A wildcard pairs only with a wildcard. The one exception is the party namespace: the client's
    `{namespace}` is the literal "Fortnite" in our routes, so that pair is accepted.
    """
    want = [seg for seg in binary_segs if seg != ""]
    have = [seg for seg in backend_segs if seg != ""]
    if len(want) < 2 or want == ["*"]:
        return False
    for start in range(len(have) - len(want) + 1):
        window = have[start:start + len(want)]
        ok = True
        for got, need in zip(window, want):
            if got == need or (got == "*" and need == "*") or (got == "Fortnite" and need == "*"):
                continue
            ok = False
            break
        if ok:
            return True
    return False


def any_match(backend_pattern, binary_text):
    return any(matches(segs, norm_binary(binary_text)) for segs in expand_regex(backend_pattern))


def load_scan(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def binary_rows(scan):
    """Path-like rows of the scan, UTF-16 preferred when both encodings exist."""
    rows = {}
    for row in scan["rows"]:
        text = row["text"]
        if "/" not in text and not text.startswith("datarouter"):
            continue
        current = rows.get(text)
        if current is None or (row["encoding"] == "utf16" and current["encoding"] != "utf16"):
            rows[text] = row
    return rows


def keys_by_function(scan):
    """Function VA -> JSON key or verb literals loaded there (generic keys shared by > 6 functions are dropped)."""
    table = {}
    for row in scan["rows"]:
        if "/" in row["text"] or not row["lea_refs"] or len(row["functions"]) > 6:
            continue
        for func in row["functions"]:
            table.setdefault(func["va"], set()).add(row["text"])
    return table


def config_hits():
    if not os.path.exists(DEFAULT_ENGINE_INI):
        return {}
    with open(DEFAULT_ENGINE_INI, encoding="utf-8", errors="replace") as handle:
        lines = handle.read().splitlines()
    hits = {}
    for name, needle in CONFIG_TEMPLATES.items():
        key = needle.split("=", 1)[0]
        for number, line in enumerate(lines, 1):
            if line.startswith(needle.split("=", 1)[0] + "=") and line.strip() == needle:
                hits[name] = {"line": number, "text": needle}
    return hits


def mcp_schema():
    structs = schema_check.parse_structs()
    single = [{"json_name": m["json_name"], "type": schema_check.describe(m["type"]),
               "cpp": m["cpp_type"]} for m in structs["FProfileUpdateSingle"]["members"]]
    envelope = [{"json_name": name, "type": schema_check.describe(spec["type"]), "cpp": spec["cpp"]}
                for name, spec in schema_check.ENVELOPE_FIELDS.items()]
    return {"FProfileUpdateSingle": single, "FProfileUpdate": single + envelope}


def reflected_struct_schema(struct_blocks_path):
    """Member list for REFLECTED_STRUCTS, from the reflection struct_blocks.json (local, git-ignored)."""
    if not struct_blocks_path or not os.path.exists(struct_blocks_path):
        return {}
    with open(struct_blocks_path, encoding="utf-8") as handle:
        blocks = json.load(handle)["structs"]
    by_name = {block["name"]: block for block in blocks if block.get("name")}
    out = {}
    for name in REFLECTED_STRUCTS:
        block = by_name.get(name)
        if block is None:
            continue
        members = []
        seen = set()
        map_inner = {prop["name"] + suffix for prop in block.get("properties", []) if prop["kind"] == "Map"
                     for suffix in ("_Key", "_Value")}
        for prop in block.get("properties", []):
            # Bitfield bools have no offset but are still serialized, so they are kept. Map key and value
            # records (Name_Key, Name_Value) are reflection-only and are not JSON members.
            if prop["name"] in seen or prop["name"] in map_inner:
                continue
            seen.add(prop["name"])
            member = {"name": prop["name"], "json_name": schema_check.standardize_case(prop["name"]),
                      "kind": prop["kind"]}
            element = ELEMENT_TYPES.get((name, prop["name"]))
            if prop["kind"] == "Array" and element:
                member["element"] = element
            members.append(member)
        out[name] = {"package": block.get("package"), "size": block.get("size"),
                     "members": members, "used_for": REFLECTED_STRUCTS[name]}
    return out


def build_rows(backend, scan, keyfuncs, bins, config):
    rows = []
    for route in backend.route_table():
        name = None
        for live in backend.router.routes:
            if live.regex.pattern == route["pattern"] and live.method == route["method"]:
                name = live.name
                break
        found = []
        for text, row in bins.items():
            if any_match(route["pattern"], text):
                found.append((text, row))
        chosen = None
        if found:
            # Prefer the template with the most literal segments, then the one with more code references.
            found.sort(key=lambda item: (-sum(1 for seg in norm_binary(item[0]) if seg not in ("", "*")),
                                         -item[1]["lea_refs"], item[0]))
            chosen = found[0]
        classes = []
        row = {
            "id": name,
            "method": route["method"],
            "backend_pattern": route["pattern"],
            "backend_label_before": route["status"],
            "served": True,
            "route_format": None,
            "exe_va": None,
            "string_encoding": None,
            "code_refs": 0,
            "builder_functions": [],
            "verb_literals": [],
            "json_keys_in_builder": [],
            "config_template": None,
            "schema": None,
            "notes": NOTES.get(name, ""),
        }
        if chosen is not None:
            text, hit = chosen
            row["route_format"] = text
            row["exe_va"] = hex(IMAGE_BASE + hit["rva"])
            row["string_encoding"] = hit["encoding"]
            row["code_refs"] = hit["lea_refs"]
            row["builder_functions"] = [hex(f["va"]) for f in hit["functions"]]
            verbs, keys = set(), set()
            for func in hit["functions"]:
                for lit in keyfuncs.get(func["va"], ()):
                    if lit in ("GET", "POST", "PUT", "DELETE", "PATCH"):
                        verbs.add(lit)
                    else:
                        keys.add(lit)
            row["verb_literals"] = sorted(verbs)
            row["json_keys_in_builder"] = sorted(keys)[:40]
            if hit["lea_refs"]:
                classes.append("binary")
            else:
                classes.append("binary_string_only")
        if name in config:
            row["config_template"] = config[name]
            classes.append("asset")
        if name in WINE_LOG:
            verb, step = WINE_LOG[name]
            row["wine_log"] = {"method": verb, "where": step}
            if row["method"] != verb:
                row["method_disagrees_with_wine_log"] = True
            classes.append("wine_log")
        elif name in WINE_LOG_PATH_ONLY:
            row["wine_log"] = {"method": None, "where": WINE_LOG_PATH_ONLY[name]}
            classes.append("wine_log")
        if name == "profile.command":
            row["schema"] = {"response": "FProfileUpdate (inherits FProfileUpdateSingle)",
                             "source": "reflection", "members": mcp_schema()["FProfileUpdate"]}
            classes.append("reflection")
        if name == "matchmaking.session_key":
            row["schema"] = {"response": "accountId, sessionId, key (base64)", "source": "binary",
                             "reader": "0x142230050", "callback": "0x142227630"}
        if name == "chat.reserve_general_rooms":
            row["schema"] = {"response": "GeneralChatReturn", "source": "reflection"}
            classes.append("reflection")
        if name == "storefront.catalog" or name == "calendar.timeline":
            row["schema"] = {"response": "CatalogDownload / CalendarDownload (reflection, not checked yet)",
                             "source": "reflection"}
            classes.append("reflection")
        if not classes:
            classes.append("convention")
        ordered = [c for c in CLASSES if c in classes]
        row["evidence_classes"] = ordered
        row["primary_class"] = ordered[0]
        rows.append(row)
    return rows


def unserved_rows(bins, served_formats):
    rows = []
    for text, hit in sorted(bins.items()):
        if text in served_formats:
            continue
        verbs = []
        rows.append({
            "route_format": text,
            "exe_va": hex(IMAGE_BASE + hit["rva"]),
            "string_encoding": hit["encoding"],
            "code_refs": hit["lea_refs"],
            "builder_functions": [hex(f["va"]) for f in hit["functions"]],
            "primary_class": "binary" if hit["lea_refs"] else "binary_string_only",
            "served": False,
            "verb_literals": verbs,
        })
    return rows


def table_markdown(rows):
    lines = ["| Endpoint | Verb | Route format (client) | exe VA | refs | Response schema | Evidence | Served |",
             "| --- | --- | --- | --- | --- | --- | --- | --- |"]
    for row in rows:
        verb = row["method"]
        if row.get("wine_log") and row["wine_log"].get("method"):
            verb = row["wine_log"]["method"]
        fmt = row["route_format"] or ("(config) " + row["config_template"]["text"] if row["config_template"] else "not in binary")
        fmt = fmt.replace("|", "\\|")
        schema = row["schema"]["response"] if row.get("schema") else "-"
        classes = ", ".join(row["evidence_classes"])
        va = row["exe_va"] or "-"
        lines.append(f"| `{row['id']}` | {verb} | `{fmt}` | {va} | {row['code_refs']} | {schema} | {classes} | yes |")
    return "\n".join(lines)


def update_markdown(table):
    if not os.path.exists(OUT_MD):
        return False
    with open(OUT_MD, encoding="utf-8") as handle:
        text = handle.read()
    if MD_BEGIN not in text or MD_END not in text:
        return False
    head, rest = text.split(MD_BEGIN, 1)
    _, tail = rest.split(MD_END, 1)
    with open(OUT_MD, "w", encoding="utf-8") as handle:
        handle.write(head + MD_BEGIN + "\n" + table + "\n" + MD_END + tail)
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--scan", required=True)
    parser.add_argument("--struct-blocks", default=os.path.join(GENERATED, "reflection", "struct_blocks.json"))
    parser.add_argument("--stdout", action="store_true", help="print the JSON instead of writing files")
    args = parser.parse_args()

    sys.path.insert(0, HERE)
    from backend import Backend
    backend = Backend(state_step_sec=0)

    scan = load_scan(args.scan)
    bins = binary_rows(scan)
    keyfuncs = keys_by_function(scan)
    config = config_hits()
    rows = build_rows(backend, scan, keyfuncs, bins, config)
    served_formats = {row["route_format"] for row in rows if row["route_format"]}
    unserved = unserved_rows(bins, served_formats)
    payload = {
        "_about": "Generated by porting/server/build_evidence.py. Edit that script, not this file. "
                  "Classes: " + ", ".join(CLASSES) + " (binary_string_only: the literal is in the exe but no "
                  "code reference was found). Verbs come from the Wine login capture, or from verb literals "
                  "in the builder function (a hint, not proof).",
        "exe": scan["exe"],
        "image_base": scan["image_base"],
        "served": rows,
        "binary_unserved": unserved,
        "reflection_schemas": {"mcp": mcp_schema(),
                               "others": reflected_struct_schema(args.struct_blocks)},
    }
    if args.stdout:
        print(json.dumps(payload, indent=1))
        return 0
    with open(OUT_JSON, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=1, ensure_ascii=False)
        handle.write("\n")
    schemas = {"_about": "Reflection-derived struct members used by the backend's schema checks. "
                         "Generated by build_evidence.py from reflection struct_blocks.json.",
               "others": payload["reflection_schemas"]["others"]}
    with open(OUT_SCHEMAS, "w", encoding="utf-8") as handle:
        json.dump(schemas, handle, indent=1, ensure_ascii=False)
        handle.write("\n")
    updated = update_markdown(table_markdown(rows))
    print(f"served rows {len(rows)}, binary-only rows {len(unserved)}, EVIDENCE.md table "
          f"{'updated' if updated else 'not updated (markers missing)'}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
