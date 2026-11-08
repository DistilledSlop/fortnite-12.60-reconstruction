"""Pin original math bodies. UFortKismetLibrary is generator-owned now; the hand-written class was removed."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent

def validate():
    root = HERE.parents[2]
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for w in evidence['windows']:
            stream.seek(int(w['begin'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(w['length'])).hexdigest() != w['sha256']:
                raise ValueError('Kismet original math changed: ' + w['begin'])
    funcs = json.loads((root / 'porting/generated/reflection/native_funcs.json').read_text())['UFortKismetLibrary']
    for e in json.loads((HERE / 'manifest-entries.json').read_text()):
        name = e['function']
        sig = e['signature']
        if not (root / e['inc_path']).is_file():
            raise ValueError('Kismet math body missing: ' + name)
        if funcs[name] != e['reflection_contract']:
            raise ValueError('Kismet math reflection changed: ' + name)
