"""Emit the original native profile entry as typed module support source."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
MODULE = 'McpProfileSys'
DEPENDENCIES = ('Core', 'CoreUObject', 'Json', 'JsonUtilities')


def sources():
    evidence = json.loads((HERE / 'evidence.json').read_text())
    image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
    # Constructor, copy constructor, JSON update, and actual owner collection.
    required = {'0x1410ebe70', '0x1410ebf10', '0x1411042a0', '0x1410f1290'}
    receipts = [item for item in evidence['functions'] if item['target'] in required]
    if {item['target'] for item in receipts} != required:
        raise ValueError('missing original profile entry receipts')
    with image.open('rb') as stream:
        for item in receipts:
            start, end = int(item['begin'], 16), int(item['end'], 16)
            stream.seek(start - 0x140000000)
            if hashlib.sha256(stream.read(end - start)).hexdigest() != item['sha256']:
                raise ValueError('profile entry original capture changed: ' + item['target'])
    json_dir = HERE.parent / 'profile_entry_json_runtime'
    with image.open('rb') as stream:
        for item in json.loads((json_dir / 'native-evidence.json').read_text())['windows']:
            stream.seek(int(item['begin'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(item['length'])).hexdigest() != item['sha256']:
                raise ValueError('profile JSON original capture changed: ' + item['begin'])
    banner = '// Generated from profile_entry_lifecycle_runtime; do not edit by hand.\n'
    result = {name: banner + (HERE / name).read_text() for name in ('McpProfileItem.h', 'McpProfileItem.cpp')}
    result.update({name: banner + (json_dir / name).read_text() for name in ('McpProfileJsonIntern.h', 'McpProfileJsonIntern.cpp')})
    return result
