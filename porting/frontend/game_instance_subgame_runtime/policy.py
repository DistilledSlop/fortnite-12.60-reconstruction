"""Bounded provenance and incomplete, deliberately unregistered fragments."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'contract.json').read_text())


def validate_original(image_path):
    with Path(image_path).open('rb') as image:
        for item in CONTRACT['windows']:
            image.seek(int(item['va'], 0) - 0x140000000)
            data = image.read(item['size'])
            if len(data) != item['size'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('game instance subgame window differs: ' + item['label'])


def fragments():
    return {name: (HERE / filename).read_text() for name, filename in (
        ('declarations', 'delegates.declarations.inc'),
        ('members', 'delegates.members.inc'),
        ('game_instance_declarations', 'game-instance.declarations.inc'),
        ('game_instance_definitions', 'game-instance.definitions.inc'),
    )}


def apply(*args, **kwargs):
    raise ValueError('subgame runtime incomplete: recover gate and subgame state writers, '
                     'service transition 141deca00, subsystem virtual 298, GI refresh '
                     '141e78990, and native delegate bindings before integration')
