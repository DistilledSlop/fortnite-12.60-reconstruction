"""Fail-closed admission for the native flying-acceleration virtual family.

Call apply after splice and finalize after the Athena movement constructor policy.
Requires the independent complete FFortPawnStats row policy and base movement policy.
"""
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
ENTRIES = json.loads((HERE / 'manifest-entries.proposed.json').read_text())
PARENTS = {'AFortPawn': 'ACharacter', 'AFortPlayerPawn': 'AFortPawn',
           'AFortPlayerPawnAthena': 'AFortPlayerPawn',
           'UFortMovementComp_CharacterAthena': 'UFortMovementComp_Character'}
FLY = 'NativeGetFlyingAcceleration14231a880'
SCALE = 'MaxAccelerationFlyingMultiplier'


def validate_required_classes(names):
    missing = set(PARENTS) - set(names)
    if missing:
        raise ValueError('flying acceleration family missing classes: ' + ', '.join(sorted(missing)))


def one_helper(model, name):
    found = [h for h in model.get('helpers', []) if h.get('name') == name]
    competing = [h for key in ('functions', 'static_helpers', 'fields', 'members_decl_order')
                 for h in model.get(key, []) if h.get('name') == name]
    if len(found) != 1 or competing:
        raise ValueError(model['name'] + ': missing or duplicate helper ' + name + '; rejected=' + repr(model.get('splice', {}).get('rejected', [])))
    return found[0]


def field(model, name, declaration):
    matches = [m for m in model.get('members_decl_order', []) if m.get('name') == name]
    if len(matches) != 1 or matches[0].get('decl', '').strip() != declaration:
        raise ValueError(model['name'] + ': unexpected field ' + name)


def apply(model):
    name = model.get('name')
    if name not in PARENTS:
        return model
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != PARENTS[name]:
        raise ValueError(name + ': ancestry changed')
    for e in (e for e in ENTRIES if e['class'] == name):
        h = one_helper(model, e['function'])
        expected_ret = 'const FFortPawnStats*' if e['function'] == 'NativeGetPawnStats141a07b70' else 'float'
        actual = (h.get('ret'), h.get('args'), h.get('const'), h.get('access'), h.get('kind'),
                  h.get('static'), h.get('virtual'), h.get('override'))
        expected = (expected_ret, [], True, 'public', 'helper', False,
                    e.get('virtual', False), e.get('override', False))
        if actual != expected or h.get('body', '').strip() != (HERE / Path(e['inc_path']).name).read_text().strip():
            raise ValueError(name + ': signature or body changed for ' + e['function'])
        if model.get('splice', {}).get('accepted', []).count(name + '::' + e['function']) != 1:
            raise ValueError(name + ': helper not uniquely admitted')
    if name == 'AFortPawn':
        field(model, 'PawnStatHandle', 'FDataTableRowHandle PawnStatHandle;')
        field(model, 'AdvancedMovementSet', 'UFortAdvancedMovementSet* AdvancedMovementSet;')
        includes = ('FFortPawnStats.h', 'UFortAdvancedMovementSet.h')
    elif name == 'AFortPlayerPawn':
        includes = ('FFortPawnStats.h', 'UFortAdvancedMovementSet.h')
    elif name == 'AFortPlayerPawnAthena':
        includes = ('FFortPawnStats.h', 'UFortAdvancedMovementSet.h', 'UFortMovementComp_CharacterAthena.h')
    else:
        field(model, 'MaxAccelerationFlying', 'float MaxAccelerationFlying;')
        if any(x.get('offset') == 0x1e74 for key in ('fields', 'members_decl_order') for x in model.get(key, [])):
            raise ValueError(name + ': competing original multiplier offset')
        if any(x.get('name') == SCALE for key in ('helpers', 'functions', 'fields', 'members_decl_order') for x in model.get(key, [])):
            raise ValueError(name + ': competing multiplier declaration')
        includes = ('AFortPawn.h',)
    for inc in includes:
        if inc not in model.setdefault('cpp_includes', []): model['cpp_includes'].append(inc)
    return model


def compact(s):
    return re.sub(r'\s+', '', s)


def finalize(model, header, cpp):
    name = model.get('name')
    if name not in PARENTS: return header, cpp
    apply(model)
    for e in (e for e in ENTRIES if e['class'] == name):
        h = one_helper(model, e['function'])
        decl = ('virtual ' if h['virtual'] else '') + e['signature'] + (' override' if h['override'] else '') + ';'
        if compact(header).count(compact(decl)) != 1:
            raise ValueError(name + ': emitted declaration changed: ' + decl)
        matches = list(re.finditer(r'\b' + name + '::' + e['function'] + r'\([^;{}]*\)\s*const\s*\{', cpp))
        if len(matches) != 1: raise ValueError(name + ': emitted definition missing/duplicate')
        start = end = matches[0].end(); depth = 1
        while depth and end < len(cpp):
            depth += (cpp[end] == '{') - (cpp[end] == '}'); end += 1
        if depth or compact(cpp[start:end-1]) != compact(h['body']):
            raise ValueError(name + ': emitted definition changed')
    if name == 'UFortMovementComp_CharacterAthena':
        if re.search(r'\b' + SCALE + r'\b', header) or cpp.count(SCALE) != 1:
            raise ValueError(name + ': competing multiplier use')
        if header.count('public:') != 1: raise ValueError(name + ': unexpected public section')
        header = header.replace('public:', 'public:\n\t// Native original +0x1e74; distinct from reflected MaxAccelerationFlying.\n\tfloat ' + SCALE + ';', 1)
        pattern = re.compile(r'\b' + name + r'::' + name + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
        matches = list(pattern.finditer(cpp))
        if len(matches) != 1: raise ValueError(name + ': expected projected constructor')
        cpp = cpp[:matches[0].end()] + '\n    ' + SCALE + ' = 1.0f; // Original constructor 0x1419b9480.\n' + cpp[matches[0].end():]
    return header, cpp
