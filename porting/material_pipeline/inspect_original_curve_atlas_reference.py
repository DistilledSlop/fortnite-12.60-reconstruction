#!/usr/bin/env python3
"""Freeze original CurveAtlasRowParameter reference hooks, without executing the client."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
BASE = 0x140000000
EXPECTED = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    out = parser.parse_args().output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    dump = ROOT / 'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin'
    assert sha(dump) == EXPECTED
    source = ROOT / 'porting/extract_reflection.py'
    spec = importlib.util.spec_from_file_location('reflection', source)
    reflection = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(reflection)

    class MemoryImage(reflection.Image):
        def off(self, address):
            offset = address - BASE if address else -1
            return offset if 0 <= offset < len(self.m) else None

    image = MemoryImage(dump)
    assert image.imagebase == BASE
    windows = {0x146bd7600: 88, 0x144788ae0: 0xc0, 0x144789ec0: 0x40,
               0x1443050b0: 0x60, 0x146ab40f0: 0x278, 0x1409bebd0: 5,
               0x1406801d0: 3, 0x14478a440: 0x10, 0x144791ca0: 0xc0,
               0x1446ef940: 0x10, 0x1446f0b70: 0xc0, 0x144307170: 0xc0}
    block = 0x146bd7600
    array = image.u64(block + 40)
    count = struct.unpack_from('<I', image.m, block - BASE + 64)[0]
    props = reflection.decode_props(image, array, count)
    atlas = next(row for row in props if row['name'] == 'Atlas')
    assert atlas['kind'] == 'Object' and atlas['offset'] == 0x68
    assert '0x1446ef940' in atlas['type_refs'].values()
    windows[array] = count * 8
    for i in range(count):
        record = image.u64(array + i * 8)
        windows[record] = 72
        for at in (record, record + 8):
            ptr = image.u64(at)
            if ptr:
                windows[ptr] = len(image.cstr(ptr).encode()) + 1
    assert image.u64(0x146ab40f0 + 0x268) == 0x1409bebd0
    assert image.u64(0x146ab40f0 + 0x270) == 0x1406801d0
    assert image.m[0x1409bebd0 - BASE:0x1409bebd0 - BASE + 5] == bytes.fromhex('488b4168c3')
    assert image.m[0x1406801d0 - BASE:0x1406801d0 - BASE + 3] == bytes.fromhex('b001c3')
    evidence = {'class_block': hex(block), 'properties': props,
                'class_getter': '0x144788ae0', 'constructor_callback': '0x144789ec0',
                'constructor': '0x1443050b0', 'vtable': '0x146ab40f0',
                'slots': {'0x268': {'function': '0x1409bebd0', 'hex': '488b4168c3', 'behavior': 'return object pointer at this+0x68'},
                          '0x270': {'function': '0x1406801d0', 'hex': 'b001c3', 'behavior': 'return true'}},
                'atlas_type_getter_thunk': '0x1446ef940', 'atlas_type_getter': '0x1446f0b70',
                'ordered_index_claim': False, 'runtime_claim': False}
    (out / 'reference-hooks.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raw = []
    for address, length in sorted(windows.items()):
        data = image.m[address - BASE:address - BASE + length]
        assert len(data) == length
        path = out / (hex(address) + '.bin')
        path.write_bytes(data)
        raw.append({'va': hex(address), 'fileoffset': address - BASE, 'size': length, 'path': str(path), 'sha256': sha(path)})
    sources = [Path(__file__).resolve(), source, ROOT / 'porting/material_pipeline/view_original_probe.py', ROOT / 'porting/ghidra/decompile_targets.py']
    for batch in range(1, 5):
        folder = ROOT / f'porting/generated/frontend-native-evidence/original-curve-atlas-row-texture-{batch}'
        assert folder.is_dir()
        sources += sorted(p for p in folder.rglob('*') if p.is_file())
    frozen = out / 'inputs'
    frozen.mkdir()
    inputs = []
    for path in sources:
        digest = sha(path)
        target = frozen / digest
        if not target.exists():
            target.write_bytes(path.read_bytes())
        assert sha(target) == digest
        inputs.append({'source': str(path), 'snapshot': str(target), 'sha256': digest})
    receipt = {'schema': 'original-curve-atlas-reference-v1', 'original': {'path': str(dump), 'sha256': EXPECTED, 'layout': 'MEMORY; VA=0x140000000+fileoffset'}, 'inputs': inputs, 'raw_windows': raw, 'outputs': [{'path': str(p), 'sha256': sha(p)} for p in sorted(out.iterdir()) if p.is_file()], 'runtime_claim': False, 'raw_windows_are_not_function_extents': True}
    receipt_path = out / 'receipt.json'
    receipt_path.write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps({'receipt': str(receipt_path), 'sha256': sha(receipt_path), 'inputs': len(inputs), 'raw_windows': len(raw)}))


if __name__ == '__main__':
    main()
