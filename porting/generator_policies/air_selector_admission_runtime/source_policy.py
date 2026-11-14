"""Fail-closed admission for the six original air selector helpers.

Integration invokes validate_required_classes once and finalize per rendered model.
This is conditional on the full PhysCustom lifecycle supplying the shared byte.
"""
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
ENTRIES = json.loads((HERE / 'manifest-entries.json').read_text())
PARENTS = {'UFortMovementComp_Character': 'UCharacterMovementComponent',
           'AFortPlayerControllerAthena': 'AFortPlayerControllerZone'}
PARAMS = ('SkydivingControlParamsPassive', 'SkydivingControlParamsActive',
          'ParachuteControlParamsSkydive', 'ParachuteControlParamsJump',
          'ParachuteControlParamsGliderRedeploy', 'BalloonControlParams',
          'FloatingControlParams', 'VortexSkydivingControlParamsPassive',
          'VortexSkydivingControlParamsActive', 'VortexSkydivingControlParamsActiveUpward',
          'VortexParachuteControlParams', 'SkyTubeSkydivingControlParamsPassive',
          'SkyTubeSkydivingControlParamsActive', 'SkyTubeSkydivingControlParamsActiveUpward')
FIELDS = {'UFortMovementComp_Character': {**{n: 'FAirControlParams ' + n + ';' for n in PARAMS},
                                         'CustomFallingAcceleration': 'FVector CustomFallingAcceleration;'},
          'AFortPlayerControllerAthena': {'SkydiveLeader': 'AFortPlayerState* SkydiveLeader;'}}

def validate_required_classes(names):
    missing = set(PARENTS) - set(names)
    if missing:
        raise ValueError('air selector family missing classes: ' + ', '.join(sorted(missing)))

def body_for(e):
    return (HERE / Path(e['inc_path']).name).read_text()

def validate_model(model):
    name = model['name']
    if name not in PARENTS:
        return
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != PARENTS[name]:
        raise ValueError(name + ': ancestry/package changed')
    for field, decl in FIELDS[name].items():
        matches = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        if len(matches) != 1 or matches[0].get('decl', '').strip() != decl:
            raise ValueError(name + ': reflected field changed: ' + field)
    if name == 'UFortMovementComp_Character':
        for dep in ('NativeHasSkyTube_1420e5300', 'NativeVortexOrSkyTube_1420d9730'):
            found = [h for h in model.get('helpers', []) if h.get('name') == dep]
            if len(found) != 1 or found[0].get('ret') != 'bool' or found[0].get('args') != [] or not found[0].get('const') or not found[0].get('body'):
                raise ValueError(name + ': missing predicate dependency ' + dep)
    for e in ENTRIES:
        if e['class'] != name:
            continue
        fn = e['function']; tag = name + '::' + fn
        splice = model.get('splice', {})
        if splice.get('accepted', []).count(tag) != 1 or any(r.get('entry') == tag for r in splice.get('rejected', [])):
            raise ValueError(tag + ': missing, duplicate or rejected admission: ' + repr(splice.get('rejected', [])))
        if e['kind'] == 'member_field':
            matches = [f for f in model.get('fields', []) if f.get('name') == fn]
            if len(matches) != 1 or any(matches[0].get(k) != v for k, v in {'type':'uint8','decl':'uint8 NativeAirborneFlag;', 'access':'public', 'default':None, 'offset':0x1dfe}.items()):
                raise ValueError(tag + ': native byte changed')
            # Do not allow another native field at the same original byte.
            if any(f.get('name') != fn and isinstance(f.get('offset'), int) and 0x1df0 <= f['offset'] <= 0x1dfe for f in model.get('fields', [])):
                raise ValueError(tag + ': competing native byte')
            if any(isinstance(m.get('offset'), int) and 0x1df0 <= m['offset'] <= 0x1dfe for m in model.get('members_decl_order', [])):
                raise ValueError(tag + ': reflected byte collision')
            continue
        helpers = [h for h in model.get('helpers', []) if h.get('name') == fn]
        if len(helpers) != 1 or any(x.get('name') == fn for k in ('functions','static_helpers','fields','members_decl_order') for x in model.get(k, [])):
            raise ValueError(tag + ': missing/competing helper')
        h = helpers[0]
        ret = e['signature'].split(' ' + fn + '(')[0]
        if ((h.get('ret'), h.get('args'), h.get('const'), h.get('access'), h.get('kind')) != (ret, [], True, 'public', 'helper')
            or h.get('static') or bool(h.get('virtual')) != e.get('virtual', False) or h.get('override')
            or h.get('body', '').strip() != body_for(e).strip()):
            raise ValueError(tag + ': signature/body changed')

def compact(s):
    return re.sub(r'\s+', '', s)

def finalize(model, header, cpp):
    validate_model(model)
    name = model['name']
    if name not in PARENTS:
        return header, cpp
    for e in ENTRIES:
        if e['class'] != name: continue
        fn = e['function']
        decl = e['signature'] + ';' if e['kind'] == 'helper' else e['signature'] + ';'
        if e.get('virtual'): decl = 'virtual ' + decl
        if compact(header).count(compact(decl)) != 1:
            raise ValueError(name + ': declaration missing/duplicated: ' + fn)
        if e['kind'] == 'member_field': continue
        matches = list(re.finditer(r'\b' + name + '::' + fn + r'\([^;{}]*\)\s*(?:const\s*)?\{', cpp))
        if len(matches) != 1: raise ValueError(name + ': definition missing/duplicated: ' + fn)
        start = end = matches[0].end(); depth = 1
        while end < len(cpp) and depth:
            depth += (cpp[end] == '{') - (cpp[end] == '}'); end += 1
        if depth or compact(cpp[start:end-1]) != compact(body_for(e)):
            raise ValueError(name + ': emitted body changed: ' + fn)
    if name == 'UFortMovementComp_Character':
        for inc in ('AFortPlayerPawn.h', 'AFortPlayerControllerAthena.h', 'AFortPlayerState.h', 'AFortSkyTube.h'):
            if cpp.count('#include "' + inc + '"') != 1:
                raise ValueError(name + ': missing or duplicate inferred include ' + inc)
    return header, cpp
