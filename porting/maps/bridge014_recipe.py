#!/usr/bin/env python3
"""Recover Bridge_014's static components from original Cue4Inspect reports.

Transforms are in the streamed level's coordinate system; a consumer must apply
its level streaming transform before treating them as persistent-world transforms.
No foliage instances or HLOD proxy are emitted as ordinary static placements.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path

LEVEL = 'FortniteGame/Content/Athena/Apollo/Maps/POI/Apollo_POI_Bridge_014'
SUPPORTS = {
    'SM_Apollo_BridgeSupport_Bottom': 'BridgeSupport_Bottom',
    'SM_Apollo_BridgeSupport_01_Top': 'BridgeSupport_01_Top',
    'SM_Apollo_BridgeSupport_02_Pillar': 'BridgeSupport_02_Pillar',
    'SM_Apollo_BridgeSupport_02_PillarTop': 'BridgeSupport_02_PillarTop',
}


def read_report(path):
    raw = path.read_text()
    start = raw.find('{')
    if start < 0:
        raise ValueError(f'no JSON in {path}')
    return json.loads(raw[start:])


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def mesh_imports(report):
    result = set()
    for item in report['imports']:
        if item['kind'] == 'StaticMesh':
            chain = item['chain']
            require(len(chain) == 2 and chain[0]['kind'] == 'Package', 'unexpected mesh import chain')
            result.add((item['name'], chain[0]['name']))
    return result


def vec(row, key):
    value = row[key]
    require(isinstance(value, list) and len(value) == 3 and all(isinstance(x, (float, int)) and not isinstance(x, bool) and math.isfinite(x) for x in value), f'invalid {key}')
    return value


def build(raw, proof, extracted):
    actor_path = raw / 'Apollo_POI_Bridge_014.actor-report.json'
    imports_path = raw / 'Apollo_POI_Bridge_014.imports-json.json'
    actor, imports = read_report(actor_path), read_report(imports_path)
    require(actor['path'] == LEVEL and imports['package'] == LEVEL, 'wrong map reports')
    rows = actor['staticMeshComponents']
    require(len(rows) == 23 and len({r['path'] for r in rows}) == 23, 'unexpected component inventory')
    map_imports = mesh_imports(imports)
    material_imports = {(i['name'], i['chain'][0]['name']) for i in imports['imports']
                        if i['kind'].startswith('Material') and len(i['chain']) == 2
                        and i['chain'][0]['kind'] == 'Package'}
    bp_packages = {i['name'].rsplit('/', 1)[-1]: i['name'] for i in imports['imports'] if i['kind'] == 'Package' and '/Blueprints/BridgeSupport_' in i['name']}
    require(set(bp_packages) == set(SUPPORTS.values()), 'unexpected support Blueprint imports')
    source_files = [actor_path, imports_path, extracted / (LEVEL + '.umap'), extracted / (LEVEL + '.uexp')]
    resolved = {}
    for mesh, name in SUPPORTS.items():
        bp = bp_packages[name]
        ar, ir = proof / (name + '.actor-report.json'), proof / (name + '.imports-json.txt')
        report, bp_imports = read_report(ar), read_report(ir)
        package = 'FortniteGame/Content/' + bp.removeprefix('/Game/')
        require(report['path'] == package and bp_imports['package'] == package, f'wrong Blueprint report: {name}')
        components = report['staticMeshComponents']
        require(len(components) == 1, f'unexpected Blueprint components: {name}')
        c = components[0]
        require(c['name'] == 'StaticMeshComponent0' and c['owner'] == 'Default__' + name + '_C' and c['mesh'] == mesh, f'wrong CDO component: {name}')
        require(c['location'] == [0, 0, 0] and c['rotation'] == [0, 0, 0] and c['scale'] == [1, 1, 1], f'nonidentity CDO transform: {name}')
        require((mesh, c['meshPackage']) in mesh_imports(bp_imports), f'unproven CDO mesh: {name}')
        require(c['overrideMaterials'] == [], f'unexpected CDO overrides: {name}')
        resolved[mesh] = (c['meshPackage'], bp, c['path'])
        source_files.extend([ar, ir, extracted / (package + '.uasset'), extracted / (package + '.uexp')])
    placements, pending, excluded = [], [], []
    for c in rows:
        owner, mesh = c['owner'], c['mesh']
        if owner == 'InstancedFoliageActor_0':
            pending.append({'component': c['path'], 'reason': 'foliage instance transforms are not in actor report', 'mesh': mesh, 'meshPackage': c['meshPackage']})
            continue
        if owner == 'LODActor_3':
            excluded.append({'component': c['path'], 'reason': 'HLOD proxy; source geometry emitted instead', 'mesh': mesh, 'meshPackage': c['meshPackage']})
            continue
        bp = None
        if mesh in resolved:
            require(c['name'] == 'StaticMeshComponent0' and c['meshPackage'].startswith('/Script/'), 'unexpected support component reference')
            package, bp, cdo = resolved[mesh]
        else:
            package = c['meshPackage']
            require((mesh, package) in map_imports, f'unproven map mesh {mesh}')
            cdo = None
        mats = c['overrideMaterials']
        for material in mats:
            require((material['name'] == 'None' and material['package'] is None) or (material['name'], material['package']) in material_imports, 'unresolved material override')
        placements.append({'sourceComponent': c['path'], 'sourceOwner': owner, 'mesh': mesh, 'meshPackage': package,
                           'blueprintPackage': bp, 'blueprintCDOComponent': cdo, 'overrideMaterials': mats,
                           'levelTransform': {'translation': vec(c, 'location'), 'rotationPitchYawRoll': vec(c, 'rotation'), 'scale': vec(c, 'scale')}})
    require(len(placements) == 21 and len(pending) == 1 and len(excluded) == 1, 'unexpected coverage')
    return {'level': LEVEL, 'transformSpace': 'streamed-level coordinates; apply streaming transform for persistent world',
            'sourceProofSha256': {str(p): digest(p) for p in source_files},
            'coverage': {'reportedComponents': len(rows), 'staticPlacements': len(placements), 'pendingFoliageComponents': len(pending), 'excludedHlodComponents': len(excluded)},
            'placements': placements, 'pending': pending, 'excluded': excluded,
            'limitations': ['Actor gameplay, collisions, foliage instances and streaming-level transform are not restored by this recipe.', 'Null material override slots are preserved as reported; consumers must retain Unreal override semantics.']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--raw', type=Path, default=Path('porting/generated/streamed_levels/raw'))
    parser.add_argument('--proof', type=Path, default=Path('porting/generated/streamed_levels/bridge014-proof'))
    parser.add_argument('--extracted', type=Path, default=Path('/mnt/vibrato/fortnite-extracted-full'))
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = build(args.raw, args.proof, args.extracted)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result['coverage']))


if __name__ == '__main__':
    main()
