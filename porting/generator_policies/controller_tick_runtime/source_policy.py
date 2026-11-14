"""Emit shared original controller tick support."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent

def sources():
    image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
    with image.open('rb') as stream:
        for item in json.loads((HERE / 'native-evidence.json').read_text())['windows']:
            stream.seek(int(item['begin'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(item['length'])).hexdigest() != item['sha256']:
                raise ValueError('controller tick evidence changed: ' + item['begin'])
    names = ('ControllerTick225b.h', 'ControllerTick225b.cpp', 'EncodedRecord.h')
    return {name: '// Generated from controller_tick_runtime.\n' + (HERE / name).read_text() for name in names}
