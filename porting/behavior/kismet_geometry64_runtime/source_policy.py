"""Pin original math bodies and the manually owned Kismet implementations."""
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
    header = (root / 'porting/native/Source/FortniteGame/Public/FortniteMapClasses.h').read_text()
    source = (root / 'porting/native/Source/FortniteGame/Private/FortniteMapClasses.cpp').read_text()
    funcs = json.loads((root / 'porting/generated/reflection/native_funcs.json').read_text())['UFortKismetLibrary']
    for e in json.loads((HERE / 'manifest-entries.json').read_text()):
        name = e['function']
        sig = e['signature']
        if header.count(sig + ';') != 1:
            raise ValueError('Kismet math declaration changed: ' + name)
        ret, rest = sig.removeprefix('static ').split(' ', 1)
        body = (root / e['inc_path']).read_text()
        definition = ret + ' UFortKismetLibrary::' + rest + '\n{\n' + ''.join('    ' + line + '\n' if line else '\n' for line in body.splitlines()) + '}\n'
        if source.count(definition) != 1:
            raise ValueError('Kismet math implementation changed: ' + name)
        if funcs[name] != e['reflection_contract']:
            raise ValueError('Kismet math reflection changed: ' + name)
