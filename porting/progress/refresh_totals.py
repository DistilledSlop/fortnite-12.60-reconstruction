#!/usr/bin/env python3
"""Regenerate the tracked denominators file porting/progress/totals.json (stdlib, offline, deterministic).

Each entry is {value, source, method, generated_utc}. The values are counted from ignored local
extraction outputs under porting/generated/ (and one tracked report), so they change only when
that data changes. progress.py reads totals.json as the denominator of its leaves.

generated_utc is the UTC modification time of the source file the value was read from, not the
wall clock, so rerunning on unchanged data writes byte-identical output.

Prefix classes of cooked packages (assets.json) are NAMING-CONVENTION HEURISTICS: a package counts
as a static mesh when its file basename starts with SM_, and so on. They are transparent rules,
not asset-class truth; the asset registry (asset-registry-summary.txt) is the class-based source
and is used wherever it has the class.

Usage:
  python3 -I porting/progress/refresh_totals.py           write porting/progress/totals.json
  python3 -I porting/progress/refresh_totals.py --print   print the result without writing
  python3 -I porting/progress/refresh_totals.py --count-tree
      walk the full raw extraction ONCE and write porting/generated/cooked-name-counts-full.json
      (ignored). Only this mode walks files; the normal refresh reads that small count file.
"""

import argparse
import datetime
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
GEN = os.path.join(REPO, "porting", "generated")
OUT = os.path.join(HERE, "totals.json")
FULL_TREE = "/mnt/vibrato/fortnite-extracted-full"
FULL_SUMMARY = os.path.join(GEN, "extract-full-summary.json")
COUNTS_FULL = os.path.join(GEN, "cooked-name-counts-full.json")

# Named-class regexes for the gameplay leaves. The same strings are the `name_regex`
# values in components.json, so a leaf and its denominator always use one pattern.
GAMEPLAY_NAME_PATTERNS = ["Weapon", "Storm|SafeZone", "BattleBus|Aircraft|Parachute|Glider"]
FORTNITE_GAME_PACKAGE = "/Script/FortniteGame"

# Naming-convention heuristics over the basename of each cooked .uasset (distinct names).
PREFIX_RULES = {
    "static_mesh": r"^SM_",
    "skeletal_mesh": r"^SK_",
    "material": r"^(M|MI|MF)_",
    "texture": r"^T_",
    "animation": r"^A_",
    "niagara": r"^NS_",
}

# Path-based heuristic for sound class and mix packages (their folders hold most of them).
SOUND_CLASS_PATH = r"SoundClass|SoundMix"


class Missing(Exception):
    pass


def rel(path):
    return os.path.relpath(path, REPO).replace(os.sep, "/")


def utc_of(path):
    if not os.path.isfile(path):
        raise Missing(f"source missing: {rel(path)} (run the extraction that writes it)")
    stamp = datetime.datetime.fromtimestamp(os.path.getmtime(path), datetime.timezone.utc)
    return stamp.strftime("%Y-%m-%dT%H:%M:%SZ")


def load_json(path):
    if not os.path.isfile(path):
        raise Missing(f"source missing: {rel(path)}")
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def entry(value, path, method):
    return {"value": value, "source": rel(path), "method": method, "generated_utc": utc_of(path)}


def count_tree(root=FULL_TREE):
    """Walk the full raw extraction once and write the small count file. Distinct names, not files."""
    summary = load_json(FULL_SUMMARY)
    if os.path.realpath(root) != os.path.realpath(summary["outputRoot"]):
        raise Missing(f"--count-tree root {root} is not the summary outputRoot {summary['outputRoot']}")
    uasset_paths = []
    for dirpath, _dirs, files in os.walk(root):
        for name in files:
            if name.endswith(".uasset"):
                full = os.path.join(dirpath, name)
                uasset_paths.append(os.path.relpath(full, root).replace(os.sep, "/"))
    uasset_paths.sort()
    stems = {os.path.basename(f)[:-len(".uasset")] for f in uasset_paths}
    prefix = {k: sum(1 for st in stems if re.search(pat, st)) for k, pat in PREFIX_RULES.items()}
    sound = sum(1 for f in uasset_paths if re.search(SOUND_CLASS_PATH, f))
    data = {
        "root": summary["outputRoot"],
        "method": "os.walk over the full raw extraction; .uasset paths relative to root; "
                  "distinct_basenames = distinct basenames without .uasset; prefix = distinct basenames "
                  "matching PREFIX_RULES (re.search); sound_class_mix_paths = paths matching SOUND_CLASS_PATH",
        "uasset_paths": len(uasset_paths),
        "distinct_basenames": len(stems),
        "prefix": prefix,
        "sound_class_mix_paths": sound,
    }
    with open(COUNTS_FULL + ".tmp", "w", encoding="utf-8") as fh:
        fh.write(json.dumps(data, indent=1, sort_keys=True) + "\n")
    os.replace(COUNTS_FULL + ".tmp", COUNTS_FULL)
    return data


def build():
    totals = {}

    # ---- UHT class blocks (reflection tables)
    cpm_path = os.path.join(GEN, "reflection", "class_parent_map.json")
    cnm_path = os.path.join(GEN, "reflection", "class_name_map.json")
    cpm = load_json(cpm_path)
    cnm = load_json(cnm_path)
    named = {va: name for va, name in cnm.items() if isinstance(name, str) and name}
    totals["uht_class_blocks"] = entry(
        len(cpm), cpm_path,
        "UHT class blocks: one entry per block address in reflection/class_parent_map.json")
    totals["uht_classes_named"] = entry(
        len(named), cnm_path,
        "UHT class blocks with a recovered name: non-empty values in reflection/class_name_map.json")

    by_pkg = {}
    for va in named:
        pkg = (cpm.get(va) or {}).get("package") or "(none)"
        by_pkg[pkg] = by_pkg.get(pkg, 0) + 1
    totals["uht_classes_named_by_package"] = entry(
        dict(sorted(by_pkg.items())), cpm_path,
        "named UHT classes per package: the named class_name_map.json blocks grouped by the 'package' "
        "field of class_parent_map.json at the same block address")

    fng = [named[va] for va in named if (cpm.get(va) or {}).get("package") == FORTNITE_GAME_PACKAGE]
    matches = {pat: sum(1 for n in fng if re.search(pat, n)) for pat in GAMEPLAY_NAME_PATTERNS}
    totals["uht_fortnitegame_name_matches"] = entry(
        matches, cnm_path,
        f"named /Script/FortniteGame classes whose name matches each regex (re.search on the class name): "
        f"keys are the regexes used by the gameplay leaves in components.json")

    # ---- structs, enums, delegates
    for key, fname, field in (
            ("uht_structs", "struct_blocks.json", "structs"),
            ("uht_enums", "enum_blocks.json", "enums"),
            ("uht_delegates", "delegate_blocks.json", "delegates")):
        path = os.path.join(GEN, "reflection", fname)
        data = load_json(path)
        totals[key] = entry(len(data[field]), path,
                            f"entries in the '{field}' list of reflection/{fname} (construct-function join)")

    # ---- native functions with an execution address
    nf_path = os.path.join(GEN, "reflection", "native_funcs.json")
    nf = load_json(nf_path)
    funcs = 0
    with_exec = 0
    for fns in nf.values():
        for meta in fns.values():
            funcs += 1
            if meta.get("exec_addr"):
                with_exec += 1
    totals["native_functions_total"] = entry(
        funcs, nf_path, "functions in reflection/native_funcs.json, over all classes")
    totals["native_functions_with_exec"] = entry(
        with_exec, nf_path,
        "functions in reflection/native_funcs.json with a non-null exec_addr (ported bodies are "
        "measured against this denominator)")

    # ---- asset registry (class-based counts, cue4inspect --registry)
    reg_path = os.path.join(GEN, "asset-registry-summary.txt")
    if not os.path.isfile(reg_path):
        raise Missing(f"source missing: {rel(reg_path)}")
    counts = {}
    with open(reg_path, encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            m = re.match(r"^([A-Za-z][A-Za-z0-9_]*): (\d+)$", line)
            if m:
                counts[m.group(1)] = int(m.group(2))
            m = re.match(r"^Registry assets: (\d+)$", line)
            if m:
                counts["Registry assets (all classes)"] = int(m.group(1))
    totals["asset_registry_counts"] = entry(
        dict(sorted(counts.items())), reg_path,
        "per-class counts printed by the asset registry dump: 'ClassName: N' lines, keyed by class name "
        "(Blueprint, WidgetBlueprint, World = maps, SoundWave, AnimBlueprint, ...)")

    # ---- cooked packages (full raw extraction, all 31 paks, verified)
    full_path = FULL_SUMMARY
    full = load_json(full_path)
    if not (isinstance(full.get("verification"), dict) and full["verification"].get("ok") is True):
        raise Missing("extract-full-summary.json has no passing verification (verification.ok is not true)")
    full_uasset = full["byExtension"][".uasset"]["files"]
    totals["cooked_uasset_entries"] = entry(
        full_uasset, full_path,
        "distinct '.uasset' paths in the verified full raw extraction of all 31 paks "
        "(extract-full-summary.json byExtension['.uasset'].files; one path per package, duplicates across "
        "paks counted once)")
    totals["pak_full_distinct_paths"] = entry(
        full["uniquePaths"], full_path,
        "distinct pak paths over all 31 paks (extract-full-summary.json uniquePaths, duplicates across paks "
        "counted once); the denominator of the raw extraction leaf assets.pak_extraction")

    # Name-prefix and sound-path counts come from one walk of the full tree (--count-tree), stored in a
    # small ignored file. The walk is not repeated here, so the normal refresh reads only that file.
    counts_path = COUNTS_FULL
    counts = load_json(counts_path)
    if counts.get("uasset_paths") != full_uasset:
        raise Missing(f"{rel(counts_path)} does not match extract-full-summary.json (.uasset files "
                      f"{counts.get('uasset_paths')} vs {full_uasset}); rerun refresh_totals.py --count-tree")
    if counts.get("root") != full.get("outputRoot"):
        raise Missing(f"{rel(counts_path)} was counted from {counts.get('root')}, not the summary outputRoot")
    totals["cooked_uasset_distinct_names"] = entry(
        counts["distinct_basenames"], counts_path,
        "distinct .uasset basenames over all 31 paks (full tree walk, --count-tree)")
    totals["pak_files_total"] = entry(
        full["pakCount"], full_path,
        "pak files in the full extraction (extract-full-summary.json pakCount; 31)")
    prefix = {key: counts["prefix"][key] for key in PREFIX_RULES}
    rules_text = "; ".join(f"{k} = basename matches {v}" for k, v in PREFIX_RULES.items())
    totals["cooked_non_mesh_distinct_names"] = entry(
        counts["distinct_basenames"] - prefix["static_mesh"] - prefix["skeletal_mesh"],
        counts_path,
        "distinct cooked .uasset basenames excluding case-sensitive SM_ and SK_ prefixes; "
        "derived from the full-tree distinct basename and disjoint prefix counts")
    totals["sound_class_mix_packages"] = entry(
        counts["sound_class_mix_paths"], counts_path,
        "NAMING-CONVENTION HEURISTIC: distinct cooked .uasset paths whose full path (folder or name) "
        f"matches /{SOUND_CLASS_PATH}/. Sound class and mix packages are mostly in SoundClasses or "
        "SoundMixes folders, so a basename rule would miss them. Full tree walk (--count-tree).")
    totals["cooked_name_prefix_counts"] = entry(
        prefix, counts_path,
        "NAMING-CONVENTION HEURISTIC, not asset class. Distinct .uasset basenames matching re.search "
        f"per class over all 31 paks (full tree walk, --count-tree): {rules_text}. Case-sensitive. "
        "Animation and material counts include only these prefixes.")

    # ---- UI modules named for the CommonUI/UMG leaf (both packages, one denominator)
    ui_named = sum(v for k, v in by_pkg.items() if k in ("/Script/CommonUI", "/Script/UMG"))
    totals["uht_classes_named_commonui_umg"] = entry(
        ui_named, cpm_path,
        "named UHT classes in /Script/CommonUI plus /Script/UMG (the CommonUI and UMG leaf)")

    # ---- streamed levels (tracked report)
    mc_path = os.path.join(REPO, "porting", "map_compatibility.md")
    if not os.path.isfile(mc_path):
        raise Missing(f"source missing: {rel(mc_path)}")
    with open(mc_path, encoding="utf-8") as fh:
        m = re.search(r"(\d+) absent streaming levels", fh.read())
    if not m:
        raise Missing("pattern '(\\d+) absent streaming levels' not found in porting/map_compatibility.md")
    totals["apollo_streaming_levels"] = entry(
        int(m.group(1)), mc_path,
        "regex '(\\d+) absent streaming levels' in the tracked map_compatibility.md report")
    return totals


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--print", action="store_true", help="print the totals instead of writing totals.json")
    ap.add_argument("--count-tree", action="store_true",
                    help="walk the full extraction once and write porting/generated/cooked-name-counts-full.json")
    args = ap.parse_args(argv)
    if args.count_tree:
        try:
            data = count_tree()
        except Missing as exc:
            print(f"refresh_totals: {exc}", file=sys.stderr)
            return 2
        print(f"refresh_totals: wrote {rel(COUNTS_FULL)}: {json.dumps(data['prefix'], sort_keys=True)}, "
              f"distinct_basenames={data['distinct_basenames']}, sound_class_mix_paths={data['sound_class_mix_paths']}")
        return 0
    try:
        totals = build()
    except Missing as exc:
        print(f"refresh_totals: {exc}; totals.json left unchanged", file=sys.stderr)
        return 2
    text = json.dumps(totals, indent=1, sort_keys=True) + "\n"
    if args.print:
        sys.stdout.write(text)
        return 0
    tmp = OUT + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        fh.write(text)
    os.replace(tmp, OUT)
    print(f"refresh_totals: wrote {rel(OUT)} ({len(totals)} entries)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
