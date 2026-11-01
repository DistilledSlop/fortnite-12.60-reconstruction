#!/usr/bin/env python3
"""Prepare reviewable UE glTF import settings from verified exporter checkpoints.

No UE invocation or content changes. One selected package per import group retains
its original /Game directory; the matched-path importer patch remains required.
"""
import argparse
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import export_meshes as exporter


def mesh_count(path):
    """Read the already validated GLB JSON chunk for importer path eligibility."""
    with path.open('rb') as stream:
        header = stream.read(20)
        json_size = struct.unpack_from('<I', header, 12)[0]
        doc = json.loads(stream.read(json_size))
    meshes = doc.get('meshes')
    return len(meshes) if isinstance(meshes, list) else 0


def build_manifest(names, state, root, output, tool_id):
    if state.get('version') != 1 or state.get('input_root') != str(root):
        raise ValueError('checkpoint version or input root mismatch')
    records = state.get('records')
    if not isinstance(records, dict):
        raise ValueError('invalid checkpoint records')
    packages = []
    groups = []
    for name in sorted(set(exporter.package_path(n) for n in names)):
        record = records.get(name, {})
        if not exporter.verified(record, root, output, tool_id, name):
            raise ValueError(f'unverified checkpoint or changed input/output: {name}')
        files = record['files']
        expected = Path(name).name + '.glb'
        if len(files) != 1 or files[0]['name'] != expected:
            raise ValueError(f'{name}: expected exactly {expected} for matched-path import')
        source = (output / 'glb' / name / expected).resolve()
        if mesh_count(source) != 1:
            raise ValueError(f'{name}: matched-path import requires exactly one glTF mesh')
        game = '/Game/' + name.removeprefix('FortniteGame/Content/')
        destination = game.rsplit('/', 1)[0]
        packages.append({'source_package': name, 'game_package': game, 'glb': str(source),
                         'sha256': files[0]['sha256'], 'log': record.get('log'),
                         'status': 'glb_verified_not_imported'})
        groups.append({'GroupName': f'FortniteMesh{len(groups):04d}', 'Filenames': [str(source)],
                       'DestinationPath': destination, 'FactoryName': 'GLTFImportFactory',
                       'bReplaceExisting': False})
    if not packages:
        raise ValueError('select at least one package')
    return {'schema': 1, 'packages': packages,
            'limitations': ['GLB is geometry only; source materials are not reconstructed',
                            'Highest LOD only; original collision is not preserved by the documented importer path',
                            'Requires verified GLTF importer patch and -FortniteMatchAssetPath; no UE import or cook performed'],
            'import_settings': {'ImportGroups': groups}}


def selection_names(data):
    if not isinstance(data, dict) or not isinstance(data.get('sources'), list) or not data['sources']:
        raise ValueError('selection must contain a nonempty sources list')
    names = []
    for item in data['sources']:
        if not isinstance(item, dict) or not isinstance(item.get('packages'), list) or not item['packages']:
            raise ValueError('selection source must contain packages')
        names.extend(exporter.package_path(name) for name in item['packages'])
    return names


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--package', action='append', default=[])
    p.add_argument('--selection', type=Path, help='JSON file with sources[].packages lists')
    p.add_argument('--input-root', type=Path, default=exporter.DEFAULT_INPUT)
    p.add_argument('--tool-dir', type=Path, default=exporter.DEFAULT_TOOL)
    p.add_argument('--output', type=Path, default=exporter.DEFAULT_OUTPUT)
    p.add_argument('--settings-only', action='store_true', help='emit bare UE ImportAssets settings JSON')
    args = p.parse_args()
    root, output, tool = args.input_root.resolve(), args.output.resolve(), args.tool_dir.resolve()
    if (exporter.PORTING / 'generated').resolve() not in output.parents:
        p.error('output must be inside porting/generated')
    try:
        state = json.loads((output / 'state.json').read_text())
        names = args.package + (selection_names(json.loads(args.selection.read_text())) if args.selection else [])
        result = build_manifest(names, state, root, output, exporter.tool_fingerprint(tool))
    except (OSError, ValueError, KeyError, TypeError) as e:
        print(f'error: {e}', file=sys.stderr)
        return 1
    print(json.dumps(result['import_settings'] if args.settings_only else result, indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
