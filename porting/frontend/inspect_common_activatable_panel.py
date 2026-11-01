#!/usr/bin/env python3
"""Snapshot bounded original CommonUI event evidence; memory-layout input only."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
EXPECTED = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'
BASE = 0x140000000


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', required=True, type=Path)
    ap.add_argument('--last-batch', type=int, default=13)
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    dump = ROOT / 'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin'
    assert sha(dump) == EXPECTED, 'wrong original memory image'
    spec = importlib.util.spec_from_file_location('reflection', ROOT / 'porting/extract_reflection.py')
    reflection = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(reflection)

    class MemoryImage(reflection.Image):
        def off(self, va):
            offset = va - BASE if va else -1
            return offset if 0 <= offset < len(self.m) else None

    image = MemoryImage(dump)
    assert image.imagebase == BASE
    data = image.m
    records = {}
    for name, address in [('Panel', 0x145a00360), ('Widget', 0x145a004e0),
                          ('CommonUISubsystem', 0x145a0b9c0),
                          ('CommonInputManager', 0x145a05fd0),
                          ('CommonInputSubsystem', 0x145a0e0d0),
                          ('CommonGlobalInputHandler', 0x145a058e0)]:
        offset = address - BASE
        prop_array = struct.unpack_from('<Q', data, offset + 40)[0]
        count = struct.unpack_from('<i', data, offset + 64)[0]
        records[name] = {'class_block': hex(address), 'properties': reflection.decode_props(image, prop_array, count)}
    for name, address in [('Panel', 0x1459edb10), ('Widget', 0x1459ed638)]:
        slots = [0x270, 0x290, 0x488, 0x490, 0x4a0, 0x4a8]
        if name == 'Panel':
            slots += [0x4b0, 0x4b8, 0x4c0, 0x4c8, 0x4d0, 0x4d8, 0x4e0]
        records[name]['vtable'] = {'address': hex(address), 'slots': {hex(slot): hex(struct.unpack_from('<Q', data, address - BASE + slot)[0]) for slot in slots}}
    records['event_params'] = {}
    for name, address in [('OnBeginIntro', 0x147830080), ('OnBeginOutro', 0x1478300d0)]:
        offset = address - BASE
        records['event_params'][name] = {'address': hex(address), 'raw72': data[offset:offset + 72].hex(), 'flags': hex(struct.unpack_from('<I', data, offset + 64)[0]), 'num_properties': struct.unpack_from('<I', data, offset + 56)[0]}
    (out / 'decoded.json').write_text(json.dumps(records, indent=2) + '\n')
    starts = [0x1407bb5f0, 0x1407bb630, 0x1407d1fb0, 0x1407d26e0, 0x1407d2330, 0x1407d2950, 0x140807e00, 0x140808180, 0x1407ae640, 0x1407c9500,
              0x1407dbc80, 0x1407dbdd0, 0x1407dbf20, 0x1407dc080,
              0x1407dc280, 0x1407fc000, 0x1407c2eb0, 0x1407f66a0,
              0x1408091b0, 0x140818410, 0x14080f000, 0x1407ac590]
    raw = {}
    for address in starts:
        # Preserve a bounded raw window, not an inferred function extent.
        # Exact instruction extents and branches are in the frozen asm files.
        payload = data[address - BASE:address - BASE + 0x1000]
        path = out / (hex(address) + '.bin')
        path.write_bytes(payload)
        raw[hex(address)] = {'window_size': len(payload), 'sha256': sha(path)}
    # Small thunks have no unwind entries. Preserve explicit adjacent 32-byte slots.
    for address in [0x14080bc70, 0x14080bc90, 0x14080a2c0, 0x14080a2e0, 0x14080bae0, 0x14080bb80, 0x140777600, 0x140808fc0, 0x140808fd0, 0x1406801c0]:
        (out / (hex(address) + '.bin')).write_bytes(data[address - BASE:address - BASE + 32])
    (out / 'raw-functions.json').write_text(json.dumps(raw, indent=2) + '\n')
    for address, size in [(0x1401c5c80, 0xc0), (0x145a03300, 0x120)]:
        (out / (hex(address) + '.bin')).write_bytes(data[address - BASE:address - BASE + size])
    sources = [Path(__file__).resolve(), ROOT / 'porting/extract_reflection.py', ROOT / 'porting/material_pipeline/view_original_probe.py', ROOT / 'porting/ghidra/decompile_targets.py', ROOT / 'porting/generated/reflection/native_funcs.json', ROOT / 'porting/generated/reflection/native_class_index.json', ROOT / 'porting/generated/frontend-probes/native-super-classification-6.json']
    for batch in range(1, args.last_batch + 1):
        sources.extend(sorted((ROOT / f'porting/generated/frontend-native-evidence/common-activatable-panel-events-{batch}').rglob('*')))
    bindings = []
    frozen = out / 'inputs'
    frozen.mkdir()
    for source in sources:
        if not source.is_file():
            continue
        digest = sha(source)
        target = frozen / digest
        if not target.exists():
            target.write_bytes(source.read_bytes())
        assert sha(target) == digest
        bindings.append({'source': str(source), 'snapshot': str(target), 'sha256': digest})
    receipt = {'schema': 'original-common-activatable-panel-checkpoint-v1', 'original': {'path': str(dump), 'sha256': EXPECTED, 'addressing': 'VA = imagebase + file offset', 'imagebase': hex(BASE)}, 'inputs': bindings, 'outputs': [{'path': str(path), 'sha256': sha(path)} for path in sorted(out.glob('*')) if path.is_file()], 'runtime_claim': False}
    (out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps({'receipt': str(out / 'receipt.json'), 'sha256': sha(out / 'receipt.json'), 'inputs': len(bindings)}))


if __name__ == '__main__':
    main()
