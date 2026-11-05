#!/usr/bin/env python3
"""Decode captured original TranslucentDepthPass metadata and compare source ABI."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from view_layout_contract import align, compare, legacy_resource_hash, original

ROOT = Path(__file__).resolve().parents[1]
CAP = ROOT / 'generated/material_pipeline'

def sha(p):
    return dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest())

def read_original():
    nested = {}
    inputs = []
    for name, init, obj, typ in [('SceneTextures', '1404c9240', '1482d2390', 'FSceneTexturesUniformParameters'), ('TranslucentSelfShadow', '1404d4bd0', '1482e1370', 'FTranslucentSelfShadowUniformParameters')]:
        chain = CAP / f'translucent-depth-{name == "SceneTextures" and "scene" or "shadow"}/member-chain.json'
        asm = CAP / f'translucent-depth-nested/{init}.asm'
        inputs += [chain, asm]
        initializer = asm.read_text()
        top = original(json.loads(chain.read_text()), asm.read_text(),
                   nested={int(n['object'], 16): n for n in nested.values()})
    return top, inputs

def flatten(layout):
    members=[]
    for m in layout['members']:
        if 'members' in m:
            for child in m['members']:
                c = dict(child, name=m['name']+'.'+child['name'], offset=m['offset']+child['offset'])
                c.pop('resource_index', None)
                members.append(c)
        else:
            members.append(dict(m))
    for i, m in enumerate(x for x in members if x['base_type'] in (5,6,8)):
        m['resource_index']=i
    return dict(struct_size=layout['struct_size'], members=members)

def source(engine):
    base = engine / 'Engine/Source/Runtime'
    files = [base/'Renderer/Private/TranslucentLighting.cpp', base/'Renderer/Public/SceneRenderTargetParameters.h', base/'Renderer/Private/ShadowRendering.h', base/'RenderCore/Public/ShaderParameterMacros.h', base/'RHI/Public/RHI.h']
    texts = [p.read_text() for p in files]
    if not re.search(r'#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof\(uint64\)', texts[4]):
        raise ValueError('unsupported pointer alignment')
    struct_align = int(re.search(r'#define SHADER_PARAMETER_STRUCT_ALIGNMENT (\d+)', texts[4])[1])
    bodies={}
    for typ, text in [('FTranslucencyDepthPassUniformParameters', texts[0]), ('FSceneTexturesUniformParameters', texts[1]), ('FTranslucentSelfShadowUniformParameters', texts[2])]:
        pattern=r'BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT\('+typ+r'\s*,[^\n]*\)(.*?)END_GLOBAL_SHADER_PARAMETER_STRUCT\(\)'
        matches=re.findall(pattern,text,re.S)
        if len(matches)!=1: raise ValueError('missing source structure '+typ)
        bodies[typ]=matches[0]
    def decode(typ):
        members=[]; offset=0
        for line in bodies[typ].splitlines():
            line=line.split('//',1)[0].strip()
            if not line: continue
            match=re.fullmatch(r'SHADER_PARAMETER(_STRUCT|_TEXTURE|_SAMPLER|_SRV)?\(([^,]+),\s*(\w+)\)',line)
            if not match: raise ValueError('unsupported source member '+line)
            kind,t,name=match.groups()
            if kind=='_STRUCT':
                child=decode(t); size=child['struct_size']; alignment=struct_align
                m=dict(name=name,base_type=17,precision=0,rows=1,columns=1,elements=0,size=size,shader_type=t,members=child['members'])
            elif kind:
                size=alignment=8
                m=dict(name=name,base_type={'_TEXTURE':5,'_SAMPLER':8,'_SRV':6}[kind],precision=0,rows=1,columns=1,elements=0,size=size,shader_type=t)
            else:
                definition=texts[3].split(f'struct TShaderParameterTypeInfo<{t}>',1)[1].split('\n};',1)[0]
                def val(k): return int(re.search(r'\b'+k+r'\s*=\s*(\d+)',definition)[1])
                alignment=val('Alignment'); rows=val('NumRows'); cols=val('NumColumns'); size=4*rows*cols
                m=dict(name=name,base_type={'UBMT_FLOAT32':4}[re.search(r'BaseType = (UBMT_\w+)',definition)[1]],precision=0,rows=rows,columns=cols,elements=0,size=size,shader_type='')
            offset=align(offset,alignment); m['offset']=offset; offset+=size; members.append(m)
        return dict(struct_size=align(offset,struct_align), members=members)
    return decode('FTranslucencyDepthPassUniformParameters'), files

def main():
    p=argparse.ArgumentParser(); p.add_argument('--engine',type=Path,default=ROOT.parent.parent/'UnrealEngine'); p.add_argument('--out',type=Path,default=CAP/'translucent-depth-identity/full-layout-contract.json'); p.add_argument('--report',type=Path,default=ROOT/'generated/paper2d-shader-semantics/conversion/d3d.json'); a=p.parse_args()
    old, inputs=read_original(); new, files=source(a.engine)
    flat=flatten(old); expected=legacy_resource_hash(flat); associations=[]
    for r in json.loads(a.report.read_text())['records']:
        arrays=[x['uniform_buffer_names'] for x in r['optional_data'] if 'uniform_buffer_names' in x]
        if len(arrays)!=1 or arrays[0].count('TranslucentDepthPass')>1: raise ValueError('invalid uniform names')
        if 'TranslucentDepthPass' not in arrays[0]: continue
        slot=arrays[0].index('TranslucentDepthPass'); hashes=r['resource_maps']['layout_hash']; active=bool(r['resource_table_bits'] & (1<<slot)); actual=hashes[slot] if slot<len(hashes) else 0
        if actual != (expected if active else 0): raise ValueError('wrapper layout hash mismatch')
        associations.append(dict(shader_index=r['shader_index'],uniform_buffer=slot,resource_table_active=active,original_layout_hash=actual))
    if len(associations)!=3: raise ValueError('unexpected shader association count')
    comparison=compare(flat,flatten(new)); comparison['member_order_match']=[m['name'] for m in flat['members']]==[m['name'] for m in flatten(new)['members']]; comparison['exact_source_layout_match'] &= comparison['member_order_match']
    result=dict(original=old,current_source=new,comparison=comparison,computed_resource_layout_hash=expected,shader_associations=associations,hash_corroborated=any(x['resource_table_active'] for x in associations),evidence=[sha(x) for x in inputs+[a.report]],source_evidence=[sha(x) for x in files])
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(size=old['struct_size'],computed_hash=expected,hash_corroborated=result['hash_corroborated'],associations=associations,comparison=comparison)))
if __name__=='__main__': main()
