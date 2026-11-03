#!/usr/bin/env python3
"""Fail-closed proof for the three inventory lookup proposals; no canonical writes."""
import hashlib, json, re, struct, sys, tempfile, importlib.util
from pathlib import Path
R = Path(__file__).resolve().parents[3]
H = Path(__file__).resolve().parent
sys.path[:0] = ['/tmp/common-input-capstone', str(R/'porting/behavior')]
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof', R/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py')
proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()==proof.SHA=='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
im=vt.Image(str(proof.IMG)); b=im.m; base=vt.BASE
load=lambda p: json.loads((R/p).read_text())
for a,raw in [(0x1420065a0,'48895c240848896c2410488974241848897c242041564883ec20410fb6e84c8bf2488bf14885d2746033db3999980300007e568bfb6666660f1f840000000000488b8690030000488b0c074885c9742b488b01ff90e8030000493bc6751d4084ed7543488b8690030000488b0c07488b01ff904804000084c0742bffc34883c7083b9e980300007cb733c0488b5c2430488b6c2438488b742440488b7c24484883c420415ec3488b86900300008bcb488b04c8ebd6'),(0x1406801e0,'32c0c3'),(0x142074560,'0fb68145010000c3'),(0x140e8d4e0,'c6416501c3'),(0x1428cb480,'40534883ec20488b194885db7470488bd1488bcbe8276b77ff488d05b841ae03c78360020000ffffffff488903'),(0x142aa06e0,'40534883ec20488b194885db7451488bd1488bcbe8c7185aff488d05d885a403c6835802000000488903')]:
 raw=bytes.fromhex(raw); assert b[a-base:a-base+len(raw)]==raw
parents=load('porting/generated/reflection/class_parent_map.json')
assert {x['name'] for x in parents.values() if 'UFortWorldItem' in x['chain'] and x['chain_ok']}=={'UFortWorldItem','UFortCreativeWorldItem','UFortPlaysetWorldItem'}
assert not [x for x in parents.values() if 'UFortWorldItem' in x['chain'] and not x['chain_ok']]
gpsc=vt.load_gpsc(vt.GPSC_INDEX)
expected={'UFortWorldItem':(0x142b5bd80,0x142b59740,0x14607a630),'UFortCreativeWorldItem':(0x1428c9f90,0x1428cb480,0x1463af658),'UFortPlaysetWorldItem':(0x142a9fe20,0x142aa06e0,0x1464e8cd8)}
with tempfile.TemporaryDirectory() as t:
 assert gpsc['UFortItem']==0x142951e90 and vt.ctor_of(im,gpsc['UFortItem'],t)==0x142024010 and vt.LAST_THUNK[0]==0x1429551e0
 assert vt.vptr_candidates(im,0x142024010,t)==[0x14606c3f0] and proof.proven_this_store(im,0x142024010,0x14606c3f0,t)
 assert im.u64(0x14606c3f0+0x448)==0x1406801e0
 for cls,(g,thunk,table) in expected.items():
  assert gpsc[cls]==g and vt.ctor_of(im,g,t)==0x142041fc0 and vt.LAST_THUNK[0]==thunk
  assert vt.vptr_candidates(im,0x142041fc0,t)==[0x14607a630] and proof.proven_this_store(im,0x142041fc0,0x14607a630,t)
  assert vt.thunk_store_candidates(im,thunk,t)==([] if cls=='UFortWorldItem' else [table])
  assert im.u64(table+0x448)==0x142074560 and im.u64(table+0x3e8)==0x14205a2f0
ref=load('porting/generated/reflection/fortnite_game_reflection.json')
classes=ref['classes_in_groups_with_fort_names']
# Struct and class records identify original offsets; generated declarations establish source members.
inv=next(x for x in classes if x.get('block_va')=='0x146412c20')
p=next(x for x in inv['properties'] if x['name']=='Inventory' and x['kind']=='Struct')
assert p['offset']==0x228 and p['type_refs']=={'struct':'0x1429583d0'}
structs=load('porting/generated/reflection/struct_blocks.json')['structs']
lst=next(x for x in structs if x['name']=='FortItemList')
q=next(x for x in lst['properties'] if x['name']=='ItemInstances' and x['kind']=='Array')
assert q['offset']==0x168 and p['offset']+q['offset']==0x390
entry=next(x for x in structs if x['name']=='FortItemEntry'); assert entry['block_va']=='0x147a933d0' and entry['size']==336
q=next(x for x in entry['properties'] if x['name']=='bWasGifted')
assert q['index']==13 and q['kind']=='Bool' and q['kind_raw']=='0x2c' and q['array_dim']==1 and q['element_size']==1 and q['size_of_outer']==336 and q['bitfield'] and q['offset'] is None and not q['replicated'] and q['rep_notify'] is None
sv=int(entry['block_va'],0)-base; arr=struct.unpack_from('<Q',b,sv+48)[0]; count=struct.unpack_from('<i',b,sv+56)[0]
assert count==len(entry['properties'])==27 and arr==0x1464169b0
pp=struct.unpack_from('<Q',b,arr-base+8*q['index'])[0]; assert pp==0x147a93170
n,rep,pflags,gflags,oflags,adim=struct.unpack_from('<QQQIII',b,pp-base)
assert bytes(b[n-base:n-base+11])==b'bWasGifted\0' and rep==0 and pflags==int(q['property_flags'],0) and gflags==0x2c and oflags==int(q['object_flags'],0) and adim==1
assert struct.unpack_from('<IQ',b,pp-base+36)==(1,336) and struct.unpack_from('<Q',b,pp-base+48)[0]==0x140e8d4e0
src=R/'porting/native/Source/FortniteGame/Generated'
assert re.search(r'\bFFortItemList\s+Inventory\s*;', (src/'AFortInventory.h').read_text())
assert re.search(r'TArray\s*<\s*UFortWorldItem\s*\*\s*>\s+ItemInstances\s*;', (src/'FFortItemList.h').read_text())
assert re.search(r'\bbool\s+bWasGifted\s*;', (src/'FFortItemEntry.h').read_text())
assert re.search(r'\bFFortItemEntry\s+ItemEntry\s*;', (src/'UFortWorldItem.h').read_text())
fn=load('porting/generated/reflection/native_funcs.json')['UFortItem']['GetItemDefinitionBP']
assert fn['exec_addr']=='0x142959140' and fn['flags']=='0x54020401' and len(fn['params'])==1
ret=fn['params'][0]; assert ret['name']=='ReturnValue' and ret['kind']=='Object' and ret['type_refs']=={'class':'0x14295b4c0'} and ret['offset']==0
assert re.search(r'virtual\s+UFortItemDefinition\s*\*\s*GetItemDefinitionBP\(\)\s+const\s*;', (src/'UFortItem.h').read_text())
manifest=load('porting/behavior/MANIFEST.json'); proposals=json.loads((H/'manifest-entries.json').read_text())
assert [(x['class'],x['function'],x['address']) for x in proposals]==[('UFortItem','NativeWasGifted','0x1406801e0'),('UFortWorldItem','NativeWasGifted','0x142074560'),('AFortInventory','NativeFindWorldItemByDefinition','0x1420065a0')]
hashes=['b71901b16868017631707c4dd479fa410bf8cb422a360b45cef466ced22db341','1c933c02cbe254dd0e82a9b77f0a4bc3ba479c9961bb3a93fd7186c1c7b9a3db','2466e1fb79cc11a804415c5de179503f6ae3982e2c85b42eb662e64fa19654b0']
for x,expected_hash in zip(proposals,hashes):
 assert hashlib.sha256((R/x['inc_path']).read_bytes()).hexdigest()==expected_hash
 assert not any(e.get('class')==x['class'] and e.get('function')==x['function'] for e in manifest)
 for ext in ('h','cpp'):
  assert not re.search(r'\b'+re.escape(x['function'])+r'\s*\(', (src/(x['class']+'.'+ext)).read_text())
 assert (R/x['inc_path']).is_file()
print('Three inventory lookup proposals passed pinned evidence gate')
