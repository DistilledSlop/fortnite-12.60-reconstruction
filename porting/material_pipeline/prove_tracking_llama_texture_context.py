#!/usr/bin/env python3
"""Replay proven original reference-array mechanics for the bounded TrackingLlama case.

Requires generic function property reports from original_texture_function_probe.
Rejects function textures/layers and unknown retained expression classes. It is
an analysis receipt producer, not a general runtime fixture or material parser.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
GENERATED = ROOT / 'porting/generated'
OWNER = 'FortniteGame/Content/UI/Foundation/Modals/ErrorWindow/Materials/M_UI_TrackingLlama'
INVENTORY = GENERATED / ('frontend-probes/frontend-material-inventory-textures/' + OWNER.replace('/', '_') + '.json')
EXPECTED_INPUTS = {'/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-probes/frontend-material-inventory-textures/FortniteGame_Content_UI_Foundation_Modals_ErrorWindow_Materials_M_UI_TrackingLlama.json': '9718be51049012615e232cd845ee8dd6ea1ae74a085e32131f3959abe1ed2d2a', '/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-probes/frontend-material-inventory-textures/FortniteGame_Content_UI_Foundation_Modals_ErrorWindow_Materials_M_UI_TrackingLlama.export-map.json': '8d010e20c722e24dd11c04e87aaeb67e54888bc566b2ca82e12eb0ab0d1bffd2', '/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Foundation/Modals/ErrorWindow/Materials/M_UI_TrackingLlama.uasset': '2b990e50ad79b9fec44b9a0dec87e840a8b8742c2a3c0abee4d0cbb24f02c43b', '/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Foundation/Modals/ErrorWindow/Materials/M_UI_TrackingLlama.uexp': 'f92b78fe4413ca8e5f2bbc3e77d918f77f44556dff2978d2fc4e458cb93af3d7', '/tmp/tracking-llama-texture-context-1/functions/Engine_Content_Functions_Engine_MaterialFunctions02_HueShift.json': 'acd7082b1690802c228952334fb5f2b7d5861624c64d32de07cb13f4da115f3e', '/tmp/tracking-llama-texture-context-1/functions/Engine_Content_Functions_Engine_MaterialFunctions02_Utility_BreakOutFloat2Components.json': '29706f2b0dc8b5ddbd1d10406edf5ec9353d5945b1cfa88dd6f8905e6b10cc04', '/tmp/tracking-llama-texture-context-1/functions/Engine_Content_Functions_Engine_MaterialFunctions02_Utility_DebugTimeSine.json': '45359d89714c03ea8c794fd22099b9a4d8f622b65294f52459879d8b5ca4e798', '/tmp/tracking-llama-texture-context-1/functions/Engine_Content_Functions_Engine_MaterialFunctions02_Utility_MakeFloat2.json': 'c11bfab476478d0a94bf73329e729ab449eeed1735006bff3723a44996ed4724', '/tmp/tracking-llama-texture-context-1/functions/Engine_Content_Functions_Engine_MaterialFunctions02_Utility_MakeFloat3.json': 'a195df0e2ff8a8d9f3cc0cf2f66640d5058334b69f6a1ee98663fda3fccffa60', '/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-probes/frontend-material-inventory-textures/FortniteGame_Content_UI_Foundation_Modals_ErrorWindow_Materials_M_UI_TrackingLlama.decoded.json': 'e8aa7adf01e351a4c95f530398bac040c1a9ac44396ed9a8af44b0ec09ea2616'}
NO_TEXTURE = {'MaterialExpressionScalarParameter', 'MaterialExpressionVectorParameter', 'MaterialExpressionStaticSwitchParameter'}


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def package_index_path(reference):
    path, separator, index = reference['ObjectPath'].rpartition('.')
    if not separator or not index.isdecimal():
        raise ValueError('Expected export-index ObjectPath')
    return path, int(index)


def prove(inventory, function_reports, fresh_inventory):
    for path, expected in EXPECTED_INPUTS.items():
        path = str(function_reports / Path(path).name) if '/functions/' in path else path.replace('/opt/fortnite/FortniteClient-12.60-CL-13477524', str(ROOT))
        if digest(Path(path)) != expected:
            raise ValueError('Pinned input changed: ' + path)
    fresh = fresh_inventory.read_text()
    main = json.loads(inventory.read_text())
    if json.loads(fresh[fresh.index('{'):]) != main:
        raise ValueError('Fresh source inspection differs from inventory')
    if len(main['materialProperties'][0]['expressionRefs']) != 37:
        raise ValueError('Wrong expression array length')
    if main['path'] != OWNER or len(main['materials']) != 1:
        raise ValueError('Only the original TrackingLlama parent is supported')
    functions = {}
    inputs = [{'path': str(inventory), 'sha256': digest(inventory)}]
    export_map_path = inventory.with_name(inventory.stem + '.export-map.json')
    export_map = json.loads(export_map_path.read_text())
    if export_map['package'] != OWNER:
        raise ValueError('Wrong parent export map')
    exports = {x['index']: x['name'] for x in export_map['exports']}
    if len(exports) != len(export_map['exports']) or len(set(exports.values())) != len(exports):
        raise ValueError('Ambiguous parent export map')
    inputs.append({'path': str(export_map_path), 'sha256': digest(export_map_path)})
    for suffix in ['.uasset', '.uexp']:
        source = Path('/mnt/vibrato/fortnite-extracted-full') / (OWNER + suffix)
        inputs.append({'path': str(source), 'sha256': digest(source)})
    for report in sorted(function_reports.glob('*.json')):
        item = json.loads(report.read_text())
        if item['schema'] != 'fortnite.original-texture-function-package.v1':
            raise ValueError('Unexpected function report')
        for source in item['inputs']:
            if digest(Path(source['path'])) != source['sha256']:
                raise ValueError('Function source hash mismatch')
        inputs.append({'path': str(report), 'sha256': digest(report)})
        if item['path'] in functions:
            raise ValueError('Duplicate function report')
        functions[item['path']] = item
    trace = []

    def function_root(path):
        item = functions[path]
        # ObjectPath export indices refer to the original export table. No
        # metadata is allowed here because the report omits its object record.
        if item['skippedMetadata']:
            raise ValueError('Function report has skipped exports')
        objects = item['objects']
        roots = [(index, x) for index, x in enumerate(objects) if x['Type'] == 'MaterialFunction']
        if len(roots) != 1:
            raise ValueError('Expected exactly one function')
        return roots[0]

    def resolve_function(reference):
        path, index = package_index_path(reference)
        root_index, root = function_root(path)
        if index != root_index or reference['ObjectName'] != "MaterialFunction'" + root['Name'] + "'":
            raise ValueError('Function target index/type/name mismatch')
        return path

    def function_objects(path):
        _, root = function_root(path)
        objects = functions[path]['objects']
        if 'FunctionExpressions' not in root['Properties']:
            raise ValueError('Missing serialized FunctionExpressions array')
        for reference in root['Properties']['FunctionExpressions']:
            if reference is None:
                continue
            owner, index = package_index_path(reference)
            if owner != path:
                raise ValueError('External expression reference')
            expression = objects[index]
            if expression['Type'] != 'MaterialExpressionMaterialFunctionCall':
                raise ValueError('Function texture or unproven expression class')
            yield expression

    def visit_function(path, ancestors, result):
        if path in ancestors:
            raise ValueError('Recursive material function')
        for expression in function_objects(path):
            target = expression['Properties'].get('MaterialFunction')
            if target:
                child = resolve_function(target)
                visit_function(child, ancestors | {path}, result)
                if child not in result:  # original callback AddUnique
                    result.append(child)
        # Every non-null FunctionExpression above is a function call, whose
        # original reference-capability virtual is false: no texture append.

    expressions = {x['name']: x for x in main['expressionExports']}
    if len(expressions) != len(main['expressionExports']):
        raise ValueError('Duplicate parent expression')
    ordered = []
    for ordinal, reference in enumerate(main['materialProperties'][0]['expressionRefs']):
        if reference['index'] == 0:
            continue
        if reference['isImport']:
            raise ValueError('Imported root expression')
        if exports[reference['index']] != reference['name']:
            raise ValueError('Parent expression index/name mismatch')
        expression = expressions[reference['name']]
        kind = expression['exportType']
        properties = {p['name']: p['value'] for p in expression['properties']}
        if kind in NO_TEXTURE:
            continue
        if kind == 'MaterialExpressionMaterialFunctionCall':
            match = re.fullmatch(r"MaterialFunction'([^']+)\.([^'.]+)' \(ObjectProperty\)", properties['MaterialFunction'])
            if not match:
                raise ValueError('Unsupported material function reference')
            function = match[1]
            _, root = function_root(function)
            if root['Name'] != match[2]:
                raise ValueError('Top-level function object name mismatch')
            dependencies = []
            visit_function(function, set(), dependencies)
            if function not in dependencies:
                dependencies.append(function)
            trace.append({'expression_position': ordinal, 'function_order': dependencies, 'textures_appended': []})
        elif kind == 'MaterialExpressionTextureSample':
            match = re.fullmatch(r"Texture2D'([^']+)' \(ObjectProperty\)", properties['Texture'])
            if not match:
                raise ValueError('Unsupported texture reference')
            path = match[1]
            if path not in ordered:
                ordered.append(path)
            trace.append({'expression_position': ordinal, 'texture': path, 'index': ordered.index(path)})
        else:
            raise ValueError('Unproven retained expression class: ' + kind)
    decoded_path = inventory.with_name(inventory.stem + '.decoded.json')
    decoded = json.loads(decoded_path.read_text())
    if decoded['package'] != OWNER or not decoded['complete_serialized_shader_map'] or decoded['shader_record_count'] != 19:
        raise ValueError('Expected complete original 19-record TrackingLlama resource')
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
    inputs.append({'path': str(decoded_path), 'sha256': digest(decoded_path)})
    return {'schema': 'fortnite.original-material-texture-context-proof.v1', 'owner': OWNER,
            'inputs': inputs, 'ordered_texture_objects': ordered, 'trace': trace,
            'texture_2d_bindings': bindings, 'original_resource_sha256': decoded['resource_sha256'],
            'scope': 'Original parent reference-array replay only; no fixture conversion, resource sizing or runtime proof.'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--function-reports', type=Path, required=True)
    parser.add_argument('--fresh-inventory', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    if args.out.exists():
        raise ValueError('Output must be new')
    result = prove(INVENTORY, args.function_reports, args.fresh_inventory)
    expected_texture = OWNER.rsplit('/', 1)[0] + '/Icon-Llama-NoEyes.Icon-Llama-NoEyes'
    if result['ordered_texture_objects'] != [expected_texture] or len(result['texture_2d_bindings']) != 1 or result['texture_2d_bindings'][0]['texture_index'] != 0:
        raise ValueError('Unexpected source-to-compiled mapping')
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'owner': result['owner'], 'ordered_texture_objects': result['ordered_texture_objects']}))
