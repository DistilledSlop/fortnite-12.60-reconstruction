"""Bounded original-image replay; this candidate is not generator-registered."""
import hashlib
import json
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())


def validate_original(image):
    with Path(image).open('rb') as stream:
        def read(address, length):
            stream.seek(address - 0x140000000)
            data = stream.read(length)
            if len(data) != length:
                raise ValueError('short original image read')
            return data
        for item in CONTRACT['windows']:
            start, end = int(item['start'], 0), int(item['end'], 0)
            if hashlib.sha256(read(start, end - start)).hexdigest() != item['sha256']:
                raise ValueError('original window mismatch: ' + item['label'])
        v = CONTRACT['vtable']
        if struct.unpack('<Q', read(int(v['table'], 0) + int(v['slot'], 0), 8))[0] != int(v['target'], 0):
            raise ValueError('CommonBorder sync virtual slot differs')
        name, target = struct.unpack('<QQ', read(int(CONTRACT['native_array'], 0), 16))
        if (name, target) != (int(CONTRACT['name'], 0), int(CONTRACT['native_exec'], 0)) or read(name, 9) != b'SetStyle\0':
            raise ValueError('CommonBorder native registration differs')
