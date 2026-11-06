"""Guard native state callbacks against original reflected contracts."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent

def apply(model, block, owner_fns):
    if model['name'] != 'AFortPlayerStateAthena':
        return model
    expected = json.loads((HERE / 'metadata-contract.json').read_text())
    if model['block_va'] != expected['block_va'] or block['block_va'] != expected['block_va']:
        raise ValueError('player-state callback class identity changed')
    for prop in expected['properties']:
        matches = [p for p in block['properties'] if p['name'] == prop['name']]
        if matches != [prop]:
            raise ValueError('player-state callback field changed: ' + prop['name'])
    current = json.loads((HERE.parents[2] / 'porting/generated/reflection/native_funcs.json').read_text())['AFortPlayerStateAthena']
    for name, fn in expected['functions'].items():
        if current.get(name) != fn:
            raise ValueError('player-state callback reflection changed: ' + name)
    return model
