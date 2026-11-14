"""Conditional admission guards for the complete base speed and pawn targets.

Call apply after behavior splicing and finalize after rendering. Existing movement
max acceleration policy must run first: it restores pawn style virtual dispatch.
"""
import json
import re
from pathlib import Path
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ENTRIES = json.loads((HERE / 'manifest-entries.proposed.json').read_text())
INCLUDES = json.loads((HERE / 'includes.json').read_text())
PARENTS = {'AFortPawn':'ACharacter', 'AFortPlayerPawn':'AFortPawn',
           'AFortCharacterVehicle':'AFortPlayerPawnAthena',
           'AFortRemoteControlledPawnAthena':'AFortPlayerPawnAthena',
           'UFortMovementComp_Character':'UCharacterMovementComponent'}

def validate_required_classes(names):
    missing = set(PARENTS) - set(names)
    if missing: raise ValueError('base movement speed family missing classes: ' + ', '.join(sorted(missing)))

def one(model, name):
    found = [h for h in model.get('helpers', []) if h.get('name') == name]
    other = [h for group in ('functions','static_helpers','fields','members_decl_order') for h in model.get(group, []) if h.get('name') == name]
    if len(found) != 1 or other: raise ValueError(model['name'] + ': missing/competing helper ' + name)
    return found[0]

def apply(model):
    name = model.get('name')
    if name not in PARENTS: return model
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != PARENTS[name]: raise ValueError(name + ': ancestry changed')
    for e in ENTRIES:
        if e['class'] != name: continue
        fn = e['function']; h = one(model, fn); tag = name + '::' + fn
        args = ['uint8 Style'] if fn == 'NativeGetMovementSpeed14231ab10' else []
        expected = ('float', args, True, 'public', 'helper', False, True, e.get('override', False))
        actual = tuple(h.get(k) for k in ('ret','args','const','access','kind','static','virtual','override'))
        body = (ROOT / e['inc_path']).read_text().strip()
        splice = model.get('splice', {})
        if actual != expected or h.get('body','').strip() != body or splice.get('accepted', []).count(tag) != 1 or any(r.get('entry') == tag for r in splice.get('rejected', [])):
            raise ValueError(tag + ': changed signature, body or admission: ' + repr(actual))
    if name == 'AFortPawn':
        h = one(model, 'NativeGetCurrentMovementStyle14231a980')
        if (h.get('ret'), h.get('args'), h.get('const'), h.get('virtual')) != ('uint8*', ['uint8* OutValue'], True, True):
            raise ValueError('movement style selector must be virtual; run movement max acceleration policy first')
    includes = model.setdefault('cpp_includes', [])
    for inc in INCLUDES[name]:
        if inc not in includes: includes.append(inc)
    return model

def compact(s): return re.sub(r'\s+', '', s)

def finalize(model, header, cpp):
    name = model.get('name')
    if name not in PARENTS: return header, cpp
    apply(model)
    for e in ENTRIES:
        if e['class'] != name: continue
        fn = e['function']; decl = 'virtual ' + e['signature'] + (' override' if e.get('override') else '') + ';'
        if compact(header).count(compact(decl)) != 1: raise ValueError(name + ': declaration missing/duplicate ' + decl)
        matches = list(re.finditer(r'\b' + name + '::' + fn + r'\([^;{}]*\)\s*const\s*\{', cpp))
        if len(matches) != 1: raise ValueError(name + ': definition missing/duplicate ' + fn)
        start = end = matches[0].end(); depth = 1
        while end < len(cpp) and depth:
            depth += (cpp[end] == '{') - (cpp[end] == '}'); end += 1
        if depth or compact(cpp[start:end-1]) != compact(one(model, fn)['body']): raise ValueError(name + ': body changed ' + fn)
    for inc in INCLUDES[name]:
        if cpp.count('#include "' + inc + '"') != 1: raise ValueError(name + ': missing/duplicate include ' + inc)
    if name == 'AFortPawn' and compact(header).count(compact('virtual uint8* NativeGetCurrentMovementStyle14231a980(uint8* OutValue) const;')) != 1: raise ValueError('style virtual declaration missing')
    return header, cpp
