#!/usr/bin/env python3
"""Decode captured original TranslucentBasePass metadata and compare source ABI."""
import argparse
import json
from pathlib import Path
import re
from view_layout_contract import align, compare, legacy_resource_hash, original
from translucent_depth_layout_contract import flatten, sha

ROOT = Path(__file__).resolve().parents[1]
CAP = ROOT/'generated/material_pipeline'

def read_original():
    inputs=[]; nested={}
    def decode(chain, asm, obj, typ):
        c, a = CAP/chain, CAP/asm
        inputs.extend([c,a])
        layout=original(json.loads(c.read_text()),a.read_text(),nested=nested)
        nested[obj]=dict(type=typ,layout=layout)
        return layout
    decode('translucent-depth-scene/member-chain.json','translucent-depth-nested/1404c9240.asm',0x1482d2390,'FSceneTexturesUniformParameters')
    for name, init, obj, typ in [('forward','1404abad0',0x14829ef30,'FForwardLightData'),('reflection','1404ac240',0x14829ebb0,'FReflectionUniformParameters'),('planar','1404c08e0',0x1482c30f0,'FPlanarReflectionUniformParameters'),('fog','14049fd60',0x14828cb50,'FFogUniformParameters')]:
        decode(f'translucent-base-{name}/member-chain.json',f'translucent-base-nested/{init}.asm',obj,typ)
    decode('translucent-base-shared/member-chain.json','translucent-base-shared/1404911d0.asm',0x148273e00,'FSharedBasePassUniformParameters')
    inputs.extend([CAP/'translucent-base-identity/refs.json', CAP/'translucent-base-shared/refs.json', CAP/'translucent-base-nested/refs.json'])
    result=decode('translucent-base-identity/member-chain.json','translucent-base-identity/1404912e0.asm',0x148273f20,'FTranslucentBasePassUniformParameters')
    return result,inputs

def source(engine):
    base=engine/'Engine/Source/Runtime'
    files=[base/'Renderer/Private'/x for x in ('BasePassRendering.h','SceneRendering.h','ReflectionEnvironment.h','PlanarReflectionRendering.h','FogRendering.h')]+[base/'Renderer/Public/SceneRenderTargetParameters.h',base/'RenderCore/Public/ShaderParameterMacros.h',base/'RHI/Public/RHI.h',base/'RHI/Public/RHIDefinitions.h',base/'Engine/Public/Matrix3x4.h']
    texts=[p.read_text() for p in files]
    if not re.search(r'#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof\(uint64\)',texts[7]): raise ValueError('unsupported pointer alignment')
    struct_align=int(re.search(r'#define SHADER_PARAMETER_STRUCT_ALIGNMENT (\d+)',texts[7])[1]); array_align=int(re.search(r'#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT (\d+)',texts[7])[1])
    enum=re.search(r'enum EUniformBufferBaseType\s*:\s*uint8\s*\{(.*?)\}',texts[8],re.S)
    names=re.findall(r'\bUBMT_\w+\b',re.sub(r'//[^\n]*|/\*.*?\*/','',enum[1],flags=re.S))
    if names[:9]!=['UBMT_INVALID','UBMT_BOOL','UBMT_INT32','UBMT_UINT32','UBMT_FLOAT32','UBMT_TEXTURE','UBMT_SRV','UBMT_UAV','UBMT_SAMPLER']: raise ValueError('unsupported base types')
    types={'FTranslucentBasePassUniformParameters':0,'FSharedBasePassUniformParameters':0,'FForwardLightData':1,'FReflectionUniformParameters':2,'FPlanarReflectionUniformParameters':3,'FFogUniformParameters':4,'FSceneTexturesUniformParameters':5}
    bodies={}
    for typ,i in types.items():
        found=re.findall(r'BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(?:_WITH_CONSTRUCTOR)?\(\s*'+typ+r'\s*,[^\n]*\)(.*?)END_GLOBAL_SHADER_PARAMETER_STRUCT\(\)',texts[i],re.S)
        if len(found)!=1: raise ValueError('missing structure '+typ)
        bodies[typ]=found[0]
    table=re.search(r'#define FORWARD_GLOBAL_LIGHT_DATA_UNIFORM_BUFFER_MEMBER_TABLE\s*\\\n(.*?)\n\n',texts[1],re.S)[1]
    bodies['FForwardLightData']=bodies['FForwardLightData'].replace('FORWARD_GLOBAL_LIGHT_DATA_UNIFORM_BUFFER_MEMBER_TABLE',table)
    constants={k:int(re.search(r'const int32 '+k+r'\s*=\s*(\d+)',texts[i])[1]) for k,i in [('GMaxForwardShadowCascades',1),('GPlanarReflectionUniformMaxReflectionViews',3)]}
    def decode(typ):
        members=[]; offset=0
        for line in bodies[typ].splitlines():
            line=line.split('//',1)[0].strip().removesuffix('\\').strip()
            if not line: continue
            m=re.fullmatch(r'SHADER_PARAMETER(_STRUCT|_TEXTURE|_SAMPLER|_SRV|_UAV|_ARRAY)?\(\s*([^,]+),\s*(\w+)\s*(?:,\s*\[(\w+)\])?\)',line)
            if not m: raise ValueError('unsupported source member '+line)
            kind,t,name,count=m.groups(); t=t.strip(); elements=(int(count) if count.isdigit() else constants[count]) if count else 0
            if (kind=='_ARRAY') != bool(count): raise ValueError('invalid array')
            if kind=='_STRUCT':
                child=decode(t); size=child['struct_size']; alignment=struct_align
                member=dict(name=name,base_type=17,precision=0,rows=1,columns=1,elements=0,size=size,shader_type=t,members=child['members'])
            elif kind in ('_TEXTURE','_SAMPLER','_SRV','_UAV'):
                size=alignment=8; member=dict(name=name,base_type={'_TEXTURE':5,'_SRV':6,'_UAV':7,'_SAMPLER':8}[kind],precision=0,rows=1,columns=1,elements=0,size=size,shader_type=t)
            else:
                definition=(texts[9] if t=='FMatrix3x4' else texts[6]).split(f'struct TShaderParameterTypeInfo<{t}>',1)[1].split('\n};',1)[0]
                def val(k): return int(re.search(r'\b'+k+r'\s*=\s*(\d+)',definition)[1])
                alignment=val('Alignment'); rows=val('NumRows'); cols=val('NumColumns'); size=4*rows*cols
                if elements: alignment=max(alignment,array_align); size=align(size,array_align)*elements
                member=dict(name=name,base_type=names.index(re.search(r'BaseType = (UBMT_\w+)',definition)[1]),precision=0,rows=rows,columns=cols,elements=elements,size=size,shader_type='')
            offset=align(offset,alignment); member['offset']=offset; offset+=size; members.append(member)
        return dict(struct_size=align(offset,struct_align),members=members)
    return decode('FTranslucentBasePassUniformParameters'),files

def compare_tree(old, new):
    """Compare every nested descriptor and its local member order and size."""
    differences=[]
    def visit(left, right, path):
        a={m['name']:m for m in left['members']}; b={m['name']:m for m in right['members']}
        if left['struct_size'] != right['struct_size']:
            differences.append(dict(path=path,field='struct_size',values=[left['struct_size'],right['struct_size']]))
        if list(a)!=list(b): differences.append(dict(path=path,field='member_order',values=[list(a),list(b)]))
        for name in sorted(a.keys() & b.keys()):
            x,y=a[name],b[name]; child=path+'.'+name if path else name
            for field in ('offset','base_type','precision','rows','columns','elements','size','shader_type'):
                if x[field]!=y[field]: differences.append(dict(path=child,field=field,values=[x[field],y[field]]))
            if 'members' in x and 'members' in y: visit(dict(struct_size=x['size'],members=x['members']),dict(struct_size=y['size'],members=y['members']),child)
    visit(old,new,'')
    return differences

def main():
    p=argparse.ArgumentParser(); p.add_argument('--engine',type=Path,default=ROOT.parent.parent/'UnrealEngine'); p.add_argument('--out',type=Path,default=CAP/'translucent-base-identity/full-layout-contract.json'); p.add_argument('--report',type=Path,default=ROOT/'generated/paper2d-shader-semantics/conversion/d3d.json'); a=p.parse_args()
    old,inputs=read_original(); new,files=source(a.engine); flat=flatten(old); newflat=flatten(new); expected=legacy_resource_hash(flat); associations=[]
    for r in json.loads(a.report.read_text())['records']:
        arrays=[x['uniform_buffer_names'] for x in r['optional_data'] if 'uniform_buffer_names' in x]
        if len(arrays)!=1 or arrays[0].count('TranslucentBasePass')>1: raise ValueError('invalid uniform names')
        if 'TranslucentBasePass' not in arrays[0]: continue
        slot=arrays[0].index('TranslucentBasePass'); hashes=r['resource_maps']['layout_hash']; active=bool(r['resource_table_bits'] & (1<<slot)); actual=hashes[slot] if slot<len(hashes) else 0
        if actual != (expected if active else 0): raise ValueError(f'wrapper layout hash mismatch: {actual} vs {expected}')
        associations.append(dict(shader_index=r['shader_index'],uniform_buffer=slot,resource_table_active=active,original_layout_hash=actual))
    if len(associations)!=2: raise ValueError('unexpected shader association count')
    comparison=compare(flat,newflat); comparison['member_order_match']=[m['name'] for m in flat['members']]==[m['name'] for m in newflat['members']]; comparison['nested_differences']=compare_tree(old,new); comparison['exact_source_layout_match'] &= comparison['member_order_match'] and not comparison['nested_differences']
    result=dict(original=old,current_source=new,comparison=comparison,computed_resource_layout_hash=expected,shader_associations=associations,hash_corroborated=any(x['resource_table_active'] for x in associations),evidence=[sha(x) for x in inputs+[a.report]],source_evidence=[sha(x) for x in files])
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(size=old['struct_size'],current_size=new['struct_size'],computed_hash=expected,hash_corroborated=result['hash_corroborated'],associations=associations,comparison=comparison)))
if __name__=='__main__': main()
