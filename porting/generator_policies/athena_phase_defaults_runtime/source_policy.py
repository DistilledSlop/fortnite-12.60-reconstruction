"""Recover the Athena game-state constructor's phase-time sentinels."""
import re

CLASS = 'AFortGameStateAthena'
FIELDS = ('WarmupCountdownStartTime', 'WarmupCountdownEndTime',
          'AircraftStartTime', 'SafeZonesStartTime', 'EndGameStartTime',
          'EndGameKickPlayerTime', 'PlaylistEndTime', 'SafeZonePauseTime')


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'AFortGameStateZone':
        raise ValueError(CLASS + ': phase default policy parent changed')
    for name in FIELDS:
        if len(re.findall(r'\bfloat\s+' + name + r'\s*;', header)) != 1:
            raise ValueError(CLASS + ': missing or competing phase time: ' + name)
    if (re.search(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.MULTILINE)
            or re.search(r'^\s*' + CLASS + '::' + CLASS + r'\s*\(', cpp, re.MULTILINE)):
        raise ValueError(CLASS + ': merge phase defaults with existing constructor explicitly')
    declaration = '\n\t' + CLASS + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n'
    if 'public:' not in header:
        raise ValueError(CLASS + ': missing public declaration section')
    header = header.replace('public:', 'public:' + declaration, 1)
    body = '\n'.join('\t%s = -1.0f;' % name for name in FIELDS)
    cpp += ('\n%s::%s(const FObjectInitializer& ObjectInitializer)\n'
            '\t: Super(ObjectInitializer)\n{\n'
            '\t// Original ctor 0x14197f7b0, stores 0x141980d44..0x141980d76.\n'
            '\t// PlaylistEndTime / SafeZonePauseTime: stores 0x14197fa6d / 0x14197fa78.\n'
            '%s\n}\n') % (CLASS, CLASS, body)
    return header, cpp
