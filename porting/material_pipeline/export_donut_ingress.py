#!/usr/bin/env python3
"""Bind Donut's original Icon texture, real family override and shader resources."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('numeric', HERE / 'export_numeric_ingress.py')
numeric = importlib.util.module_from_spec(spec)
spec.loader.exec_module(numeric)
ROOT, GENERATED = numeric.ROOT, numeric.GENERATED
FOLDER = '/Game/Athena/UI/Challenges/Art/Donut/'


def report(path):
    raw = path.read_bytes()
    return json.loads(raw[raw.index(b'{'):]), raw


def create_bundle():
    framing_path, decoded_path = ROOT/'donut-inline-resource.json', ROOT/'donut-legacy-prefix.json'
    framing_raw, decoded_raw = framing_path.read_bytes(), decoded_path.read_bytes()
    framing, decoded = json.loads(framing_raw), json.loads(decoded_raw)
    source = numeric.EXTRACTION/'Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons.uexp'
    source_raw = source.read_bytes()
    payload = numeric.bundle.validate_source(source_raw, framing)
    parsed = numeric.bundle.decoder.parse_prefix(payload, framing['names'])
    library = Path(decoded['shader_library']['path'])
    library_hash = numeric.bundle.library_reader.sha256(library)
    if library_hash != decoded['shader_library']['sha256']:
        raise ValueError('original shader library changed')
    code_receipts = []
    with numeric.bundle.library_reader.ShaderArchive(library, layout='fortnite-12.60') as archive:
        numeric.bundle.decoder.associate_library(parsed, archive)
        for record in numeric.bundle.all_records(parsed):
            index = record['library_association']['index']
            code = archive.shader_code(index)
            code_receipts.append(dict(type=record['type'], output_hash=record['output_hash']['hash'],
                library_index=index, frequency=record['frequency'], bytes=len(code), sha256=numeric.digest(code)))
    for key, value in parsed.items():
        if key == 'original_serializer_evidence':
            if any(decoded[key].get(k) != v for k, v in value.items()):
                raise ValueError('original serializer evidence changed')
        elif decoded.get(key) != value:
            raise ValueError('original complete parse changed: '+key)
    if parsed['undecoded_bytes'] or parsed['shader_record_count'] != 19:
        raise ValueError('original shader resource is incomplete')
    arrays = parsed['uniform_expression_arrays']
    expressions = arrays['texture_2d']['expressions']
    if len(expressions) != 1 or any(v['count'] for k,v in arrays.items() if k.startswith('texture_') and k!='texture_2d'):
        raise ValueError('unexpected original Donut texture array')
    expression = expressions[0]
    parameter = expression['parameter']
    if (expression['type'] != 'FMaterialUniformExpressionTextureParameter' or
            parameter != dict(name=dict(name='Icon', number=0), association=2, index=-1) or
            expression['texture'] != dict(texture_index=0, layer_index=-1, page_table_layer_index=-1,
                                          sampler_source=0, virtual_texture=False)):
        raise ValueError('unexpected original Icon parameter contract')
    owner_path = FOLDER+'M_UI_DonutIcons.M_UI_DonutIcons'
    default_path = FOLDER+'T_UI_Donut_PC_Icon.T_UI_Donut_PC_Icon'
    override_path = FOLDER+'T_UI_Donut_App_Icon.T_UI_Donut_App_Icon'
    instance_path = FOLDER+'M_UI_DonutIcons_App2.M_UI_DonutIcons_App2'
    owner_report_path = GENERATED/'frontend-probes/material-donut.json'
    owner, owner_raw = report(owner_report_path)
    texture_export = next(row for row in owner['expressionExports'] if row['name']=='MaterialExpressionTextureSampleParameter2D_0')
    properties = {row['name']:row['value'] for row in texture_export['properties']}
    if properties['ParameterName']!='Icon (NameProperty)' or properties['Texture'] != "Texture2D'FortniteGame/Content/Athena/UI/Challenges/Art/Donut/T_UI_Donut_PC_Icon.T_UI_Donut_PC_Icon' (ObjectProperty)":
        raise ValueError('original texture expression default does not match indexed reference')
    instance_report_path = GENERATED/'frontend-probes/material-donut-app2.json'
    instance, instance_raw = report(instance_report_path)
    info = instance['materials'][0]
    override = instance['instanceParameters'][0]['textures']
    if (info['type']!='UMaterialInstanceConstant' or info['name']!='M_UI_DonutIcons_App2' or
            info['parent']!='M_UI_DonutIcons' or info['staticPermutation'] or
            override != [dict(parameter='Icon', texture='T_UI_Donut_App_Icon', package=FOLDER+'T_UI_Donut_App_Icon')]):
        raise ValueError('original App2 family override differs from source')
    source_inputs = [(framing_path,framing_raw),(decoded_path,decoded_raw),(owner_report_path,owner_raw),
                     (instance_report_path,instance_raw),(source,source_raw),
                     (source.with_suffix('.uasset'),source.with_suffix('.uasset').read_bytes())]
    textures = []
    for key, expected in [('default',default_path),('override',override_path)]:
        path = ROOT/('donut-texture-'+key)/'texture-report.json'
        meta, raw = report(path)
        if numeric.package_path(meta['package'])+'.'+meta['name'] != expected:
            raise ValueError('decoded texture report identity differs from source')
        pixels = Path(meta['rgba']).read_bytes()
        if (meta['decodedPixelFormat']!='PF_R8G8B8A8' or len(pixels)!=meta['width']*meta['height']*4 or
                numeric.digest(pixels)!=meta['rgbaSha256'] or numeric.digest(Path(meta['png']).read_bytes())!=meta['pngSha256']):
            raise ValueError('CPU decoded original texture pixels changed')
        meta['asset_path'] = expected
        source_inputs.append((path,raw))
        for suffix in ('.uasset','.uexp','.ubulk'):
            original = numeric.EXTRACTION/(numeric.package_path(meta['package']).removeprefix('/Game/')+suffix)
            source_inputs.append((original,original.read_bytes()))
        textures.append(meta)
    if textures[0]['rgbaSha256']==textures[1]['rgbaSha256']:
        raise ValueError('family textures do not exercise a source-content change')
    nodes, roots = numeric.bundle.fixture_exporter.flatten_expressions(arrays)
    if len(nodes)!=12 or len(roots['vector_roots'])!=4 or len(roots['scalar_roots'])!=5:
        raise ValueError('original Donut numeric graph changed')
    layout = dict(constant_bytes=128, resource_offsets=[96,104,112,120], resource_types=[5,8,8,8])
    records = parsed['material_shaders']['records']
    if len(records)!=7 or any(row['debug_uniform_buffer_layout']!=layout for row in records):
        raise ValueError('original texture resource layout changed')
    mapped_path, reflected_path = ROOT/'frontend-mapped/mapped-translation-report.json', ROOT/'mapped-spirv-reflection.json'
    mapped_raw, reflected_raw = mapped_path.read_bytes(), reflected_path.read_bytes()
    mapped, reflected = json.loads(mapped_raw),json.loads(reflected_raw)
    if mapped['archive_sha256']!=library_hash or reflected['mapped_report_sha256']!=numeric.digest(mapped_raw):
        raise ValueError('mapped texture shader provenance changed')
    rows = [{r['shader_index']:r for r in doc['records']} for doc in (mapped,reflected)]
    shaders=[]
    for record in records:
        index=record['library_association']['index']; shader,refl=rows[0][index],rows[1][index]
        code=Path(refl['output']).read_bytes()
        if not shader['validated'] or numeric.digest(code)!=shader['output_sha256'] or numeric.digest(code)!=refl['spirv_sha256']:
            raise ValueError('mapped original texture program changed')
        shaders.append(dict(record=record,mapped=shader,reflection=refl,spirv_sha1=hashlib.sha1(code).hexdigest()))
    parameters={n['parameter']['name']['name']:dict(type='scalar' if n['type']=='ScalarParameter' else 'vector',parameter=n['parameter'])
                for n in nodes if n['type'] in ('ScalarParameter','VectorParameter')}
    def case(name,values,unmatched=False):
        entries=[dict(copy.deepcopy(parameters[key]),value=value) for key,value in values]
        if unmatched:entries[0]['parameter']['index']=7
        raw=numeric.evaluator.evaluate_report(parsed,numeric.evaluator.overrides_from_json([] if unmatched else entries))[0]
        return dict(name=name,overrides=entries,numeric_values=list(struct.unpack('<'+'f'*(len(raw)//4),raw)),
                    numeric_sha256=numeric.digest(raw),abs_tolerance=0.00001,rel_tolerance=0.00001)
    cases=[case('defaults',[]),case('Tint',[('Tint',1.0)]),case('EraseLine',[('EraseLine',0.5)]),
           case('TintColor',[('TintColor',[0.1,0.8,0.3,0.9])]),case('unmatched_index',[('Tint',1.0)],True)]
    fixture=dict(schema='fortnite.legacy-slate.v3',material={key:parsed[key] for key in ('friendly_name','material_path','quality_level',
        'feature_level','cooked_shader_map_id_hash','debug_description')},expected_layout=layout,nodes=nodes,**roots,shaders=shaders,
        numeric_comparison_bytes=84,uniform_cases=cases,owner_uniform_cases=copy.deepcopy(cases),owner_parameter_expectations=[],
        draw_parameter='TintColor',texture_2d_parameters=[dict(parameter=parameter,texture_index=0,texture_layer_index=-1,
            page_table_layer_index=-1,sampler_source=0,virtual_texture=False,default_texture_path=default_path)],
        texture_contract=dict(parameter=parameter,default_texture_path=default_path,override_texture_path=override_path,
            instance_path=instance_path,default=textures[0],override=textures[1],
            scope='Original indexed default and actual App2 Icon override; CPU pixels are a source decode, not a rendered original-client oracle'),
        provenance=dict(resource_sha256=parsed['resource_sha256'],shader_library_sha256=library_hash))
    source_inputs.extend([(mapped_path,mapped_raw),(reflected_path,reflected_raw)])
    return dict(schema='fortnite.legacy-material-ingress.v3',owner_path=owner_path,owner_type='UMaterial',parent_path='',
        framing=framing,decoded=decoded,runtime_fixture=fixture,resource_raw_hex=payload.hex(),original_shader_code=code_receipts,
        provenance=dict(inputs=[dict(path=str(p.resolve()),bytes=len(raw),sha256=numeric.digest(raw)) for p,raw in source_inputs],
            resource_sha1=hashlib.sha1(payload).hexdigest(),shader_library=dict(path=str(library),sha256=library_hash),
            exporter_sha256=numeric.digest(Path(__file__).read_bytes())),
        limitations=['Donut original texture sampling and owner reference restoration require runtime validation',
                    'Other texture types, full cached-expression APIs, Editor and original pixel parity remain unverified'])


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'donut-ingress-bundle.json')
    args=parser.parse_args()
    try:
        if args.output.exists() or GENERATED.resolve() not in args.output.resolve().parents:
            raise ValueError('choose a fresh output under generated to preserve pinned bundles')
        data=create_bundle();raw=(json.dumps(data,indent=2,allow_nan=False)+'\n').encode()
        args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(raw)
        print(json.dumps(dict(path=str(args.output.resolve()),bundle_sha1=hashlib.sha1(raw).hexdigest(),
            resource_sha1=data['provenance']['resource_sha1'],resource_bytes=len(bytes.fromhex(data['resource_raw_hex'])),
            texture_contract=data['runtime_fixture']['texture_contract']['scope'])))
    except (OSError,ValueError,KeyError,TypeError,StopIteration) as error:
        parser.exit(1,f'error: {error}\n')


if __name__=='__main__':main()
