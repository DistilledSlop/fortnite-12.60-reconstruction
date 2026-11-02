"""Fail-closed input contract and source for the recovered CommonInput module."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
WINDOWS = json.loads((HERE / 'original-windows.json').read_text())['windows']


def validate_source(image_path):
    """Verify the bounded original module, registration and lifecycle evidence."""
    with Path(image_path).open('rb') as image:
        for window in WINDOWS:
            image.seek(int(window['va'], 16) - 0x140000000)
            data = image.read(window['size'])
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('original CommonInput module code differs: ' + window['va'])


def validate_classes(blocks, name_to_block):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if (name_to_block.get('UCommonInputSettings') != identity['block_va'] or block is None
            or any(block.get(k) != v for k, v in identity.items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('original CommonInputSettings identity or properties differ')


def sources(module, blocks, name_to_block):
    if module != 'CommonInput':
        raise ValueError('CommonInput module policy used for another module')
    validate_classes(blocks, name_to_block)
    return {name: (HERE / name).read_text() for name in
            ('CommonInput.cpp', 'CommonInputModuleRuntime.h')}
