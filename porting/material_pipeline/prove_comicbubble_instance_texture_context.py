#!/usr/bin/env python3
"""Bind ComicBubble_Inst's regular Texture uniforms through the proven original instance path.

Bounded analysis only. Original instance +0x3a0 appends overridden layer textures
before invoking its parent. The original constructor makes the missing layer
array empty. This owner has no serialized layer entries, so its ordered array
equals the separately proven parent array, including unused reference entries.
"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GENERATED = ROOT / 'porting/generated'
OWNER = 'FortniteGame/Content/Environments/FrontEnd/BackPlates/UITakeovers/M_UI_ComicBubble_Inst'
PARENT = OWNER.removesuffix('_Inst')
PARENT_PROOF_SHA256 = 'e7cf1f59b06e26f3ca4b5191c44cd86cfdd56c6e402c3d2f683001cb948a6a51'


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def prove(report_path, parent_proof_path):
    if digest(parent_proof_path) != PARENT_PROOF_SHA256:
        raise ValueError('Unpinned parent context proof')
    parent = json.loads(parent_proof_path.read_text())
    if parent['owner'] != PARENT:
        raise ValueError('Wrong parent proof')
    report = json.loads(report_path.read_text())
    if report['schema'] != 'fortnite.original-texture-instance-package.v1' or report['path'] != OWNER:
        raise ValueError('Wrong instance report')
    if report['skippedMetadata'] or len(report['objects']) != 1:
        raise ValueError('Unexpected instance exports')
    instance = report['objects'][0]
    if instance['Type'] != 'MaterialInstanceConstant' or instance['Name'] != 'M_UI_ComicBubble_Inst':
        raise ValueError('Wrong instance object')
    properties = instance['Properties']
    if properties['bHasStaticPermutationResource'] is not True:
        raise ValueError('Expected original static permutation')
    target = properties['Parent']
    if target != {'ObjectName': "Material'M_UI_ComicBubble'", 'ObjectPath': PARENT + '.0'}:
        raise ValueError('Wrong parent object reference')
    static = properties['StaticParameters']
    if static.get('MaterialLayersParameters', []) != []:
        raise ValueError('Layer parameters require a separate ordering proof')
    if set(static) - {'StaticSwitchParameters', 'MaterialLayersParameters'}:
        raise ValueError('Unsupported static parameter fields')
    if properties.get('TextureParameterValues', []) != []:
        raise ValueError('Texture overrides outside bounded proof')
    inputs = []
    for source in report['inputs']:
        if digest(Path(source['path'])) != source['sha256']:
            raise ValueError('Instance package source hash mismatch')
        inputs.append(source)
    inputs += [{'path': str(report_path), 'sha256': digest(report_path)},
               {'path': str(parent_proof_path), 'sha256': digest(parent_proof_path)}]
    stem = OWNER.replace('/', '_')
    inventory = GENERATED / 'frontend-probes/frontend-material-inventory-textures'
    decoded_path = inventory / (stem + '.decoded.json')
    export_map_path = inventory / (stem + '.export-map.json')
    export_map = json.loads(export_map_path.read_text())
    if export_map['package'] != OWNER or len(export_map['exports']) != 1 or export_map['exports'][0]['index'] != 1 or export_map['exports'][0]['name'] != instance['Name']:
        raise ValueError('Wrong instance export map')
    decoded = json.loads(decoded_path.read_text())
    if decoded['package'] != OWNER or not decoded['complete_serialized_shader_map'] or decoded['shader_record_count'] != 19:
        raise ValueError('Expected complete original 19-record instance resource')
    ordered = parent['ordered_texture_objects']
    bindings = []
    for ordinal, expression in enumerate(decoded['uniform_expression_arrays']['texture_2d']['expressions']):
        if expression['type'] != 'FMaterialUniformExpressionTexture':
            raise ValueError('Expected regular Texture uniform')
        texture = expression['texture']
        if texture['virtual_texture'] or texture['layer_index'] != -1 or texture['page_table_layer_index'] != -1:
            raise ValueError('Unsupported layered/virtual texture')
        index = texture['texture_index']
        if index < 0 or index >= len(ordered):
            raise ValueError('Unresolved original texture index')
        bindings.append({'uniform_position': ordinal, **texture, 'texture_object': ordered[index]})
    inputs += [{'path': str(p), 'sha256': digest(p)} for p in [export_map_path, decoded_path]]
    return {'schema': 'fortnite.original-material-instance-texture-context-proof.v1', 'owner': OWNER,
            'inputs': inputs, 'ordered_texture_objects': ordered, 'texture_2d_bindings': bindings,
            'original_resource_sha256': decoded['resource_sha256'],
            'instance_layer_entries': 0, 'layer_array_tag_present': 'MaterialLayersParameters' in static,
            'scope': 'Original regular Texture reference identity only; no dimensions, conversion, arbitrary instance support or runtime proof.'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--instance-report', type=Path, required=True)
    parser.add_argument('--parent-proof', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    if args.out.exists():
        raise ValueError('Output must be new')
    result = prove(args.instance_report, args.parent_proof)
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'owner': OWNER, 'texture_2d_bindings': result['texture_2d_bindings']}))
