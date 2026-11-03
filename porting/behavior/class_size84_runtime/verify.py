#!/usr/bin/env python3
"""Verify three GetPrivateStaticClassBody size arguments from a pinned runtime image."""
import json
import struct
from pathlib import Path

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
HELPER = 0x142d698b0
# name, FClassParams, no-register function, builder, name literal, size-store, call, size, alignment
ROWS = [
 ('UFortGenericRecord', 0x1463f6140, 0x14287cce0, 0x14292e780, 0x1463f73a0, 0x14292e80f, 0x14292e817, 0xf0, 8),
 ('AFortPhysicsPawn', 0x14648e670, 0x1428394a0, 0x142a25dd0, 0x14648f8e8, 0x142a25e66, 0x142a25e6e, 0x380, 16),
 ('AFortGameState_InGame', 0x1463f1e00, 0x142926770, 0x142926770, 0x1463f3558, 0x142926806, 0x14292680e, 0x6b8, 8),
]
reflection = json.loads(Path('porting/generated/reflection/fortnite_game_reflection.json').read_text())
evidence = json.loads(Path('porting/generated/reflection/class_name_map.evidence.json').read_text())
blocks = {int(x['block_va'], 16): x for x in reflection['classes_in_groups_with_fort_names']}
names = {int(x['block_va'],16): x for x in evidence['blocks']}
with IMAGE.open('rb') as image:
 def read(va, length):
  image.seek(va - BASE)
  return image.read(length)
 for name, block, nr, builder, literal, store, call, size, align in ROWS:
  assert int(blocks[block]['class_noregister_fn'],16) == nr
  assert name in names[block]['candidates']
  assert read(literal, len(name.encode('utf-16le')) + 2) == name.encode('utf-16le') + b'\0\0'
  if nr != builder:
   b = read(nr, 5)
   assert b[0] == 0xe9 and nr + 5 + struct.unpack('<i', b[1:])[0] == builder
  # Literal address is loaded into RDX, and drops the conventional A/U prefix.
  body = read(builder, call + 5 - builder)
  lea = body.find(b'\x48\x8d\x15')
  assert lea >= 0 and builder + lea + 7 + struct.unpack('<i', body[lea+3:lea+7])[0] == literal + 2
  assert read(store-8, 8) == b'\xc7\x44\x24\x28' + struct.pack('<I', align)
  assert read(store, 8) == b'\xc7\x44\x24\x20' + struct.pack('<I', size)
  b = read(call, 5)
  assert b[0] == 0xe8 and call + 5 + struct.unpack('<i', b[1:])[0] == HELPER
  print(f'{name}: builder={builder:#x} call={call:#x} size={size:#x} align={align:#x}')
