"""Bounded source and reflection replay; no whole-image hash."""
import json
from pathlib import Path
from policy import CONTRACT, validate_source

root = Path(__file__).resolve().parents[3]
classes = json.loads((root / 'porting/generated/reflection/fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
owner = next(c for c in classes if c.get('block_va') == CONTRACT['class_block'])
assert owner['class_noregister_fn'] == CONTRACT['class_noregister_fn']
assert owner['properties'] == CONTRACT['properties']
validate_source(CONTRACT['image'])
print('PASS: widget reflection and six bounded original windows; no whole-image hash')
