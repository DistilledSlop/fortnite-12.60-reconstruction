"""Replay the original reflection records and disassembly windows, read-only."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
contract = json.loads((HERE / 'original-contract.json').read_text())
ref = ROOT / 'porting/generated/reflection'
classes = json.loads((ref / 'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
owner = next(c for c in classes if c['block_va'] == contract['class_block'])
assert owner['class_noregister_fn'] == contract['class_noregister_fn']
assert [f for f in owner['functions'] if f['name'] == 'GetDisplayText'] == [contract['function']]
assert [p for p in owner['properties'] if p['name'] == 'InputActions'] == contract['input_actions']
assert json.loads((ref / 'native_funcs.json').read_text())['UCommonActionWidget']['GetDisplayText'] == contract['native_func']
image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
hash_ = hashlib.sha256()
with image.open('rb') as f:
    for chunk in iter(lambda: f.read(4 * 1024 * 1024), b''):
        hash_.update(chunk)
    assert hash_.hexdigest() == contract['image_sha256']
    for va, expected in contract['windows'].items():
        f.seek(int(va, 16) - 0x140000000)
        assert f.read(len(bytes.fromhex(expected))) == bytes.fromhex(expected), va
print('PASS: owner, function, input property, native exec, pinned image and instruction windows')
