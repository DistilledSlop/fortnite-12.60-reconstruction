#!/usr/bin/env python3
"""Inspect source-validated frontend texture expressions without exporting bundles.

Texture imports are candidates only; they do not prove an expression-to-asset
binding. This diagnostic does not translate shaders or establish runtime support.
"""
import argparse
import json
import sys
from pathlib import Path

import export_frontend_numeric_batch as batch


def parse_resource(row, archive):
    framing_path = row['report_path'].with_suffix('.inline-resource.json')
    framing = json.loads(framing_path.read_bytes())
    if batch.digest(Path(row['entry']['export_map']).read_bytes()) != framing['export_map_sha256']:
        raise ValueError('export map provenance changed')
    if batch.package_path(framing['package']) != row['owner_path'].split('.')[0] or framing['export_name'] != row['owner']['name']:
        raise ValueError('framing owner identity mismatch')
    original = batch.EXTRACTION / (row['entry']['package'] + '.uexp')
    payload = batch.bundle.validate_source(original.read_bytes(), framing)
    parsed = batch.bundle.decoder.parse_prefix(payload, framing['names'])
    batch.bundle.decoder.associate_library(parsed, archive)
    return parsed


def describe(owner, owners, archive, cache):
    chain = batch.owner_chain(owner, owners)
    resource_owner = next(row for row in reversed(chain)
                          if row['owner']['type'] == 'UMaterial' or row['owner']['staticPermutation'] is True)
    resource_path = resource_owner['owner_path']
    if resource_path not in cache:
        cache[resource_path] = parse_resource(resource_owner, archive)
    parsed = cache[resource_path]
    if parsed['material_path'] != chain[0]['owner_path']:
        raise ValueError('shader base identity differs from actual inheritance root')
    arrays = parsed['uniform_expression_arrays']
    expressions = {}
    for name, array in arrays.items():
        if not name.startswith('texture_'):
            continue
        if array['count'] != len(array['expressions']):
            raise ValueError('texture expression count mismatch: ' + name)
        if array['count']:
            expressions[name] = array['expressions']
    imports = [dict(owner_path=row['owner_path'], root=item['root'],
                    object_name=item['objectName'], class_name=item['className'])
               for row in chain for item in row['report']['imports']
               if item['className'] in ('Texture2D', 'TextureCube', 'Texture2DArray', 'VolumeTexture')]
    overrides = []
    for row in chain:
        _, textures = batch.serialized_overrides(row)
        if textures:
            overrides.append(dict(owner_path=row['owner_path'], textures=textures))
    return dict(owner_path=owner, base_path=chain[0]['owner_path'], resource_owner_path=resource_path,
                texture_expressions=expressions, texture_import_candidates=imports,
                serialized_texture_overrides=overrides)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, default=batch.GENERATED / 'frontend-probes/frontend-material-inventory/summary.json')
    parser.add_argument('--owner', action='append', help='Exact owner object path; repeat to select multiple owners')
    args = parser.parse_args()
    try:
        owners = batch.load_inventory(args.inventory)
        selected = sorted(set(args.owner) if args.owner else owners)
        missing = [owner for owner in selected if owner not in owners]
        if missing:
            raise ValueError('owner outside inventory: ' + ', '.join(missing))
        archive_path = batch.EXTRACTION / 'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
        results, cache = [], {}
        with batch.bundle.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
            for owner in selected:
                try:
                    result = describe(owner, owners, archive, cache)
                    result['status'] = 'inspected'
                except (ValueError, KeyError, OSError) as error:
                    result = dict(owner_path=owner, status='rejected', reason=str(error))
                results.append(result)
        print(json.dumps(dict(schema='fortnite.frontend-texture-candidates.v1',
                              inventory=batch.source_receipt(args.inventory),
                              limitations=['Imports are candidates, not proven texture bindings',
                                           'No shader translation or runtime support is established'],
                              owners=results), indent=2, allow_nan=False))
    except (ValueError, KeyError, OSError, TypeError) as error:
        print(str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
