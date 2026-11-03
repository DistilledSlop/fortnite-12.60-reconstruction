#!/usr/bin/env python3
"""Check the original image, reflection, interface targets, and proposed source."""
import hashlib
import json
import struct
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / 'porting/behavior'))
import build_vtable_slots as vt

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
with IMAGE.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == SHA
    for window in json.loads((HERE / 'evidence.json').read_text()):
        f.seek(int(window['va'], 0) - vt.BASE)
        assert hashlib.sha256(f.read(window['size'])).hexdigest() == window['sha256'], window['reason']
im = vt.Image(str(IMAGE))
assert vt.load_gpsc(vt.GPSC_INDEX)['UFortItemDefinition'] == 0x14295c2b0
with tempfile.TemporaryDirectory() as t:
    assert vt.ctor_of(im, 0x14295c2b0, t) == 0x1420241d0
# Class registration, interface offsets and complete secondary dispatch entries.
assert im.u64(0x146416900 + 0x30) == 0x1464168e0
assert [im.u64(0x1464168e0 + 8*i) for i in range(4)] == [0x143e98250, 0x30, 0x142b10360, 0x38]
assert [im.u64(0x14606c390 + 8*i) for i in range(2, 6)] == [0x1420302f0, 0x143e915a0, 0x143e90ec0, 0x143e912e0]
assert im.m[0x142024209-vt.BASE:0x142024218-vt.BASE] == bytes.fromhex('488d05808104044c89734048894330')
assert im.m[0x1420302f0-vt.BASE:0x142030302-vt.BASE] == bytes.fromhex('488bc2488d91b8000000488bc8e9aebee501')
names = json.loads((ROOT / 'porting/generated/reflection/class_name_map.json').read_text())
assert names['0x146416900'] == 'UFortItemDefinition'
classes = json.loads((ROOT / 'porting/generated/reflection/fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
cls = next(c for c in classes if c['block_va'] == '0x146416900')
p = next(p for p in cls['properties'] if p['name'] == 'GameplayTags')
assert p['offset'] == 0xe8 and p['kind'] == 'Struct' and p['type_refs'] == {'struct': '0x143e9a300'}
assert not any('GetOwnedGameplayTags' in str(f) for f in cls['functions'])
assert hashlib.sha256((HERE / 'UFortItemDefinition_GetOwnedGameplayTags.inc').read_bytes()).hexdigest() == '5ae80f44d3b6d4fa65de4c1898673e45a8cca9dcf62bfc454ddb621ce814c8aa'
assert hashlib.sha256((HERE / 'manifest-entries.json').read_bytes()).hexdigest() == '9216d76a51a696d22d6ec4ddc0a2f640764f5736e08f30e9be54209330eb6efa'
entries = json.loads((HERE / 'manifest-entries.json').read_text())
assert len(entries) == 1 and entries[0]['signature'] == 'void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const'
print('Item gameplay-tag interface original evidence and proposal verified')
