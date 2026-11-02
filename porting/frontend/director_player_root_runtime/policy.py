"""Source fragments and bounded original contracts, deliberately not registered.

Validation establishes provenance, not transitive dependency implementation.
See required_real_dependencies in contract.json before generator integration.
"""
import hashlib
import json
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'contract.json').read_text())


def validate_original(image_path):
    with Path(image_path).open('rb') as image:
        for item in CONTRACT['windows']:
            image.seek(int(item['va'], 0) - 0x140000000)
            data = image.read(item['size'])
            if len(data) != item['size'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('director/player/root source differs: ' + item['label'])
        for item in CONTRACT['slots']:
            image.seek(int(item['table'], 0) + int(item['slot'], 0) - 0x140000000)
            if image.read(8) != struct.pack('<Q', int(item['body'], 0)):
                raise ValueError('director/player/root virtual dispatch differs')


def validate_classes(blocks, names):
    for name, expected in CONTRACT['classes'].items():
        va = expected['block_va']
        if names.get(name) != va or blocks.get(va) != expected:
            raise ValueError('director/player/root class record differs: ' + name)


def fragments():
    """Return reviewable generator inputs, without enabling incomplete behavior."""
    return {
        'declarations': json.loads((HERE / 'declarations.json').read_text()),
        'definitions': {
            'UFortUIDirector': (HERE / 'director.definitions.inc').read_text(),
            'UFortUIManager': (HERE / 'manager.definitions.inc').read_text() + (HERE / 'manager-world.definitions.inc').read_text(),
        },
        'source_includes': {
            'UFortUIDirector': ['UFortUIManager.h', 'UFortLocalPlayer.h',
                                'UFortGameInstance.h', 'Misc/CommandLine.h', 'Misc/Parse.h'],
            'UFortUIManager': ['UFortLocalPlayer.h', 'Blueprint/UserWidget.h',
                              'Engine/LocalPlayer.h', 'Engine/GameInstance.h', 'GameFramework/PlayerController.h'],
        },
    }


def apply(*args, **kwargs):
    raise ValueError('director/player/root promotion is not closed: ' +
                     '; '.join(CONTRACT['required_real_dependencies']))
