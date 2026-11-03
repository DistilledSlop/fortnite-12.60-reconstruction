#!/usr/bin/env python3
"""Prove the WorldItem loaded-ammo override needed by GetRemainingAmmo."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py'); proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
img=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
with img.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()=='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
im=vt.Image(str(img))
with tempfile.TemporaryDirectory() as d:
 gpsc=vt.load_gpsc(vt.GPSC_INDEX)['UFortWorldItem']; ctor=vt.ctor_of(im,gpsc,d); tables=vt.vptr_candidates(im,ctor,d)
 assert (gpsc,ctor,tables)==(0x142b5bd80,0x142041fc0,[0x14607a630])
 assert vt.LAST_THUNK[0]==0x142b59740 and not vt.thunk_store_candidates(im,vt.LAST_THUNK[0],d) and proof.proven_this_store(im,ctor,tables[0],d)
 assert im.u64(tables[0]+0x378)==0x14205a600 and im.u64(tables[0]+0x380)==0x14205b6b0
for a,s in [(0x14205a600,'4881c1e0000000e9145c1aff'),(0x141200220,'8b412cc3')]:
 raw=bytes.fromhex(s); assert im.m[a-vt.BASE:a-vt.BASE+len(raw)]==raw
load=lambda p:json.loads((ROOT/p).read_text())
names=load('porting/generated/reflection/class_name_map.json'); refl=load('porting/generated/reflection/fortnite_game_reflection.json'); blocks={x['block_va']:x for x in refl['classes_in_groups_with_fort_names']}
b=blocks[next(k for k,v in names.items() if v=='UFortWorldItem')]
p=next(x for x in b['properties'] if x['name']=='ItemEntry'); assert p['offset']==0xe0 and p['type_refs']=={'struct':'0x14295cb30'}
s=next(x for x in load('porting/generated/reflection/struct_blocks.json')['structs'] if '0x14295cb30' in x['construct_fns']); p=next(x for x in s['properties'] if x['name']=='LoadedAmmo'); assert p['offset']==44 and p['kind']=='Int' and p['array_dim']==1
assert 'int32 LoadedAmmo;' in (ROOT/'porting/native/Source/FortniteGame/Generated/FFortItemEntry.h').read_text()
rows=load('porting/behavior/MANIFEST.json'); assert any(x.get('class')=='UFortItem' and x.get('function')=='GetLoadedAmmo' and x.get('virtual') and x.get('signature')=='int32 GetLoadedAmmo() const' and x.get('status')=='ported' for x in rows)
assert not any(x.get('class')=='UFortWorldItem' and x.get('function')=='GetLoadedAmmo' and x.get('status')=='ported' for x in rows)
(HERE/'GetLoadedAmmo.inc').write_text('// WorldItem virtual override 0x14205a600.\n// CONFIDENCE: HIGH\nreturn ItemEntry.LoadedAmmo;\n')
row={'class':'UFortWorldItem','function':'GetLoadedAmmo','kind':'helper','signature':'int32 GetLoadedAmmo() const','signature_status':'proposed','access':'public','virtual':True,'inc_path':'porting/behavior/world_item_ammo_runtime/GetLoadedAmmo.inc','address':'0x14205a600','confidence':'high','status':'ported','notes':'WorldItem vtable slot 0x378 returns ItemEntry.LoadedAmmo; required by GetRemainingAmmo. See world_item_ammo_runtime/HANDOFF.md.'}
(HERE/'manifest-entries.json').write_text(json.dumps([row],indent=2)+'\n')
print('1 proven override')
