"""Complete jump bodies; input and saved-move lifecycle acceptance remains open."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent


def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['source_windows']:
            stream.seek(int(window['address'], 0) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('base jump evidence changed: ' + window['address'])
    return {filename: (HERE / filename).read_text() for filename in
            ('FortJumpMathNative.h', 'FortParachuteInputNative.h', 'FortParachuteInputNative.cpp')}


def validate(ctx, snippets_by_class):
    if ctx['class_pkg'].get('UFortMovementComp_Character') != '/Script/FortniteGame':
        raise ValueError('base jump owner changed')
    fields = [e for e in snippets_by_class.get('UFortMovementComp_Character', [])
              if e.get('function') == 'NativeDoJumpFlag1dfd']
    if (len(fields) != 1 or fields[0].get('kind') != 'member_field'
            or fields[0].get('signature') != 'uint8 NativeDoJumpFlag1dfd'
            or int(str(fields[0].get('offset')), 0) != 0x1dfd
            or fields[0].get('default') is not None):
        raise ValueError('base jump shared native flag changed')
    for cls, name, signature, offset, default in [
        ('UFortMovementComp_CharacterAthena', 'NativeJumpPenaltyIndex1e54',
         'int32 NativeJumpPenaltyIndex1e54', 0x1e54, '0'),
        ('UFortMovementComp_CharacterAthena', 'NativeJumpPenaltyRemaining1e58',
         'float NativeJumpPenaltyRemaining1e58', 0x1e58, '0.0f'),
        ('UFortMovementComp_CharacterAthena', 'NativeMeleeSlowRemaining1e5c',
         'float NativeMeleeSlowRemaining1e5c', 0x1e5c, '0.0f'),
        ('ABuildingSMActor', 'NativeFloorAttributionWeak718',
         'TWeakObjectPtr<AController> NativeFloorAttributionWeak718', 0x718, None),
        ('AFortPlayerPawn', 'NativeParachuteRequest1fc0',
         'int32 NativeParachuteRequest1fc0', 0x1fc0, None),
        ('AFortPlayerPawn', 'NativeParachuteLastTime264c',
         'float NativeParachuteLastTime264c', 0x264c, '0.0f'),
        ('AFortPlayerControllerGameplay', 'NativeInputBlockTimer2568',
         'FTimerHandle NativeInputBlockTimer2568', 0x2568, None),
    ]:
        matches = [e for e in snippets_by_class.get(cls, [])
                   if e.get('function') == name]
        if (len(matches) != 1 or matches[0].get('kind') != 'member_field'
                or matches[0].get('signature') != signature
                or int(str(matches[0].get('offset')), 0) != offset
                or matches[0].get('default') != default):
            raise ValueError('Athena jump shared state changed: ' + cls + '::' + name)
