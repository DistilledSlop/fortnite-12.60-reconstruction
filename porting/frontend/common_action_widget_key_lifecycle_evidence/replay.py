"""Replay bounded original code and original reflected setter ownership."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
contract = json.loads((HERE / 'original-contract.json').read_text())
ref = ROOT / 'porting/generated/reflection'
classes = json.loads((ref / 'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
owner = next(c for c in classes if c['block_va'] == contract['class_block'])
assert owner['class_noregister_fn'] == contract['class_noregister_fn']
names = ('SetInputAction', 'SetInputActions')
assert [f for f in owner['functions'] if f['name'] in names] == contract['functions']
native = json.loads((ref / 'native_funcs.json').read_text())['UCommonActionWidget']
assert {name: native[name] for name in names} == contract['native']
fd = os.open(contract['image'], os.O_RDONLY)
try:
    for window in contract['windows']:
        raw = os.pread(fd, window['size'], int(window['va'], 16) - 0x140000000)
        assert len(raw) == window['size'], window['name']
        assert hashlib.sha256(raw).hexdigest() == window['sha256'], window['name']
finally:
    os.close(fd)
print('PASS: reflected setters/native joins and 19 bounded original windows; no whole-image hash')
