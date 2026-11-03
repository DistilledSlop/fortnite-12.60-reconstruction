#!/usr/bin/env python3
"""Finite item owner/lifetime recovery; verify the original image and emit proposals."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
spec = importlib.util.spec_from_file_location('proof', ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py')
proof = importlib.util.module_from_spec(spec)
spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == proof.SHA
im = vt.Image(str(proof.IMG))
evidence = json.loads((HERE/'evidence.json').read_text())
for addr, raw in evidence['code'].items():
    data = bytes.fromhex(raw)
    assert im.m[int(addr, 0)-vt.BASE:int(addr, 0)-vt.BASE+len(data)] == data
load = lambda p: json.loads((ROOT/p).read_text())
with tempfile.TemporaryDirectory() as scratch:
    for cls, gpsc, ctor, table in [
        ('UFortItem', 0x142951e90, 0x142024010, 0x14606c3f0),
        ('UFortWorldItem', 0x142b5bd80, 0x142041fc0, 0x14607a630),
    ]:
        assert vt.load_gpsc(vt.GPSC_INDEX)[cls] == gpsc
        assert vt.ctor_of(im, gpsc, scratch) == ctor
        assert vt.vptr_candidates(im, ctor, scratch) == [table]
        assert not vt.thunk_store_candidates(im, vt.LAST_THUNK[0], scratch)
        assert proof.proven_this_store(im, ctor, table, scratch)
        assert len(vt.table_entries(im, table)) > 0x4a0//8
assert im.u64(0x14606c3f0+0x4a0) == 0x142030a60
for table in [0x14607a630, 0x1463af658, 0x1464e8cd8]:
    for slot, target in [(0x150, 0x1410cf280), (0x460, 0x14205b110), (0x4a0, 0x14205b080)]:
        assert im.u64(table+slot) == target
names = load('porting/generated/reflection/class_name_map.json')
classes = load('porting/generated/reflection/fortnite_game_reflection.json')['classes_in_groups_with_fort_names']
blocks = {x['block_va']: x for x in classes}
block = lambda name: blocks[next(k for k, v in names.items() if v == name)]
prop = lambda cls, name: next(x for x in block(cls)['properties'] if x['name'] == name)
p = prop('UFortWorldItem', 'OwnerInventory')
assert p['kind'] == 'Object' and p['offset'] == 0xd8 and p['type_refs'] == {'class': '0x1429516f0'}
p = prop('AActor', 'Owner')
assert p['kind'] == 'Object' and p['offset'] == 0xe0 and p['type_refs'] == {'class': '0x1446ab9e0'}
b = next(x for x in load('porting/generated/reflection/bool_bits.json')['by_class']['UFortItem'] if x['name'] == 'bTemporaryItemOwningController')
assert b['offset'] == 0x80 and b['mask'] == 2
native = load('porting/generated/reflection/native_funcs.json')['UFortItem']
for name, addr, flags, pname, pflags in [
    ('GetOwningController', '0x142959560', '0x54020400', 'ReturnValue', 0x10000000000580),
    ('SetOwningControllerForTemporaryItem', '0x14295a650', '0x4020401', 'InController', 0x10000000000080),
]:
    fn = native[name]
    assert fn['exec_addr'] == addr and fn['flags'] == flags and len(fn['params']) == 1
    p = fn['params'][0]
    assert p['name'] == pname and p['kind'] == 'Object' and p['kind_raw'] == '0x12'
    assert p['type_refs'] == {'class': '0x142a35ea0'} and p['array_dim'] == 1 and p['offset'] == 0
    assert int(p['property_flags'], 0) == pflags
rows = [
    ('UFortItem', 'GetOwningController', 'exec', 'AFortPlayerController* GetOwningController() const', True, '0x142959560', '''AFortPlayerController* Controller = nullptr;
if (NativeOwningController.IsValid())
{
    Controller = static_cast<AFortPlayerController*>(NativeOwningController.Get());
}
return Controller;'''),
    ('UFortItem', 'SetOwningControllerForTemporaryItem', 'exec', 'void SetOwningControllerForTemporaryItem(AFortPlayerController* InController)', False, '0x14295a650', '''(void)NativeOwningController.IsValid();
bTemporaryItemOwningController = true;
AFortPlayerController* Controller = InController;
NativeOwningController = Controller;'''),
    ('UFortWorldItem', 'NativeGetOwnerInventory', 'helper', 'AFortInventory* NativeGetOwnerInventory() const', True, '0x14205b110', 'return OwnerInventory;'),
    ('UFortWorldItem', 'GetOwningController', 'helper', 'AFortPlayerController* GetOwningController() const', True, '0x14205b080', '''if (NativeOwningController.IsValid())
{
    return static_cast<AFortPlayerController*>(NativeOwningController.Get());
}
if (NativeGetOwnerInventory())
{
    AFortInventory* Inventory = NativeGetOwnerInventory();
    return Cast<AFortPlayerController>(Inventory->GetOwner());
}
return nullptr;'''),
    ('UFortWorldItem', 'GetWorld', 'helper', 'UWorld* GetWorld() const', True, '0x1410cf280', '''const UObject* Outer = GetOuter();
return Outer ? Outer->GetWorld() : nullptr;'''),
]
out = []
manifest = load('porting/behavior/MANIFEST.json')
used = {(x.get('class'), x.get('function')) for x in manifest}
for cls, name, kind, signature, virtual, addr, body in rows:
    assert (cls, name) not in used
    inc = HERE/(cls+'_'+name+'.inc')
    inc.write_text('// Original item owner/lifetime behavior; see HANDOFF.md.\n// CONFIDENCE: HIGH\n'+body+'\n')
    row = dict(function=name, kind=kind, signature=signature, signature_status='proposed', access='public',
               inc_path=str(inc.relative_to(ROOT)), address=addr, confidence='high', status='ported')
    row['class'] = cls
    if virtual:
        row['virtual'] = True
    if name == 'GetWorld':
        row['includes'] = ['Engine/World.h']
    if kind == 'exec':
        row['reflection_flags'] = native[name]['flags']
    out.append(row)
assert ('UFortItem', 'NativeOwningController') not in used
out.append({'class': 'UFortItem', 'function': 'NativeOwningController', 'kind': 'member_field',
            'signature': 'FWeakObjectPtr NativeOwningController', 'signature_status': 'proposed',
            'access': 'protected', 'includes': ['UObject/WeakObjectPtr.h'], 'offset': '0x78', 'confidence': 'high', 'status': 'ported',
            'notes': 'Assigned name for original FWeakObjectPtr storage; original ctor initializes INDEX_NONE/0, matching UE default weak construction. Read/written by recovered controller methods.'})
(HERE/'manifest-entries.json').write_text(json.dumps(out, indent=2)+'\n')
print('5 item owner/lifetime bodies + 1 supporting weak field')
