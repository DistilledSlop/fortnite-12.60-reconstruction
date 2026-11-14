"""Project recovered reflected Athena movement defaults into each native instance.

This is deliberately not the complete original constructor; see EVIDENCE.md.
"""
import hashlib
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent


def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    for source, key in [('image', 'source_windows'), ('pe', 'pe_windows')]:
        with Path(evidence[source]).open('rb') as stream:
            for window in evidence[key]:
                stream.seek(int(window['offset'], 0))
                data = stream.read(window['length'])
                if hashlib.sha256(data).hexdigest() != window['sha256']:
                    raise ValueError('Athena dual-move evidence changed: ' + window['address'])
    return {name: (HERE / name).read_text() for name in
            ('FortAthenaMovementNative.h', 'FortAthenaMovementNative.cpp')}

CLASS = 'UFortMovementComp_CharacterAthena'
OWN = ('TArray<FAthenaJumpPenalty> JumpPenalties;', 'float JumpPenaltyResetTime;',
       'int32 NetworkSkipSkyDivingCollisionForPawnLOD;', 'float MaxAccelerationFlying;')
FIELDS = ('JumpPenalties', 'JumpPenaltyResetTime', 'NetworkSkipSkyDivingCollisionForPawnLOD',
          'MaxAccelerationFlying', 'NetworkMaxSmoothUpdateDistance', 'NetworkNoSmoothUpdateDistance',
          'NetworkSimulatedSmoothLocationTime', 'NetworkSmoothingVisibilityThreshold',
          'MinTimeBetweenTimeStampResets', 'BrakingSubStepTime')


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'UFortMovementComp_Character':
        raise ValueError(CLASS + ': unexpected parent')
    include = '#include "FortAthenaMovementNative.h"'
    if include in cpp:
        raise ValueError(CLASS + ': competing dual-move support include')
    cpp = cpp.replace('#include "' + CLASS + '.h"',
                      '#include "' + CLASS + '.h"\n' + include, 1)
    if cpp.count(include) != 1:
        raise ValueError(CLASS + ': missing generated class include')
    for declaration in OWN:
        field = re.search(r'\b(\w+)\s*;$', declaration).group(1)
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        found = re.findall(r'^\s*[^\n;{}]*\b' + field + r'\b[^\n;{}]*;', header, re.M)
        if len(members) != 1 or members[0].get('decl', '').strip() != declaration or len(found) != 1 or found[0].strip() != declaration:
            raise ValueError(CLASS + ': unexpected field declaration: ' + field)
    decls = list(re.finditer(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.M))
    defs = list(re.finditer(r'\b' + CLASS + r'::' + CLASS + r'\s*\(', cpp))
    if len(decls) > 1 or len(defs) > 1 or bool(decls) != bool(defs):
        raise ValueError(CLASS + ': expected at most one matching constructor')
    if not defs:
        if header.count('public:') != 1:
            raise ValueError(CLASS + ': expected one public section')
        header = header.replace('public:', 'public:\n\t' + CLASS + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n', 1)
        cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n}\n' % (CLASS, CLASS)
    pattern = re.compile(r'\b' + CLASS + r'::' + CLASS + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError(CLASS + ': unsupported constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{': depth += 1
        elif cpp[end] == '}': depth -= 1
        end += 1
    if depth:
        raise ValueError(CLASS + ': unterminated constructor')
    if any(re.search(r'\b' + field + r'\b', cpp[match.start():end - 1]) for field in FIELDS):
        raise ValueError(CLASS + ': competing field use in constructor')
    body = '''    // Reflected-field projection of original constructor 0x1419b9480; see policy evidence.
    static_assert(TIsSame<decltype(FAthenaJumpPenalty::JumpScalar), float>::Value, "JumpScalar type changed");
    static_assert(TIsSame<decltype(FAthenaJumpPenalty::MovementScalar), float>::Value, "MovementScalar type changed");
    static_assert(TIsSame<decltype(NetworkMaxSmoothUpdateDistance), float>::Value, "movement field type changed");
    static_assert(TIsSame<decltype(NetworkNoSmoothUpdateDistance), float>::Value, "movement field type changed");
    static_assert(TIsSame<decltype(NetworkSimulatedSmoothLocationTime), float>::Value, "movement field type changed");
    static_assert(TIsSame<decltype(NetworkSmoothingVisibilityThreshold), float>::Value, "movement field type changed");
    static_assert(TIsSame<decltype(MinTimeBetweenTimeStampResets), float>::Value, "movement field type changed");
    static_assert(TIsSame<decltype(BrakingSubStepTime), float>::Value, "movement field type changed");
    JumpPenalties.SetNum(5);
    JumpPenalties[0].JumpScalar = 1.0f; JumpPenalties[0].MovementScalar = 1.0f;
    JumpPenalties[1].JumpScalar = 1.0f; JumpPenalties[1].MovementScalar = 1.0f;
    JumpPenalties[2].JumpScalar = 0.75f; JumpPenalties[2].MovementScalar = 0.75f;
    JumpPenalties[3].JumpScalar = 0.5f; JumpPenalties[3].MovementScalar = 0.5f;
    JumpPenalties[4].JumpScalar = 0.25f; JumpPenalties[4].MovementScalar = 0.25f;
    JumpPenaltyResetTime = 1.0f;
    NetworkMaxSmoothUpdateDistance = 768.0f;
    NetworkNoSmoothUpdateDistance = 1024.0f;
    NetworkSimulatedSmoothLocationTime = 0.15f;
    NetworkSmoothingVisibilityThreshold = 1.0f;
    NetworkSkipSkyDivingCollisionForPawnLOD = 2;
    MinTimeBetweenTimeStampResets = 1200.0f;
    BrakingSubStepTime = 1.0f / 60.0f;
    bEnableServerDualMoveScopedMovementUpdates = NativeServerDualMoveScopedUpdates1478c2860();
    MaxAccelerationFlying = 1000.0f;
'''
    return header, cpp[:end - 1] + body + cpp[end - 1:]
