#!/usr/bin/env python3
"""Recover the original dynamic Material metadata for the observed Paper2D shapes."""
import argparse
import json
from pathlib import Path
import struct

from export_original_uniform_contracts import IMAGE, IMAGE_HASH, literal, receipt
from view_layout_contract import legacy_resource_hash

ROOT = Path(__file__).resolve().parents[1]
ASM = ROOT/'generated/material_pipeline/paper2d-material-layout/14434c060.asm'
MANIFEST = ROOT/'generated/paper2d-shader-semantics/shader-manifest.json'
REPORT = ROOT/'generated/paper2d-shader-semantics/conversion/d3d.json'
CTOR = ROOT/'generated/apollo-material-evidence/uniform-ctor/0x143a74db0.asm'
LANDSCAPE_INIT = ROOT/'generated/apollo-material-evidence/terrain-uniform-initializers/0x140506010.asm'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=ROOT/'material_pipeline/evidence/paper2d_material_uniform_layout_runtime/evidence.json')
    args = parser.parse_args()
    require(receipt(IMAGE)['sha256'] == IMAGE_HASH, 'original image mismatch')
    addresses = {0x1459a4b30:'', 0x146ac22e0:'VectorExpressions', 0x146ac2308:'ScalarExpressions',
                 0x146ac2330:'Texture2D_%d', 0x146ac2350:'Texture2D_%dSampler',
                 0x14668ca80:'Texture2D', 0x14668cc10:'SamplerState',
                 0x146ac25e0:'Wrap_WorldGroupSettings', 0x146ac2610:'Clamp_WorldGroupSettings',
                 0x146ac2648:'MaterialUniforms', 0x146708740:'Material',
                 0x1467b66b0:'FLandscapeUniformShaderParameters', 0x1467b6688:'LandscapeParameters'}
    with IMAGE.open('rb') as image:
        require(all(literal(image, a) == s for a, s in addresses.items()), 'original literal mismatch')
        # Original FName initializer, independent of current metadata naming.
        init = 0x140549be0
        expected_init = (bytes.fromhex('41b801000000488d15') +
            struct.pack('<i', 0x146708740 - (init + 13)) + bytes.fromhex('488d0d') +
            struct.pack('<i', 0x148336168 - (init + 20)) + bytes.fromhex('e9') +
            struct.pack('<i', 0x142cd4ab0 - (init + 25)))
        image.seek(init - 0x140000000)
        require(image.read(25) == expected_init, 'original Material FName initializer mismatch')
        image.seek(0x148336168 - 0x140000000)
        require(struct.unpack('<II', image.read(8)) == (0x6f8cd, 0), 'original Material FName global mismatch')
    asm = ASM.read_text()
    ctor = ' '.join(CTOR.read_text().split())
    landscape_init = ' '.join(LANDSCAPE_INIT.read_text().split())
    require(all(x in ctor for x in ('143a74dc2 MOV qword ptr [RCX + 0x8],R9',
        '143a74dd3 MOV RAX,qword ptr [RSP + 0x70]', '143a74ddb MOV qword ptr [RCX + 0x10],RAX',
        '143a74de9 MOV RAX,qword ptr [R8]', '143a74df4 MOV qword ptr [RCX + 0x38],RAX')),
        'original constructor field stores mismatch')
    require(all(x in landscape_init for x in ('1405060c6 LEA R9,[0x1467b66b0]',
        '1405060e3 LEA RAX,[0x1467b6688]', '1405060f2 MOV qword ptr [RSP + 0x20],RAX',
        '1405060f7 CALL 0x143a74db0')), 'landscape constructor identity mismatch')
    # Check the original member stores and control flow, not just literal references.
    checks = [
        '14434c179 MOV EDX,dword ptr [R13 + 0x18]', '14434c1b2 LEA RAX,[0x146ac22e0]',
        '14434c1b9 MOV qword ptr [RCX + 0x8],RSI', '14434c1c0 MOV dword ptr [RCX + 0x10],EBX',
        '14434c1c3 MOV byte ptr [RCX + 0x14],0x4', '14434c1c7 MOV dword ptr [RCX + 0x18],0x1',
        '14434c1ce MOV dword ptr [RCX + 0x1c],0x1', '14434c1d5 MOV dword ptr [RCX + 0x20],0x4',
        '14434c1dc MOV dword ptr [RCX + 0x24],EDX', '14434c1df MOV qword ptr [RCX + 0x28],0x0', '14434c1eb SHL EDX,0x4',
        '14434c1f0 MOV EDX,dword ptr [R13 + 0x28]', '14434c248 LEA RDX,[0x146ac2308]',
        '14434c230 MOV qword ptr [RCX + 0x8],RSI', '14434c235 MOV dword ptr [RCX + 0x10],EBX',
        '14434c23b MOV byte ptr [RCX + 0x14],0x4', '14434c241 MOV dword ptr [RCX + 0x18],0x1',
        '14434c255 MOV qword ptr [RCX],RDX', '14434c266 MOV qword ptr [RCX + 0x28],0x0',
        '14434c258 MOV dword ptr [RCX + 0x1c],0x1', '14434c25f MOV dword ptr [RCX + 0x20],0x4',
        '14434c22d LEA EAX,[RDX + 0x3]', '14434c24f SAR EAX,0x2', '14434c252 MOV dword ptr [RCX + 0x24],EAX',
        '14434c27b SAR EAX,0x2', '14434c27e SHL EAX,0x4',
        '14434c393 LEA RDX,[0x146ac2330]', '14434c3a4 LEA RSI,[R12 + 0x83368c0]',
        '14434c409 LEA RDX,[0x146ac2350]', '14434c41a LEA RSI,[R12 + 0x83370d0]',
        '14434c9d1 CMP dword ptr [R13 + 0x38],R14D', '14434ca1f MOV RAX,qword ptr [RDI + R12*0x1 + 0x83368c0]',
        '14434ca3c MOV qword ptr [RCX + 0x8],RDX', '14434c9c3 LEA RDX,[0x14668ca80]',
        '14434ca40 MOV dword ptr [RCX + 0x10],EBX', '14434ca38 MOV qword ptr [RCX + 0x28],RAX',
        '14434ca43 MOV byte ptr [RCX + 0x14],0x5', '14434ca35 MOV dword ptr [RCX + 0x18],EAX',
        '14434ca47 MOV dword ptr [RCX + 0x1c],0x1', '14434ca4e MOV qword ptr [RCX + 0x20],0x1',
        '14434ca5b ADD EBX,0x8', '14434ca91 MOV RAX,qword ptr [RDI + R12*0x1 + 0x83370d0]',
        '14434c9ca LEA R15,[0x14668cc10]', '14434caae MOV qword ptr [RCX + 0x8],R15',
        '14434cab2 MOV dword ptr [RCX + 0x10],EBX', '14434caaa MOV qword ptr [RCX + 0x28],RAX',
        '14434cab5 MOV byte ptr [RCX + 0x14],0x8', '14434caa7 MOV dword ptr [RCX + 0x18],EAX',
        '14434cab9 MOV dword ptr [RCX + 0x1c],0x1', '14434cac0 MOV qword ptr [RCX + 0x20],0x1',
        '14434cac8 ADD EBX,0x8', '14434d208 LEA RAX,[0x146ac25e0]',
        '14434d1fa LEA RSI,[0x14668cc10]', '14434d20f MOV qword ptr [RCX + 0x8],RSI',
        '14434d216 MOV dword ptr [RCX + 0x10],EBX', '14434d230 MOV qword ptr [RCX + 0x28],R15',
        '14434d219 MOV byte ptr [RCX + 0x14],0x8', '14434d21d MOV dword ptr [RCX + 0x18],R15D',
        '14434d221 MOV dword ptr [RCX + 0x1c],0x1', '14434d228 MOV qword ptr [RCX + 0x20],0x1',
        '14434d239 ADD EBX,0x8', '14434d264 LEA RAX,[0x146ac2610]',
        '14434d26b MOV qword ptr [RCX + 0x8],RSI', '14434d272 MOV dword ptr [RCX + 0x10],EBX',
        '14434d28c MOV qword ptr [RCX + 0x28],R15', '14434d275 MOV byte ptr [RCX + 0x14],0x8', '14434d279 MOV dword ptr [RCX + 0x18],R15D',
        '14434d27d MOV dword ptr [RCX + 0x1c],0x1', '14434d284 MOV qword ptr [RCX + 0x20],0x1',
        '14434d290 LEA EDI,[RBX + 0x17]', '14434d293 AND EDI,0xfffffff0',
        '14434d2b9 MOV EDX,0x2', '14434d2c3 LEA R9,[0x146ac2648]',
        '14434d2ca LEA RAX,[0x146708740]', '14434d2d1 MOV dword ptr [RSP + 0x28],EDI',
        '14434d2d5 LEA R8,[0x148336168]', '14434d2dc MOV qword ptr [RSP + 0x20],RAX',
        '14434d2e8 CALL 0x143a74db0']
    require(all(x in asm for x in checks), 'original instruction capture mismatch')
    def member(name, offset, base, precision, rows, columns, elements, size, shader_type, index=None):
        m = dict(name=name, offset=offset, base_type=base, precision=precision, rows=rows,
                 columns=columns, elements=elements, size=size, shader_type=shader_type)
        if index is not None: m['resource_index'] = index
        return m
    members = [member('VectorExpressions',0,4,1,1,4,2,32,''), member('ScalarExpressions',32,4,1,1,4,1,16,''),
               member('Texture2D_0',48,5,0,1,1,0,8,'Texture2D',0), member('Texture2D_0Sampler',56,8,0,1,1,0,8,'SamplerState',1),
               member('Wrap_WorldGroupSettings',64,8,0,1,1,0,8,'SamplerState',2), member('Clamp_WorldGroupSettings',72,8,0,1,1,0,8,'SamplerState',3)]
    layout = dict(struct_size=80, members=members)
    h = legacy_resource_hash(layout)
    manifest = json.loads(MANIFEST.read_text()); owners = []
    expected_names = ['/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial', '/Paper2D/TranslucentUnlitSpriteMaterial.TranslucentUnlitSpriteMaterial']
    require([o['owner_path'] for o in manifest['owners']] == expected_names, 'unexpected owners')
    debug = dict(constant_bytes=80, resource_offsets=[48,56,64,72], resource_types=[5,8,8,8])
    for o, scalar in zip(manifest['owners'], (1,2)):
        d = o['decoded_resource']; counts = {k:v['count'] for k,v in d['uniform_expression_arrays'].items()}
        require(counts == dict(vector=2, scalar=scalar, texture_2d=1, texture_cube=0, texture_volume=0, texture_virtual=0, texture_external=0, texture_2d_array=0), 'unsupported expression shape')
        require(d['virtual_texture_stacks']['count'] == 0, 'unsupported VT stacks')
        for r in o['shader_records']:
            rec = r['decoded_record']
            require(rec['serialized_expression_counts'] == [2,scalar,1,0,0,0,0] and rec['debug_uniform_buffer_layout'] == debug and rec['layout_name'] == dict(name='Material',number=0), 'shader debug layout mismatch')
        owners.append(dict(owner_path=o['owner_path'], expression_counts=counts, shader_records=len(o['shader_records'])))
    require(sum(o['shader_records'] for o in owners) == 42, 'expected 42 debug layouts')
    bindings = []
    for row in json.loads(REPORT.read_text())['records']:
        arrays = [x['uniform_buffer_names'] for x in row['optional_data'] if 'uniform_buffer_names' in x]
        require(len(arrays) == 1 and arrays[0].count('Material') <= 1, 'invalid wrapper names')
        if 'Material' not in arrays[0]: continue
        slot = arrays[0].index('Material'); active = bool(row['resource_table_bits'] & (1 << slot)); hashes = row['resource_maps']['layout_hash']
        actual = hashes[slot] if slot < len(hashes) else 0
        require(actual == (h if active else 0), 'Material wrapper hash mismatch')
        bindings.append(dict(shader_index=row['shader_index'], slot=slot, active=active, hash=actual))
    require(len(bindings) == 14 and all(x['active'] for x in bindings), 'expected 14 active Material bindings')
    result = dict(original_metadata=dict(function='0x14434c060', constructor='0x143a74db0', constructor_use_case_raw=2,
        constructor_layout_name_literal='Material', constructor_struct_type_literal='MaterialUniforms',
        constructor_shader_variable_literal='Material', constructor_r8_global='0x148336168',
        struct_type_name='MaterialUniforms', layout_name_initializer='0x140549be0', layout_name_fname_index=0x6f8cd, layout_name_fname_number=0,
        constructor_field_identity_evidence=dict(constructor_stores=['0x143a74dc2 stores R9 at metadata +0x8',
            '0x143a74ddb stores stack argument at metadata +0x10', '0x143a74df4 stores dereferenced R8 FName at metadata +0x38'],
            paper2d_caller=['0x14434d2c3 sets R9 to MaterialUniforms', '0x14434d2ca loads Material for stack argument',
                '0x14434d2d5 sets R8 to the Material FName global'],
            independent_landscape_initializer=['0x1405060c6 sets R9 to FLandscapeUniformShaderParameters',
                '0x1405060e3 loads LandscapeParameters for stack argument', '0x1405060f7 calls the same constructor'],
            interpretation='The R9 pointer is struct type, stack pointer is shader variable, and R8 FName is layout identity. The prior report incorrectly called the layout FName the struct type.'),
        layout_name_observed_in_all_debug_records='Material', **layout), owners=owners, computed_legacy_resource_hash=h,
        shader_bindings=bindings, original_instruction_checks=checks, original_literals={hex(a):s for a,s in addresses.items()},
        capture_evidence=[receipt(p) for p in (ASM, MANIFEST, REPORT, CTOR, LANDSCAPE_INIT)], image_sha256=IMAGE_HASH,
        limitations=['Only the two observed expression shapes are supported; other Material layouts are not inferred.',
          'The constructor R8 FName is resolved by original initializer bytes and the original Material literal, independently of current source.',
          'Original metadata is recovered offline; current source and runtime binding/rendering are not verified.'],
        reproduction='From porting: python3 material_pipeline/paper2d_material_layout_contract.py')
    args.out.parent.mkdir(parents=True, exist_ok=True); args.out.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(dict(owners=owners, debug_records=42, bindings=len(bindings), legacy_hash=h)))

if __name__ == '__main__': main()
