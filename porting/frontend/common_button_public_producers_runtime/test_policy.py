"""Bounded source and metadata checks; no generation or full-image read."""
import copy
import json
from pathlib import Path
import policy

root = Path(__file__).resolve().parents[3]
reflection = root / 'porting/generated/reflection'
classes = json.loads((reflection / 'fortnite_game_reflection.json').read_text())
blocks = {b['block_va']: b for b in classes['classes_in_groups_with_fort_names']}
index = json.loads((reflection / 'native_class_index.json').read_text())
names = {'UCommonButton': index['UCommonButton']['block_va']}
funcs = json.loads((reflection / 'native_funcs.json').read_text())
policy.validate_classes(blocks, names, funcs)
changed = copy.deepcopy(funcs['UCommonButton'])
changed['ClearSelection']['exec_addr'] = '0x0'
try:
    policy.validate_classes(blocks, names, {'UCommonButton': changed})
    raise AssertionError('mutated exec accepted')
except ValueError:
    pass
policy.validate_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
print('PASS: original bounded source and metadata; modified exec rejected')
