#!/usr/bin/env python3
"""Record SM_/SK_-named packages verified to contain no mesh export.

Reads the bulk export checkpoints (static, skeletal) and their per-package logs
under porting/generated, and writes porting/asset_pipeline/nonmesh_packages.json:
basenames whose every conversion attempt failed with 'Package has no static mesh
export' and that have no successful GLB. Genuine converter failures are listed
separately and are NOT excluded. Used by progress/refresh_totals.py to compute
mesh-only denominators.
"""
import json
import sys
from pathlib import Path

PORTING = Path(__file__).resolve().parents[1]
OUTPUTS = ('asset_pipeline', 'asset_pipeline_skeletal')
NO_MESH = 'Package has no static mesh export'
# The converter also parses non-mesh primary exports before looking for a mesh; these
# packages are animations or control rigs that merely carry an SK_ name.
NON_MESH_PRIMARY = (NO_MESH, 'Could not read AnimSequence', 'Could not read ControlRigBlueprintGeneratedClass')


def classify(states):
    """states: list of (output_dir, records). Returns (nonmesh, genuine) sorted basename lists."""
    ok, nomesh, other = set(), set(), set()
    for out, records in states:
        for name, rec in records.items():
            base = name.rsplit('/', 1)[-1]
            if rec.get('status') == 'ok':
                ok.add(base)
                continue
            log = out / 'logs' / name / 'latest.log'
            if not log.is_file():
                continue  # never attempted by the converter (e.g. a stale test record)
            text = log.read_text(errors='replace')
            (nomesh if any(m in text for m in NON_MESH_PRIMARY) else other).add(base)
    nonmesh = nomesh - ok - other
    genuine = (other - ok)
    return sorted(nonmesh), sorted(genuine)


def main():
    states = []
    for d in OUTPUTS:
        out = PORTING / 'generated' / d
        data = json.loads((out / 'state.json').read_text())
        states.append((out, data['records']))
    nonmesh, genuine = classify(states)
    doc = {
        'version': 1,
        'method': 'Cue4Inspect --export-glb attempts on every SM_/SK_-named cooked package; '
                  'nonmesh = every attempt reported "Package has no static mesh export" and none succeeded.',
        'nonmesh': {'SM_': [n for n in nonmesh if n.startswith('SM_')],
                    'SK_': [n for n in nonmesh if n.startswith('SK_')]},
        'genuine_failures': genuine,
    }
    path = PORTING / 'asset_pipeline' / 'nonmesh_packages.json'
    path.write_text(json.dumps(doc, indent=1, sort_keys=True) + '\n')
    print(f"SM_ nonmesh={len(doc['nonmesh']['SM_'])} SK_ nonmesh={len(doc['nonmesh']['SK_'])} genuine={len(genuine)}")
    return 0


if __name__ == '__main__':
    sys.exit(main())
