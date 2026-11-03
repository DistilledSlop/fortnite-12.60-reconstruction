#!/usr/bin/env python3
"""Inventory the Apollo_Terrain streamed levels from the full loose extraction.

The authoritative list is the set of sublevel package references (soft object
paths) in the Apollo_Terrain.umap name table: every `/Game/..Name` that also
appears as `/Game/..Name.Name`, minus the parent world itself.  That set equals
the 87 `LevelStreamingAlwaysLoaded` exports of the parent.

For each level the Cue4Inspect modes --actor-report, --landscape-report and
--imports-json are run (read-only, loose mode) and cached as raw JSON.  Outputs
go only under porting/generated/streamed_levels/ (plus an optional --report).
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

PORTING = Path(__file__).resolve().parents[1]
OUT_DIR = PORTING / 'generated' / 'streamed_levels'
DEFAULT_TOOL = Path('/mnt/vibrato/fortnite-asset-pipeline/tool-skeletal/Cue4Inspect.dll')
DEFAULT_EXTRACTION = Path('/mnt/vibrato/fortnite-extracted-full')
PARENT = '/Game/Athena/Apollo/Maps/Apollo_Terrain'
MODES = ('actor-report', 'landscape-report', 'imports-json')
MAX_WORKERS = 8
NON_ACTOR_TYPES = {
    'Level', 'Model', 'Package', 'MetaData', 'ObjectRedirector', 'BodySetup', 'World', 'NavigationSystemModuleConfig',
    'LandscapeMaterialInstanceConstant', 'MaterialInstanceConstant', 'LandscapeLayerInfoObject', 'PhysicalMaterial',
    'RuntimeVirtualTexture', 'LevelStreamingAlwaysLoaded', 'LevelStreamingDynamic', 'BlueprintGeneratedClass',
}


# ---------------------------------------------------------------- pure helpers

def parse_tool_json(text: str):
    """Tool stdout may start with an 'Indexed N ...' line; parse from the first '{'."""
    start = text.find('{')
    if start < 0:
        raise ValueError('no JSON object in tool output')
    return json.loads(text[start:])


def soft_level_refs(data: bytes, parent: str = PARENT) -> list[str]:
    """Package paths P where both P and P.<LastSegment> appear as strings (soft refs)."""
    strings = {m.decode('ascii') for m in re.findall(rb'/Game/[\x20-\x7e]{3,300}', data)}
    levels = sorted(s for s in strings if '.' not in s and f"{s}.{s.rsplit('/', 1)[1]}" in strings)
    return [s for s in levels if s != parent]


def game_path_to_file(extraction: Path, package: str, ext: str) -> Path:
    assert package.startswith('/Game/')
    return extraction / 'FortniteGame' / 'Content' / (package[len('/Game/'):] + ext)


def is_actor_type(type_name: str) -> bool:
    """Heuristic: export class names are all that actor-report gives, so exclude components,
    textures, materials, meshes and other known non-actor exports."""
    if type_name in NON_ACTOR_TYPES:
        return False
    return not re.search(r'(Component|Texture\w*|Material\w*|StaticMesh|SkeletalMesh|Class|Function|Property)$', type_name)


def summarize_actor_report(report: dict) -> dict:
    hist = {t['type']: t['count'] for t in report.get('exportTypes', [])}
    return {
        'export_count': sum(hist.values()),
        'actor_count': sum(c for t, c in hist.items() if is_actor_type(t)),
        'class_histogram': dict(sorted(hist.items(), key=lambda kv: (-kv[1], kv[0]))),
        'static_mesh_components': len(report.get('staticMeshComponents', [])),
    }


def summarize_landscape_report(report: dict) -> dict:
    actors = report.get('actors', [])
    return {
        'landscape_actors': len(actors),
        'landscape_components': sum(len([c for c in a.get('components', []) if c]) for a in actors),
    }


def summarize_imports(report: dict) -> dict:
    """Distinct /Script classes (as /Script/Module.Class) and /Game package roots."""
    script, structs, game = set(), set(), set()
    for item in report.get('imports', []):
        chain = item.get('chain') or []
        root = chain[0]['name'] if chain else ''
        kind = item.get('kind')
        if root.startswith('/Script/'):
            if kind == 'Class':
                script.add(f"{root}.{item['name']}")
            elif kind == 'ScriptStruct':
                structs.add(f"{root}.{item['name']}")
        elif item.get('kind') == 'Package' and item.get('name', '').startswith('/Game/'):
            game.add(item['name'])
    return {'script_classes': sorted(script), 'script_structs': sorted(structs), 'game_packages': sorted(game)}


def aggregate_gap(levels: list[dict], native: set[str], engine: set[str]) -> list[dict]:
    """Rank /Script classes imported by the levels. native/engine are sets of declared C++ names."""
    needed: dict[str, list[str]] = {}
    for level in levels:
        for cls in level.get('imports', {}).get('script_classes', []):
            needed.setdefault(cls, []).append(level['package'])
    rows = []
    for cls, pkgs in needed.items():
        name = cls.rsplit('.', 1)[1]
        in_native = any(p + name in native for p in 'AUFI')
        in_engine = any(p + name in engine for p in 'AUFI')
        rows.append({
            'class': cls, 'levels': len(pkgs), 'in_native': in_native, 'in_engine_425': in_engine,
            'gap': not in_native and not in_engine, 'example_levels': sorted(pkgs)[:3],
        })
    rows.sort(key=lambda r: (-r['levels'], r['class']))
    return rows


DECL = re.compile(r'^\s*(?:class|struct)\s+(?:[A-Z][A-Z0-9_]*_API\s+)?([AUFI][A-Z]\w*)\b(?!\s*;)', re.M)


def scan_declarations(roots: list[Path]) -> set[str]:
    names: set[str] = set()
    for root in roots:
        if not root.is_dir():
            continue
        for header in root.rglob('*.h'):
            try:
                names.update(DECL.findall(header.read_text(errors='replace')))
            except OSError:
                pass
    return names


# ------------------------------------------------------------------ tool runs

def run_mode(tool: Path, extraction: Path, package: str, mode: str, raw_dir: Path, force: bool, timeout: int):
    out = raw_dir / f"{package.rsplit('/', 1)[1]}.{mode}.json"
    if out.is_file() and not force:
        return out, None
    cmd = ['dotnet', str(tool), package, str(extraction), '--loose', f'--{mode}']
    try:
        run = subprocess.run(cmd, capture_output=True, text=True, errors='replace', timeout=timeout)
    except subprocess.TimeoutExpired:
        return out, f'{mode}: timeout after {timeout}s'
    if run.returncode != 0:
        return out, f'{mode}: exit {run.returncode}: {run.stderr.strip()[-300:] or run.stdout.strip()[-300:]}'
    try:
        data = parse_tool_json(run.stdout)
    except ValueError as exc:
        return out, f'{mode}: {exc}'
    out.write_text(json.dumps(data, separators=(',', ':')))
    return out, None


def inventory_level(package: str, args, raw_dir: Path) -> dict:
    rec = {'package': package, 'files': {}, 'failures': []}
    for ext in ('.umap', '.uexp', '.ubulk'):
        p = game_path_to_file(args.extraction, package, ext)
        rec['files'][ext] = p.stat().st_size if p.is_file() else None
    rec['exists'] = rec['files']['.umap'] is not None
    rec['size'] = sum(v for v in rec['files'].values() if v)
    if not rec['exists']:
        rec['failures'].append('missing .umap in extraction')
        return rec
    results = {}
    for mode in MODES:
        out, err = run_mode(args.tool, args.extraction, package, mode, raw_dir, args.force, args.timeout)
        if err:
            rec['failures'].append(err)
            continue
        try:
            results[mode] = json.loads(out.read_text())
        except (OSError, ValueError) as exc:
            rec['failures'].append(f'{mode}: bad cache {exc}')
    if 'actor-report' in results:
        rec.update(summarize_actor_report(results['actor-report']))
    if 'landscape-report' in results:
        rec.update(summarize_landscape_report(results['landscape-report']))
    if 'imports-json' in results:
        rec['imports'] = summarize_imports(results['imports-json'])
    rec['parsed'] = not rec['failures']
    return rec


# ---------------------------------------------------------------------- report

def render_report(summary: dict, gap: list[dict], failures: list[dict]) -> str:
    L = ['# Apollo streamed levels inventory', '',
         'Generated by `porting/asset_pipeline/streamed_levels_inventory.py` from the full loose extraction. '
         'Counts come from `porting/generated/streamed_levels/` (ignored).', '',
         '## Counts', '']
    for k, v in summary.items():
        L.append(f'- {k}: {v}')
    L += ['', '## Source of the list', '',
          f'`{PARENT}.umap` references each sublevel as a soft object path (a `/Game/..P` plus `/Game/..P.Name` string pair in '
          'its name table). Excluding the parent itself this yields the 87 sublevels, matching its 87 `LevelStreamingAlwaysLoaded` '
          'exports (`--actor-report`). They are 66 `Landscape/Apollo_Terrain_LS_*` tiles plus 21 auxiliary maps '
          '(HLOD, audio, kill volumes, FX, POI foundations, background, nav, ...).', '',
          '## /Script class gap (top 40)', '',
          'Classes imported by at least one level and not declared under `porting/native` (Source and Plugins headers), '
          'ranked by number of levels. "UE 4.25" says whether the stock 4.25 engine tree declares it; "no" there is a true gap. Declaration scan is a regex over `class`/`struct` headers (A/U/F/I prefix tried), so '
          'a declared class is not proof it is in a built module or in `native_class_set.txt`.', '',
          '| # | Class | Levels | UE 4.25 |', '|---|---|---|---|']
    missing = [g for g in gap if not g['in_native']]
    for i, r in enumerate(missing[:40], 1):
        L.append(f"| {i} | `{r['class']}` | {r['levels']} | {'yes' if r['in_engine_425'] else 'NO (gap)'} |")
    L += ['', f'{len(missing)} of {len(gap)} imported /Script classes are not declared under `porting/native`; '
          f'{sum(g["gap"] for g in gap)} of those are in no tree.']
    L += ['', '## Failures', '']
    if failures:
        for f in failures[:30]:
            L.append(f"- `{f['package']}`: {'; '.join(f['failures'])[:200]}")
    else:
        L.append('None.')
    L += ['', '## Rerun', '', '```sh',
          'python3 -I porting/asset_pipeline/streamed_levels_inventory.py --report porting/streamed_levels_inventory.md',
          'python3 -I porting/asset_pipeline/test_streamed_levels_inventory.py', '```', '',
          'Raw tool JSON is cached in `porting/generated/streamed_levels/raw/`; add `--force` to re-run the inspector. '
          'Each inspector call indexes ~455k packages (~15 s, ~1 GB); at most 8 run in parallel (`--workers`). '
          'Also writes `levels.json`, `class_gap.json`, `summary.json` there. The extraction is never written.', '']
    return '\n'.join(L)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--tool', type=Path, default=DEFAULT_TOOL)
    ap.add_argument('--extraction', type=Path, default=DEFAULT_EXTRACTION)
    ap.add_argument('--workers', type=int, default=MAX_WORKERS)
    ap.add_argument('--timeout', type=int, default=300)
    ap.add_argument('--force', action='store_true')
    ap.add_argument('--limit', type=int, help='only the first N levels (testing)')
    ap.add_argument('--report', type=Path, help='write the markdown report here (e.g. porting/streamed_levels_inventory.md)')
    args = ap.parse_args()
    args.workers = max(1, min(args.workers, MAX_WORKERS))
    ext = args.extraction.resolve()
    out_dir = OUT_DIR.resolve()
    if out_dir == ext or ext in out_dir.parents:
        ap.error('output directory must not be inside the extraction')
    raw_dir = OUT_DIR / 'raw'
    raw_dir.mkdir(parents=True, exist_ok=True)

    parent_file = game_path_to_file(args.extraction, PARENT, '.umap')
    levels = soft_level_refs(parent_file.read_bytes())
    if args.limit:
        levels = levels[:args.limit]
    print(f'{len(levels)} streamed levels from {parent_file.name}', file=sys.stderr)
    with ThreadPoolExecutor(args.workers) as pool:
        records = list(pool.map(lambda p: inventory_level(p, args, raw_dir), levels))

    native = scan_declarations([PORTING / 'native' / 'Source', PORTING / 'native' / 'Plugins'])
    engine = scan_declarations([Path('/opt/fortnite/UnrealEngine/Engine/Source'), Path('/opt/fortnite/UnrealEngine/Engine/Plugins')])
    gap = aggregate_gap(records, native, engine)
    failures = [r for r in records if r['failures']]
    summary = {
        'streamed_levels': len(records),
        'umap_present': sum(r['exists'] for r in records),
        'uexp_present': sum(r['files']['.uexp'] is not None for r in records),
        'ubulk_present': sum(r['files']['.ubulk'] is not None for r in records),
        'fully_parsed': sum(bool(r.get('parsed')) for r in records),
        'with_failures': len(failures),
        'total_bytes': sum(r['size'] for r in records),
        'total_export_count': sum(r.get('export_count', 0) for r in records),
        'total_actor_count_heuristic': sum(r.get('actor_count', 0) for r in records),
        'total_landscape_components': sum(r.get('landscape_components', 0) for r in records),
        'levels_with_landscape': sum(r.get('landscape_components', 0) > 0 for r in records),
        'distinct_script_classes': len(gap),
        'script_classes_in_native': sum(g['in_native'] for g in gap),
        'script_classes_in_ue425_only': sum(g['in_engine_425'] and not g['in_native'] for g in gap),
        'script_class_gaps': sum(g['gap'] for g in gap),
        'distinct_game_packages': len({p for r in records for p in r.get('imports', {}).get('game_packages', [])}),
    }
    (OUT_DIR / 'levels.json').write_text(json.dumps(records, indent=1))
    (OUT_DIR / 'class_gap.json').write_text(json.dumps(gap, indent=1))
    (OUT_DIR / 'summary.json').write_text(json.dumps(summary, indent=1))
    if args.report:
        args.report.write_text(render_report(summary, gap, failures))
    print(json.dumps(summary, indent=1))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
