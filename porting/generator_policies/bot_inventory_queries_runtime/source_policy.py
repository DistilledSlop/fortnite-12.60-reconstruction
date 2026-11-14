"""Emit shared original bot CVar and logging support."""
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
                raise ValueError('bot inventory evidence changed: ' + item['begin'])
    original = Path('/mnt/vibrato/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe')
    with original.open('rb') as stream:
        stream.seek(0x78c17a1)
        if stream.read(1) != bytes((1,)):
            raise ValueError('Phoebe.InfiniteAmmo initialized PE default changed')
    names = ('PhoebeInfiniteAmmoCVar.h', 'PhoebeInfiniteAmmoCVar.cpp',
             'FortAthenaBotsLog.h', 'FortAthenaBotsLog.cpp')
    return {name: '// Generated from bot_inventory_queries_runtime.\n' + (HERE / name).read_text() for name in names}
