#!/usr/bin/env python3
"""Decode original ShadowDepthPass metadata and compare the current source ABI."""
import argparse
import json
from pathlib import Path
import re
from view_layout_contract import align, compare, legacy_resource_hash, original
from translucent_depth_layout_contract import flatten, sha

ROOT = Path(__file__).resolve().parents[1]
CAP = ROOT / 'generated/material_pipeline'


def read_original():
    nested = {}
    inputs = []
    for chain, asm, obj, typ in [
        ('translucent-depth-scene/member-chain.json', 'translucent-depth-nested/1404c9240.asm', 0x1482d2390, 'FSceneTexturesUniformParameters'),
        ('shadow-depth-lpv/member-chain.json', 'shadow-depth-lpv/1404ac130.asm', 0x14829f4a0, 'FLpvWriteUniformBufferParameters'),
    ]:
        c, a = CAP / chain, CAP / asm
        inputs += [c, a]
        nested[obj] = dict(type=typ, layout=original(json.loads(c.read_text()), a.read_text()))
    chain, asm = CAP/'shadow-depth-identity/member-chain.json', CAP/'shadow-depth-identity/1404d4ac0.asm'
    inputs += [chain, asm]
    return original(json.loads(chain.read_text()), asm.read_text(), nested=nested), inputs


def source(engine):
    base = engine / 'Engine/Source/Runtime'
    files = [base/'Renderer/Private/ShadowRendering.h', base/'Renderer/Public/SceneRenderTargetParameters.h',
             base/'Renderer/Private/LightPropagationVolume.h', base/'RenderCore/Public/ShaderParameterMacros.h',
             base/'RHI/Public/RHI.h', base/'RHI/Public/RHIDefinitions.h']
    texts = [p.read_text() for p in files]
    if not re.search(r'#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof\(uint64\)', texts[4]):
        raise ValueError('unsupported pointer alignment')
    struct_align = int(re.search(r'#define SHADER_PARAMETER_STRUCT_ALIGNMENT (\d+)', texts[4])[1])
    array_align = int(re.search(r'#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT (\d+)', texts[4])[1])
    bodies = {}
    for typ, text in [('FShadowDepthPassUniformParameters', texts[0]), ('FSceneTexturesUniformParameters', texts[1]), ('FLpvWriteUniformBufferParameters', texts[2])]:
        matches = re.findall(r'BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT\(\s*'+typ+r'\s*,[^\n]*\)(.*?)END_GLOBAL_SHADER_PARAMETER_STRUCT\(\)', text, re.S)
        if len(matches) != 1: raise ValueError('missing source structure '+typ)
        bodies[typ] = matches[0]
    enum = re.search(r'enum EUniformBufferBaseType\s*:\s*uint8\s*\{(.*?)\}', texts[5], re.S)
    if not enum: raise ValueError('missing base type enum')
    names = re.findall(r'\bUBMT_\w+\b', re.sub(r'//[^\n]*|/\*.*?\*/', '', enum[1], flags=re.S))
    if names[:9] != ['UBMT_INVALID', 'UBMT_BOOL', 'UBMT_INT32', 'UBMT_UINT32', 'UBMT_FLOAT32', 'UBMT_TEXTURE', 'UBMT_SRV', 'UBMT_UAV', 'UBMT_SAMPLER']:
        raise ValueError('unsupported base type enum')
    def decode(typ):
        members=[]; offset=0
        for line in bodies[typ].splitlines():
            line=line.split('//',1)[0].strip()
            if not line: continue
            match=re.fullmatch(r'SHADER_PARAMETER(_STRUCT|_TEXTURE|_SAMPLER|_SRV|_UAV|_ARRAY)?\(\s*([^,]+),\s*(\w+)\s*(?:,\s*\[(\d+)\])?\)',line)
            if not match: raise ValueError('unsupported source member '+line)
            kind,t,name,count=match.groups(); t=t.strip(); elements=int(count or 0)
            if (kind == '_ARRAY') != bool(count): raise ValueError('invalid array declaration')
            if kind=='_STRUCT':
                child=decode(t); size=child['struct_size']; alignment=struct_align
                m=dict(name=name,base_type=17,precision=0,rows=1,columns=1,elements=0,size=size,shader_type=t,members=child['members'])
            elif kind in ('_TEXTURE','_SAMPLER','_SRV','_UAV'):
                size=alignment=8
                m=dict(name=name,base_type={'_TEXTURE':5,'_SAMPLER':8,'_SRV':6,'_UAV':7}[kind],precision=0,rows=1,columns=1,elements=0,size=size,shader_type=t)
            else:
                source_type = 'int32' if t == 'int' else t
                definition=texts[3].split(f'struct TShaderParameterTypeInfo<{source_type}>',1)[1].split('\n};',1)[0]
                def val(k): return int(re.search(r'\b'+k+r'\s*=\s*(\d+)',definition)[1])
                alignment=val('Alignment'); rows=val('NumRows'); cols=val('NumColumns'); size=4*rows*cols
                if elements: alignment=max(alignment,array_align); size=align(size,array_align)*elements
                m=dict(name=name,base_type=names.index(re.search(r'BaseType = (UBMT_\w+)',definition)[1]),precision=0,rows=rows,columns=cols,elements=elements,size=size,shader_type='')
            offset=align(offset,alignment); m['offset']=offset; offset+=size; members.append(m)
        return dict(struct_size=align(offset,struct_align),members=members)
    return decode('FShadowDepthPassUniformParameters'), files


def main():
    p=argparse.ArgumentParser(); p.add_argument('--engine',type=Path,default=ROOT.parent.parent/'UnrealEngine'); p.add_argument('--out',type=Path,default=CAP/'shadow-depth-identity/full-layout-contract.json'); p.add_argument('--report',type=Path,default=ROOT/'generated/paper2d-shader-semantics/conversion/d3d.json'); a=p.parse_args()
    old,inputs=read_original(); new,files=source(a.engine); flat=flatten(old); newflat=flatten(new)
    # Flatten's shared implementation numbers the original resource types; current UAVs are resources too.
    for i,m in enumerate(x for x in newflat['members'] if x['base_type'] in (5,6,7,8)): m['resource_index']=i
    expected=legacy_resource_hash(flat); associations=[]
    for r in json.loads(a.report.read_text())['records']:
        arrays=[x['uniform_buffer_names'] for x in r['optional_data'] if 'uniform_buffer_names' in x]
        if len(arrays)!=1 or arrays[0].count('ShadowDepthPass')>1: raise ValueError('invalid uniform names')
        if 'ShadowDepthPass' not in arrays[0]: continue
        slot=arrays[0].index('ShadowDepthPass'); hashes=r['resource_maps']['layout_hash']; active=bool(r['resource_table_bits'] & (1<<slot)); actual=hashes[slot] if slot<len(hashes) else 0
        if actual != (expected if active else 0): raise ValueError('wrapper layout hash mismatch')
        associations.append(dict(shader_index=r['shader_index'],uniform_buffer=slot,resource_table_active=active,original_layout_hash=actual))
    if len(associations)!=4: raise ValueError('unexpected shader association count')
    comparison=compare(flat,newflat); comparison['member_order_match']=[m['name'] for m in flat['members']]==[m['name'] for m in newflat['members']]; comparison['exact_source_layout_match'] &= comparison['member_order_match']
    result=dict(original=old,current_source=new,comparison=comparison,computed_resource_layout_hash=expected,shader_associations=associations,hash_corroborated=any(x['resource_table_active'] for x in associations),evidence=[sha(x) for x in inputs+[a.report]],source_evidence=[sha(x) for x in files])
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(size=old['struct_size'],current_size=new['struct_size'],computed_hash=expected,hash_corroborated=result['hash_corroborated'],associations=associations,comparison=comparison)))
if __name__=='__main__': main()
