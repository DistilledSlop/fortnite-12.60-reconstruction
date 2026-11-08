#!/usr/bin/env python3
"""Derive tracking-llama-bundle-3 from the pinned bundle-2 by proving the Llama texture's effective sampling state.

bundle-2 is left untouched. This script re-reads the ORIGINAL Icon-Llama-NoEyes .uasset/.uexp/.ubulk, proves that the
serialized property list contains only ImportedSize and LightingGuid (so sRGB/address/filter/LOD group/compression come
from the UTexture2D class defaults), and records the UE 4.25 source defaults those values come from.
"""
import argparse, copy, hashlib, json, struct
from pathlib import Path
HERE=Path(__file__).resolve().parent
GENERATED=HERE.parent/'generated'
BUNDLE2=GENERATED/'material_pipeline/tracking-llama-bundle-2/M_UI_TrackingLlama.json'
BUNDLE2_SHA1='7cd748bc94085ebc3b24462b181d8d10cc21f9e2'
PACKAGE=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Foundation/Modals/ErrorWindow/Materials/Icon-Llama-NoEyes')
ENGINE=Path('/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine')
SOURCE_SHA256=dict(uasset='4c415a1c34a4b8d3f69219ab5485a61233abb7625e730473a89c63ebb0aea9ac',uexp='2fdb9853caa5343ac5ee76d14edf30d4cfb7d4e9d16dfac75eb26f6e85283a3a',ubulk='0e6187d315f165e7005e2b769afcdf2fef1b9e554079a1cdd45cdc0c9b367671')
SOURCE_BYTES=dict(uasset=748,uexp=6018,ubulk=81920)
def require(ok,msg):
    if not ok: raise ValueError(msg)
def sha256(raw): return hashlib.sha256(raw).hexdigest()

def parse_names(asset):
    # legacy -7 summary: folder name at 28, PackageFlags at 37 (see proof notes in the bundle).
    require(struct.unpack_from('<i',asset,4)[0]==-7 and struct.unpack_from('<I',asset,24)[0]==len(asset),'unexpected uasset summary')
    flen=struct.unpack_from('<i',asset,28)[0]; require(asset[32:32+flen-1]==b'None','unexpected folder name')
    _flags,count,offset=struct.unpack_from('<Iii',asset,32+flen)
    names=[]; p=offset
    for _ in range(count):
        n=struct.unpack_from('<i',asset,p)[0]; require(0<n<256,'bad name'); names.append(asset[p+4:p+3+n].decode('ascii')); p+=4+n+4
    return names

def parse_properties(exports,names):
    """Walk FPropertyTag records at the start of the Texture2D export until None."""
    p=0; result=[]
    while True:
        idx,num=struct.unpack_from('<ii',exports,p); name=names[idx]; p+=8
        if name=='None': return result,p
        tidx,_tn,size,array=struct.unpack_from('<iiii',exports,p); kind=names[tidx]; p+=16
        require(kind=='StructProperty' and array==0,'unexpected property tag kind')
        sidx,_sn=struct.unpack_from('<ii',exports,p); struct_name=names[sidx]; p+=8+16
        require(exports[p]==0,'property guid present'); p+=1
        value=exports[p:p+size]; p+=size; result.append(dict(name=name,struct=struct_name,value_hex=value.hex()))

def prove_properties():
    files={k:(PACKAGE.with_suffix('.'+k)).read_bytes() for k in SOURCE_SHA256}
    for k,v in files.items(): require(len(v)==SOURCE_BYTES[k] and sha256(v)==SOURCE_SHA256[k],'original %s changed'%k)
    names=parse_names(files['uasset'])
    require('Default__Texture2D' in names and 'Texture2D' in names and 'PF_DXT5' in names,'unexpected archetype/class names')
    props,end=parse_properties(files['uexp'],names)
    require([p['name'] for p in props]==['ImportedSize','LightingGuid'],'serialized property list changed')
    require(props[0]['struct']=='IntPoint' and struct.unpack('<ii',bytes.fromhex(props[0]['value_hex']))==(256,256),'ImportedSize changed')
    absent=['SRGB','AddressX','AddressY','Filter','LODGroup','CompressionSettings','LODBias','NeverStream','bNoTiling','MipGenSettings']
    require(not any(a in names for a in absent),'name table contains a texture-setting property name')
    return dict(name_table=names,serialized_properties=[p['name'] for p in props],property_list_end_offset=end,absent_property_names=absent)

def line(path,needle):
    for i,t in enumerate(path.read_text().splitlines(),1):
        if needle in t: return '%s:%d'%(path.relative_to(ENGINE.parents[2]),i)
    raise ValueError('source evidence missing: '+needle)

def defaults_evidence():
    tex=ENGINE/'Private/Texture.cpp'; tex2=ENGINE/'Private/Texture2D.cpp'
    group=ENGINE/'Classes/Engine/TextureDefines.h'; header=ENGINE/'Classes/Engine/Texture.h'
    return dict(srgb=dict(value=True,source=line(tex,'SRGB = true;')+' and '+line(tex2,'SRGB = true;')),
        filter=dict(value='TF_Default',source=line(tex,'Filter = TF_Default;')),
        address_x=dict(value='TA_Wrap',source='UTexture2D ctor does not assign AddressX; zero-initialized UObject member, TA_Wrap is enumerator 0 ('+line(header,'TA_Wrap UMETA')+')'),
        address_y=dict(value='TA_Wrap',source='same as address_x'),
        lod_group=dict(value='TEXTUREGROUP_World',source='UTexture ctor does not assign LODGroup; zero-initialized, TEXTUREGROUP_World is enumerator 0 ('+line(group,'TEXTUREGROUP_World UMETA')+')'),
        compression_settings=dict(value='TC_Default',source='UTexture ctor does not assign CompressionSettings; zero-initialized, TC_Default is enumerator 0 ('+line(header,'TC_Default\t')+')'))

def build():
    raw=BUNDLE2.read_bytes(); require(hashlib.sha1(raw).hexdigest()==BUNDLE2_SHA1,'bundle-2 changed')
    bundle=json.loads(raw); proof=prove_properties(); defaults=defaults_evidence()
    fixture=bundle['runtime_fixture']; contract=fixture['texture_contract']; texture=contract['textures'][0]
    require(contract['textures'].__len__()==1 and texture['effective_sampler'] is None and texture['effective_srgb'] is None,'bundle-2 texture contract changed')
    expression=fixture['texture_2d_parameters'][0]; require(expression['sampler_source']==0,'sampler source changed')
    texture['effective_sampler']=dict(sampler_source=0,sampler_source_mode='SSM_FromTextureAsset',
        note='ESamplerSourceMode value 0 is SSM_FromTextureAsset (Engine/Classes/Engine/EngineTypes.h), NOT a shared WorldGroupSettings sampler: the sampler is the texture asset sampler (wrap/wrap, filter from the World texture group)',
        address_x=defaults['address_x']['value'],address_y=defaults['address_y']['value'],filter=defaults['filter']['value'])
    texture['effective_srgb']=True
    texture['effective_properties']=dict(lod_group=defaults['lod_group']['value'],compression_settings=defaults['compression_settings']['value'],
        pixel_format='PF_DXT5',width=256,height=256,mip_count=9,evidence=dict(defaults,serialized=proof))
    texture['source_package']={k:dict(bytes=SOURCE_BYTES[k],sha256=SOURCE_SHA256[k]) for k in SOURCE_SHA256}
    texture['metadata_limit']='Effective SRGB/Filter/LODGroup/AddressX/AddressY/CompressionSettings are the UTexture2D class defaults, proven by the absence of those properties from the original serialized property list (evidence.serialized) and the UE 4.25 constructor sources (evidence).'
    contract['scope']='Original compressed mips, serialized properties and class-default effective sampling state'
    bundle['limitations']=['Offline source conversion only; runtime attachment, draw and original pixel parity unverified']
    bundle['provenance']['derived_from']=dict(path=str(BUNDLE2),sha1=BUNDLE2_SHA1,script=Path(__file__).name,script_sha256=sha256(Path(__file__).read_bytes()))
    return bundle

def main():
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument('--output',type=Path,default=GENERATED/'material_pipeline/tracking-llama-bundle-3'); args=parser.parse_args()
    require(GENERATED.resolve() in args.output.resolve().parents and not args.output.exists(),'choose fresh generated output directory')
    item=build(); args.output.mkdir(parents=True); path=args.output/'M_UI_TrackingLlama.json'
    raw=(json.dumps(item,indent=2,allow_nan=False)+'\n').encode(); path.write_bytes(raw)
    print(json.dumps(dict(path=str(path),bytes=len(raw),sha1=hashlib.sha1(raw).hexdigest(),sha256=sha256(raw),resource_sha1=item['provenance']['resource_sha1'])))
if __name__=='__main__': main()
