"""Fail-closed source contract for the Win64 FortKismetLibrary implementation."""
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())


def validate(block, native_execs, image_path):
    if block.get('block_va') != CONTRACT['owner_block'] or block.get('class_noregister_fn') != CONTRACT['owner_noregister']:
        raise ValueError('FortKismetLibrary owner mismatch')
    if [f for f in block.get('functions', []) if f.get('name') == 'IsMobileGame'] != [CONTRACT['function']]:
        raise ValueError('IsMobileGame original function mismatch')
    if native_execs.get('IsMobileGame') != CONTRACT['exec']:
        raise ValueError('IsMobileGame native exec mismatch')
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for address, expected in CONTRACT['spans'].items():
            expected = bytes.fromhex(expected)
            if os.pread(fd, len(expected), int(address, 16) - int(CONTRACT['image_base'], 16)) != expected:
                raise ValueError('IsMobileGame original instruction mismatch at ' + address)
    finally:
        os.close(fd)


def validate_rendered(header, source):
    for output, name in ((header, 'declarations.inc'), (source, 'definitions.inc')):
        if output.count((HERE / name).read_text().strip()) != 1:
            raise ValueError('IsMobileGame source declaration/body missing or duplicated')
