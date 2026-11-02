"""Focused fail-closed tests; no source generation or full-image hashing."""
import copy
import importlib.util
import json
import os
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
spec = importlib.util.spec_from_file_location('focus_selection_policy', HERE / 'policy.py')
policy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(policy)
reflection = ROOT / 'porting/generated/reflection'
classes = json.loads((reflection / 'fortnite_game_reflection.json').read_text())
blocks = {b['block_va']: b for b in classes['classes_in_groups_with_fort_names']}
index = json.loads((reflection / 'native_class_index.json').read_text())
names = {name: index[name]['block_va'] for name in ('UCommonButton', 'UCommonInputSettings', 'UCommonUIInputData')}
funcs = json.loads((reflection / 'native_funcs.json').read_text())
policy.validate_classes(blocks, names, funcs)
changed = copy.deepcopy(blocks)
record = next(p for p in changed[names['UCommonInputSettings']]['properties'] if p['name'] == 'InputDataClass')
record['offset'] += 1
try:
    policy.validate_classes(changed, names, funcs)
    raise AssertionError('changed original property accepted')
except ValueError:
    pass
changed = copy.deepcopy(funcs)
changed['UCommonButton']['HandleButtonClicked']['exec_addr'] = '0x0'
try:
    policy.validate_classes(blocks, names, changed)
    raise AssertionError('changed native exec accepted')
except ValueError:
    pass
image = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
policy.validate_original_image(image)
# Sparse fixture contains only the exact mapped windows; corruption must fail.
source = os.open(image, os.O_RDONLY)
try:
    with tempfile.TemporaryFile() as fixture:
        for item in policy.CONTRACT['windows']:
            pos = int(item['va'], 16) - 0x140000000
            os.pwrite(fixture.fileno(), os.pread(source, item['length'], pos), pos)
        policy.validate_original_image('/proc/self/fd/' + str(fixture.fileno()))
        pos = int(policy.CONTRACT['windows'][0]['va'], 16) - 0x140000000
        b = os.pread(fixture.fileno(), 1, pos)
        os.pwrite(fixture.fileno(), bytes([b[0] ^ 1]), pos)
        try:
            policy.validate_original_image('/proc/self/fd/' + str(fixture.fileno()))
            raise AssertionError('altered original instruction accepted')
        except ValueError:
            pass
finally:
    os.close(source)
print('original metadata and bounded source guards: PASS; mutations rejected')
