#!/usr/bin/env python3
"""Prove and emit only the WorldItem GetLevel override; no normalized proposal."""
import hashlib, importlib.util, json, re, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py'); proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()==proof.SHA
im=vt.Image(str(proof.IMG))
with tempfile.TemporaryDirectory() as s:
 for cls,gpsc,thunk,ctor,table in [('UFortItem',0x142951e90,0x1429551e0,0x142024010,0x14606c3f0),('UFortWorldItem',0x142b5bd80,0x142b59740,0x142041fc0,0x14607a630)]:
  assert vt.load_gpsc(vt.GPSC_INDEX)[cls]==gpsc
  assert vt.ctor_of(im,gpsc,s)==ctor and vt.LAST_THUNK[0]==thunk
  assert vt.vptr_candidates(im,ctor,s)==[table] and not vt.thunk_store_candidates(im,thunk,s)
  assert proof.proven_this_store(im,ctor,table,s) and len(vt.table_entries(im,table))>0x268//8
assert im.u64(0x14606c3f0+0x268)==0x140680300
assert im.u64(0x14607a630+0x268)==0x14205a560
for addr,raw in [(0x142959330,'40534883ec20488b42204533c94885c0498bd8410f95c14c03c84c894a20488b01ff906802000089034883c4205bc3'),(0x140680300,'33c0c3'),(0x14205a560,'4881c1e0000000e9a46622ff'),(0x141280c10,'8b4128c3')]:
 b=bytes.fromhex(raw); assert im.m[addr-vt.BASE:addr-vt.BASE+len(b)]==b
load=lambda p:json.loads((ROOT/p).read_text())
f=load('porting/generated/reflection/native_funcs.json')['UFortItem']['GetLevel']
assert f['exec_addr']=='0x142959330' and f['flags']=='0x54020400' and len(f['params'])==1
p=f['params'][0]; assert p['name']=='ReturnValue' and p['kind']=='Int' and p['array_dim']==1 and int(p['property_flags'],0)==0x10000000000580 and p['kind_raw']=='0x3' and p['offset']==0 and not p.get('type_refs')
names=load('porting/generated/reflection/class_name_map.json'); refl=load('porting/generated/reflection/fortnite_game_reflection.json'); blocks={x['block_va']:x for x in refl['classes_in_groups_with_fort_names']}
world=blocks[next(v for v,n in names.items() if n=='UFortWorldItem')]; item=next(x for x in world['properties'] if x['name']=='ItemEntry'); assert item['offset']==0xe0 and item['kind']=='Struct' and item['type_refs']=={'struct':'0x14295cb30'}
entry=next(x for x in load('porting/generated/reflection/struct_blocks.json')['structs'] if '0x14295cb30' in x['construct_fns']); assert entry['name']=='FortItemEntry'
level=next(x for x in entry['properties'] if x['name']=='Level'); assert level['kind']=='Int' and level['offset']==0x28 and level['array_dim']==1 and level['kind_raw']=='0x3' and not level.get('type_refs')
base=(ROOT/'porting/native/Source/FortniteGame/Generated/UFortItem.h').read_text(); wh=(ROOT/'porting/native/Source/FortniteGame/Generated/UFortWorldItem.h').read_text(); eh=(ROOT/'porting/native/Source/FortniteGame/Generated/FFortItemEntry.h').read_text()
assert 'virtual int32 GetLevel() const;' in base and 'FFortItemEntry ItemEntry;' in wh and 'offset 40, flags' in eh and 'int32 Level;' in eh
manifest=load('porting/behavior/MANIFEST.json'); b=[x for x in manifest if x.get('class')=='UFortItem' and x.get('function')=='GetLevel' and x.get('status')=='ported']; assert len(b)==1 and b[0]['signature']=='int32 GetLevel() const' and b[0].get('virtual') is True and b[0]['address']=='0x142959330' and b[0]['native_body_address']=='0x140680300'
assert not any(x.get('class')=='UFortWorldItem' and x.get('function')=='GetLevel' and x.get('status')=='ported' for x in manifest)
assert not re.search(r'\bGetLevel\s*\(',wh)
source=ROOT/'porting/native/Source'
for cpp in source.rglob('*.cpp'):
 assert not re.search(r'\bUFortWorldItem::GetLevel\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace'))
inc=HERE/'GetLevel.inc'; inc.write_text('// WorldItem virtual override 0x14205a560.\n// CONFIDENCE: HIGH\nreturn ItemEntry.Level;\n')
row={'class':'UFortWorldItem','function':'GetLevel','kind':'helper','signature':'int32 GetLevel() const','signature_status':'proposed','access':'public','virtual':True,'inc_path':str(inc.relative_to(ROOT)),'address':'0x14205a560','confidence':'high','status':'ported','notes':'WorldItem constructor 0x142041fc0 primary vtable 0x14607a630 slot 0x268; exact thunk, leaf, reflected storage, and existing base virtual verified in world_item_normalized_runtime/prepare_getlevel.py.'}
(HERE/'manifest-entries.json').write_text(json.dumps([row],indent=2)+'\n'); print('1 proven GetLevel override')
