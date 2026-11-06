#!/usr/bin/env python3
"""Preserve bounded original entry/JSON/GC evidence; no runtime tests."""
import hashlib, json, sys
from pathlib import Path
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[2]
sys.path.insert(0,str(ROOT/'porting/behavior'))
import build_vtable_slots as vt
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
with IMAGE.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()==SHA
im=vt.Image(str(IMAGE));rows=[]
for va in [0x1410ebe70,0x1410ebf10,0x141100f10,0x1410ee790,0x1411042a0,0x141104980,0x14110d9e0,0x142cf2c40,0x1410f1290]:
 lo,hi=im.func_of(va);raw=bytes(im.m[lo-vt.BASE:hi-vt.BASE]);rows.append(dict(target=hex(va),begin=hex(lo),end=hex(hi),sha256=hashlib.sha256(raw).hexdigest(),code=raw.hex()))
(HERE/'evidence.json').write_text(json.dumps(dict(image_sha256=SHA,functions=rows),indent=2)+'\n')
print('Preserved nine finite original evidence windows; no canonical credit')
