#!/usr/bin/env python3
"""Gate only the recovered base key implementation; not a promotion gate."""
import hashlib, importlib.util, json, sys, tempfile, struct
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT/'porting/behavior')]
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86 import X86_REG_RDX, X86_REG_RIP, X86_OP_MEM
import build_vtable_slots as vt
spec = importlib.util.spec_from_file_location('proof', ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py')
proof = importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f: assert hashlib.file_digest(f, 'sha256').hexdigest() == proof.SHA
im = vt.Image(str(proof.IMG))
for a, raw in json.loads((HERE/'evidence.json').read_text()).items():
    b = bytes.fromhex(raw); assert im.m[int(a, 0)-vt.BASE:int(a, 0)-vt.BASE+len(b)] == b
with tempfile.TemporaryDirectory() as t:
    for cls, gpsc, ctor, table in [('UFortItemDefinition',0x14295c2b0,0x1420241d0,0x14606bf00),('UFortWorldItemDefinition',0x142b5be40,0x142042090,0x14606f8f8)]:
        assert vt.load_gpsc(vt.GPSC_INDEX)[cls] == gpsc
        assert vt.ctor_of(im, gpsc, t) == ctor
        assert vt.vptr_candidates(im, ctor, t) == [table]
        assert vt.thunk_store_candidates(im, vt.LAST_THUNK[0], t) == ([table] if cls == 'UFortItemDefinition' else [])
        assert proof.proven_this_store(im, ctor, table, t)
        assert im.u64(table+0x3e0) == 0x142030d00
names = json.loads((ROOT/'porting/generated/reflection/class_name_map.json').read_text())
classes = json.loads((ROOT/'porting/generated/reflection/fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
block = next(k for k,v in names.items() if v == 'UFortItemDefinition')
cls = next(c for c in classes if c['block_va'] == block)
p = next(p for p in cls['properties'] if p['name'] == 'ItemType')
assert p['offset'] == 0x51 and p['kind'] == 'Enum' and p['type_refs'] == {'enum':'0x142a246a0'}
# The helper's complete switch and static initializers bind each input byte to
# the exact UTF-16 literal; 88 and 110 deliberately select WorldItem.
a, end = 0x141f23770, 0x141f28af4
assert hashlib.sha256(im.m[a-vt.BASE:end-vt.BASE]).hexdigest() == '63e56fd2aef7df4d32db8a125344cdc99f16e4bc5c9f995d2337159952587690'
md = Cs(CS_ARCH_X86, CS_MODE_64); md.detail = True
rows = json.loads((HERE/'type-names.json').read_text())
assert len(rows) == 112
for n, row in enumerate(rows):
    assert row['value'] == n
    o = 0x141f28934-vt.BASE+n*4
    target = vt.BASE + struct.unpack('<I', im.m[o:o+4])[0]
    ins = list(md.disasm(im.m[target-vt.BASE:target-vt.BASE+120], target))
    slow = next(i.operands[0].imm for i in ins if i.mnemonic == 'jg')
    ins = list(md.disasm(im.m[slow-vt.BASE:slow-vt.BASE+160], slow))
    lea = next(i for i in ins if i.mnemonic == 'lea' and i.operands[0].reg == X86_REG_RDX and i.operands[1].type == X86_OP_MEM and i.operands[1].mem.base == X86_REG_RIP)
    literal = lea.address + lea.size + lea.operands[1].mem.disp
    assert literal == int(row['literal'], 0)
    data = (row['text'] + '\0').encode('utf-16le')
    assert im.m[literal-vt.BASE:literal-vt.BASE+len(data)] == data
assert bytes.fromhex(json.loads((HERE/'evidence.json').read_text())['0x145a95870']) == ':\0'.encode('utf-16le')
print('Definition key base implementation and type-name literal map verified')

# Pin reviewed source and proposal; original evidence remains independently checked above.
assert hashlib.sha256((HERE/'UFortItemDefinition_NativeGetProfileKey.inc').read_bytes()).hexdigest() == '0f7bec06dec59f936df34aa32974f1179f56aab7585bb5ac40bde048da537224'
assert hashlib.sha256((HERE/'manifest-entries.json').read_bytes()).hexdigest() == 'da7444425c86e101696a095042b29217ff38be4a2fb45bb9003e1f81119edf4d'
