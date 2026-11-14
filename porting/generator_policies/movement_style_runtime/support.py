"""Typed storage and shared CVar for complete movement-style helper bodies.

This does not install the still-incomplete compressed-input/jump/ability paths.
"""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')


def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with IMAGE.open('rb') as stream:
        for window in evidence['source_windows']:
            stream.seek(int(window['address'], 0) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('movement-style evidence changed: ' + window['address'])
    return {
        'FortMovementStyleNative.h': '#pragma once\n#include "CoreMinimal.h"\nbool NativeRequireSprintTag1478de49c();\n',
        'FortMovementStyleNative.cpp': (HERE / 'FortMovementStyleNative.cpp').read_text(),
    }


def validate(ctx, snippets_by_class):
    fields = {
        'NativeSprintState1062Bit0': ('bool NativeSprintState1062Bit0', 0x1062),
        'NativeLocalMovementNotify2004Bit2': ('bool NativeLocalMovementNotify2004Bit2', 0x2004),
        'NativePersonalVehicleStyleChanged1608':
            ('TMulticastDelegate<void, bool> NativePersonalVehicleStyleChanged1608', 0x1608),
    }
    if ctx['class_pkg'].get('AFortPlayerPawn') != '/Script/FortniteGame':
        raise ValueError('movement-style owner changed')
    for name, (signature, offset) in fields.items():
        entries = [e for e in snippets_by_class.get('AFortPlayerPawn', []) if e.get('function') == name]
        if (len(entries) != 1 or entries[0].get('kind') != 'member_field'
                or entries[0].get('signature') != signature
                or int(str(entries[0].get('offset')), 0) != offset):
            raise ValueError('movement-style native field changed: ' + name)
