#!/usr/bin/env python3
"""Replay bounded original Fortnite GI Init/module proof without Ghidra or execution."""
import argparse, hashlib, json, struct
from pathlib import Path
BASE = 0x140000000
SHA = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'
def inspect(image, proof):
    data = image.read_bytes()
    assert hashlib.sha256(data).hexdigest() == SHA, 'original memory image SHA mismatch'
    for w in proof['windows']:
        a = int(w['va'], 16) - BASE
        actual = data[a:a+w['size']]
        assert hashlib.sha256(actual).hexdigest() == w['sha256'], w['va']
        assert actual.hex() == w['bytes'], w['va']
    for a, target, kind in proof['direct_edges']:
        a, target = int(a,16), int(target,16)
        offset = a-BASE
        op = data[offset]
        if op in (0xe8,0xe9):
            resolved = a+5+struct.unpack_from('<i', data, offset+1)[0]
        elif op == 0xeb:
            resolved = a+2+struct.unpack_from('<b', data, offset+1)[0]
        else:
            raise AssertionError('unsupported direct edge opcode '+hex(a))
        assert resolved == target, hex(a)
    slots = [struct.unpack_from('<Q',data,0x6d71770+i*8)[0] for i in range(8)]
    assert slots == [0x144da17e0,0x144da0800,0x144da0aa0,0x144da09b0,0x144da0be0,0x1406801d0,0x1406801d0,0x1406801d0]
    assert data[0x987c5b1:0x987c5b1+16].hex() == '969d113bd565089b050dabdae5d15bb2'
    return {'passed':True, 'image_sha256':SHA, 'windows_verified':len(proof['windows']),
            'direct_edges_verified':len(proof['direct_edges']),
            'init_complete':False, 'frontier':'0x14987c5b1',
            'module':'FortniteUI', 'initialize_ui_module_hook':'PostLoadCallback',
            'runtime_claim':False}
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--image',type=Path,required=True)
    ap.add_argument('--proof',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    assert not args.output.exists(), 'output must be new'
    result=inspect(args.image,json.loads(args.proof.read_text()))
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result))
if __name__=='__main__': main()
