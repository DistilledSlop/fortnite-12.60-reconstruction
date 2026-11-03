"""Original reflected RPC contracts for the pawn-to-ASC forwarding path."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent

def validate():
    root = HERE.parents[2]
    current = json.loads((root / 'porting/generated/reflection/native_funcs.json').read_text())
    contracts = json.loads((HERE / 'native-contracts.json').read_text())
    for owner, funcs in contracts['functions'].items():
        for name, expected in funcs.items():
            if current[owner][name] != expected:
                raise ValueError('pawn cue RPC reflection changed: ' + owner + '::' + name)
