#!/usr/bin/env python3
"""Finite WorldItem virtual overrides; verify exact shipping-image and storage evidence."""
import hashlib, importlib.util, json, re, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py'); proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()==proof.SHA
im=vt.Image(str(proof.IMG))
with tempfile.TemporaryDirectory() as s:
 gpsc=vt.load_gpsc(vt.GPSC_INDEX)['UFortWorldItem']; ctor=vt.ctor_of(im,gpsc,s); thunk=vt.LAST_THUNK[0]; tables=vt.vptr_candidates(im,ctor,s)
 assert (gpsc,thunk,ctor,tables)==(0x142b5bd80,0x142b59740,0x142041fc0,[0x14607a630])
 assert not vt.thunk_store_candidates(im,thunk,s) and proof.proven_this_store(im,ctor,tables[0],s)
 assert len(vt.table_entries(im,tables[0]))>0x3e8//8
checks={0x142059c00:'40534883ec20488b01488bd9ff90c002000084c07411488d8be00000004883c4205be91937fdfff30f10056dfb94034883c4205bc3',0x14205ca30:'40534883ec20488b01ff90e8030000488bd84885c07431e8f4f3af00488b4b104883c030486350083b51387f1b488b4930483904d17511f68343030000017408b0014883c4205bc332c04883c4205bc3',0x14205af00:'48895c2408574883ec20488b01488bf9ff90e8030000488bd84885c07432e81d0fb000488b4b104883c030486350083b51387f1c488b4930483904d175120fb68334070000488b5c24304883c4205fc3488bcf488b5c24304883c4205fe96e5262fe',0x14205ac70:'4881c1e0000000e9e416fdff',0x14205a2f0:'4881c1e0000000e9247b76fe',0x14202c360:'8b410cc3',0x1407c1e20:'488b4118c3',0x14202d340:'f30f104124c3',0x1406801d0:'b001c3'}
for va,hexbytes in checks.items():
 raw=bytes.fromhex(hexbytes); assert im.m[va-vt.BASE:va-vt.BASE+len(raw)]==raw
const=0x1459a979c
sections=[(n,ch) for n,start,size,ch in im.secs if start<=const-vt.BASE and const-vt.BASE+4<=start+size]
assert len(sections)==1 and sections[0][1]&0x40 and sections[0][1]&0x40000000 and not sections[0][1]&0x80000000
import struct
assert struct.unpack_from('<f',im.m,const-vt.BASE)[0]==-1.0
load=lambda p:json.loads((ROOT/p).read_text())
name=load('porting/generated/reflection/class_name_map.json'); refl=load('porting/generated/reflection/fortnite_game_reflection.json'); blocks={x['block_va']:x for x in refl['classes_in_groups_with_fort_names']}
block=lambda cls:blocks[next(v for v,n in name.items() if n==cls)]
prop=lambda cls,n:next(x for x in block(cls)['properties'] if x['name']==n)
item=prop('UFortWorldItem','ItemEntry'); assert item['offset']==0xe0 and item['kind']=='Struct' and item['type_refs']=={'struct':'0x14295cb30'}
assert prop('UFortWorldItemDefinition','NumberOfSlotsToTake')['offset']==0x734
assert next(x for x in load('porting/generated/reflection/bool_bits.json')['by_class']['UFortWorldItemDefinition'] if x['name']=='bItemHasDurability')['offset']==0x343
assert next(x for x in load('porting/generated/reflection/bool_bits.json')['by_class']['UFortWorldItemDefinition'] if x['name']=='bItemHasDurability')['mask']==1
assert vt.load_gpsc(vt.GPSC_INDEX)['UFortWorldItemDefinition']==0x142b5be40
assert im.m[0x14657c3fa-vt.BASE:0x14657c3fa-vt.BASE+len('FortWorldItemDefinition'.encode('utf-16le'))+2]=='FortWorldItemDefinition\0'.encode('utf-16le')
gpsc_body=im.func_of(0x142b5be40); assert gpsc_body==(0x142b5be40,0x142b5bef2)
assert hashlib.sha256(im.m[gpsc_body[0]-vt.BASE:gpsc_body[1]-vt.BASE]).hexdigest()=='f27b3f65838f4e6c30adb078bcc86c95d3b9771e50da7c1fb37d25eec9d62373'
# Assert reflected struct layout and exact generated representation.
structs=load('porting/generated/reflection/struct_blocks.json')
entry=next(x for x in structs['structs'] if '0x14295cb30' in x['construct_fns'])
assert entry['name']=='FortItemEntry'
for n,k,o in [('Count','Int',12),('ItemDefinition','Object',24),('Durability','Float',36)]:
 field=next(x for x in entry['properties'] if x['name']==n); assert field['kind']==k and field['offset']==o and field['array_dim']==1
 if n=='ItemDefinition': assert field['type_refs']=={'class':'0x14295b4c0'}
# Assert generated representation as well as original reflected offsets.
hdr=(ROOT/'porting/native/Source/FortniteGame/Generated/FFortItemEntry.h').read_text()
for text in ('offset 12, flags','int32 Count;', 'offset 24, flags','UFortItemDefinition* ItemDefinition;', 'offset 36, flags','float Durability;'): assert text in hdr
worldhdr=(ROOT/'porting/native/Source/FortniteGame/Generated/UFortWorldItem.h').read_text()
defhdr=(ROOT/'porting/native/Source/FortniteGame/Generated/UFortWorldItemDefinition.h').read_text()
for text in ('FFortItemEntry ItemEntry;',): assert text in worldhdr
for text in ('uint8 bItemHasDurability : 1;','uint8 NumberOfSlotsToTake;'):
 assert text in defhdr and defhdr.rfind('public:',0,defhdr.index(text))>max(defhdr.rfind('private:',0,defhdr.index(text)),defhdr.rfind('protected:',0,defhdr.index(text)))
rows=[('GetDurability','float',0x2c8,0x142059c00,'return HasDurability() ? ItemEntry.Durability : -1.0f;',[]),('HasDurability','bool',0x2c0,0x14205ca30,'const UFortWorldItemDefinition* Definition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());\nreturn Definition && Definition->bItemHasDurability;',['UFortWorldItemDefinition.h']),('GetNumInStack','int32',0x368,0x14205ac70,'return ItemEntry.Count;',[]),('GetNumberOfSlotsTaken','uint8',0x2e8,0x14205af00,'const UFortWorldItemDefinition* Definition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());\nreturn Definition ? Definition->NumberOfSlotsToTake : 1;',['UFortWorldItemDefinition.h']),('GetItemDefinitionBP','UFortItemDefinition*',0x3e8,0x14205a2f0,'return ItemEntry.ItemDefinition;',[])]
manifest=load('porting/behavior/MANIFEST.json')
ported={(x.get('class'),x.get('function')) for x in manifest if x.get('status')=='ported'}
source=ROOT/'porting/native/Source'; maintained=set()
for cpp in source.rglob('*.cpp'):
 if 'Generated' not in cpp.parts: maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace')))
cpp=(source/'FortniteGame/Generated/UFortWorldItem.cpp').read_text(); out=[]
for n,typ,slot,va,body,includes in rows:
 assert im.u64(tables[0]+slot)==va
 base=[x for x in manifest if x.get('class')=='UFortItem' and x.get('function')==n and x.get('status')=='ported']
 assert len(base)==1 and base[0].get('virtual') is True and base[0]['signature']==f'{typ} {n}() const'
 assert ('UFortWorldItem',n) not in ported and ('UFortWorldItem',n) not in maintained
 assert not re.search(r'\b'+re.escape(n)+r'\s*\(',worldhdr) and not re.search(r'\bUFortWorldItem::'+re.escape(n)+r'\s*\(',cpp)
 inc=HERE/(n+'.inc'); inc.write_text('// WorldItem virtual override '+hex(va)+'.\n// CONFIDENCE: HIGH\n'+body+'\n')
 row={'class':'UFortWorldItem','function':n,'kind':'helper','signature':f'{typ} {n}() const','signature_status':'proposed','access':'public','virtual':True,'inc_path':str(inc.relative_to(ROOT)),'address':hex(va),'confidence':'high','status':'ported','notes':f'WorldItem constructor {hex(ctor)} primary vtable {hex(tables[0])} slot {hex(slot)}; exact body and reflected storage in world_item_queries_runtime/HANDOFF.md.'}
 # Same-module generated class includes are resolved by generator body_refs.
 out.append(row)
(HERE/'manifest-entries.json').write_text(json.dumps(out,indent=2)+'\n'); print(f'{len(out)} proven overrides')
