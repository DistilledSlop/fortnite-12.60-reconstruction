#!/usr/bin/env python3
"""Freeze bounded original CommonInputManager queue evidence, without Ghidra/UE writes."""
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
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--last-batch', type=int, default=3)
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    dump = ROOT / 'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin'
    assert sha(dump) == EXPECTED
    spec = importlib.util.spec_from_file_location('reflection', ROOT / 'porting/extract_reflection.py')
    reflection = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(reflection)

    class MemoryImage(reflection.Image):
        def off(self, address):
            offset = address - BASE if address else -1
            return offset if 0 <= offset < len(self.m) else None

    image = MemoryImage(dump)
    assert image.imagebase == BASE
    data = image.m
    record = 0x147835bb0
    fields = struct.unpack_from('<7Q', data, record - BASE)
    count = struct.unpack_from('<I', data, record - BASE + 56)[0]
    decoded = {'struct': {'address': hex(record), 'name': image.cstr(fields[3]), 'size': fields[4], 'alignment': fields[5], 'properties': reflection.decode_props(image, fields[6], count)}, 'enum': {}}
    enum = 0x147837c50
    fields = struct.unpack_from('<5QII', data, enum - BASE)
    decoded['enum'] = {'address': hex(enum), 'name': image.cstr(fields[2]), 'enumerators': [{'name': image.cstr(image.u64(fields[4] + i * 16)), 'value': struct.unpack_from('<q', data, fields[4] - BASE + i * 16 + 8)[0]} for i in range(fields[5])]}
    assert decoded['struct']['name'] == 'Operation' and decoded['struct']['size'] == 40
    assert decoded['enum']['enumerators'] == [{'name': 'EOperation::Intro', 'value': 0}, {'name': 'EOperation::Outro', 'value': 1}, {'name': 'EOperation::Push', 'value': 2}, {'name': 'EOperation::Pop', 'value': 3}, {'name': 'EOperation::Invalid', 'value': 4}]
    for address, expected in [(0x140746210, 'c6411001c3'), (0x14077d400, 'c6411101c3'), (0x14080f6e0, 'c6411201c3'), (0x14075d4a0, 'c6415001c3')]:
        assert data[address - BASE:address - BASE + 5].hex() == expected
    decoded['manager_tick'] = {'secondary_vtable': '0x1459ed5f8', 'this_adjustment': 0x28, 'slot0': hex(image.u64(0x1459ed5f8))}
    assert decoded['manager_tick']['slot0'] == '0x1407ffc50'
    decoded['startup_setup_slot'] = hex(image.u64(0x147011440 + 0x268))
    assert decoded['startup_setup_slot'] == '0x14515aac0'
    class_offset = 0x14700e250 - BASE
    decoded['fort_ui_manager_properties'] = reflection.decode_props(image, struct.unpack_from('<Q', data, class_offset + 40)[0], struct.unpack_from('<I', data, class_offset + 64)[0])
    (out / 'record.json').write_text(json.dumps(decoded, indent=2) + '\n')
    windows = {0x147835bb0: 72, 0x147837c50: 56, 0x14080f6e0: 16, 0x14077d400: 16, 0x140746210: 16, 0x1407e55a0: 32, 0x1407fc000: 0x1000, 0x1407ce8a0: 0x500, 0x1407e0c80: 0x500, 0x1407a95e0: 0x500, 0x1407ad3f0: 0x200, 0x1407ab920: 0x100}
    windows[fields[4]] = fields[5] * 16
    windows.update({0x140796de0: 0x400, 0x1459ed398: 0x2a0,
                    0x1407ffc50: 0x100, 0x1407c5600: 32,
                    0x1407f7b90: 0x300, 0x1407e1ed0: 0x80,
                    0x1407c9550: 32, 0x145a0c7d0: 0x278,
                    0x1407ae2c0: 0x100, 0x14515a790: 0x400,
                    0x14515aac0: 0x40, 0x145143bd0: 0x100,
                    0x145312900: 0x80, 0x145312b50: 0x100,
                    0x147011440: 0x2c0, 0x14075d4a0: 16,
                    0x1407f1b20: 0x200})
    for address, size in windows.items():
        (out / (hex(address) + '.bin')).write_bytes(data[address - BASE:address - BASE + size])
    sources = [Path(__file__).resolve(), ROOT / 'porting/extract_reflection.py', ROOT / 'porting/material_pipeline/view_original_probe.py', ROOT / 'porting/ghidra/decompile_targets.py', ROOT / 'porting/generated/reflection/native_funcs.json']
    for batch in [20, 21, 22, 24, 25, 26, 27, 28]:
        sources.extend(sorted((ROOT / f'porting/generated/frontend-native-evidence/common-activatable-panel-events-{batch}').rglob('*')))
    for batch in range(1, args.last_batch + 1):
        directory = ROOT / f'porting/generated/frontend-native-evidence/common-input-manager-queue-{batch}'
        assert directory.is_dir(), directory
        sources.extend(sorted(directory.rglob('*')))
    frozen = out / 'inputs'
    frozen.mkdir()
    bindings = []
    for source in sources:
        if not source.is_file():
            continue
        digest = sha(source)
        snapshot = frozen / digest
        if not snapshot.exists():
            snapshot.write_bytes(source.read_bytes())
        assert sha(snapshot) == digest
        bindings.append({'source': str(source), 'snapshot': str(snapshot), 'sha256': digest})
    receipt = {'schema': 'original-common-input-manager-queue-v1', 'original': {'path': str(dump), 'sha256': EXPECTED, 'layout': 'memory; VA=0x140000000+fileoffset'}, 'inputs': bindings, 'outputs': [{'path': str(path), 'sha256': sha(path)} for path in sorted(out.glob('*')) if path.is_file()], 'raw_code_is_bounded_windows_not_inferred_function_extents': True, 'runtime_claim': False}
    (out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps({'receipt': str(out / 'receipt.json'), 'sha256': sha(out / 'receipt.json'), 'inputs': len(bindings)}))


if __name__ == '__main__':
    main()
