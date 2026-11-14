"""Admission guards for GetMaxAcceleration and the existing pawn virtual target.

Integration must call apply(model) after behavior splice and finalize(model, header,
cpp) after rendering. This policy does not create a second pawn helper.
"""
import hashlib
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ENTRIES = json.loads((HERE / 'manifest-entries.proposed.json').read_text())
PAWN_BODY = ROOT / 'porting/behavior/vk3_runtime/AFortPawn/NativeGetCurrentMovementStyle14231a980.inc'
PAWN_HASH = '5d6bbd063ec8240920221d39e76b280631763ae71f771eb457287ff6b9a6d782'
PAWN_HELPER = 'NativeGetCurrentMovementStyle14231a980'

def one_helper(model, name):
    found = [h for h in model.get('helpers', []) if h.get('name') == name]
    competing = [h for group in ('functions', 'static_helpers', 'fields', 'members_decl_order')
                 for h in model.get(group, []) if h.get('name') == name]
    if len(found) != 1 or competing:
        raise ValueError(model['name'] + ': missing or duplicate ' + name + '; rejected=' + repr(model.get('splice', {}).get('rejected', [])))
    return found[0]

def apply(model):
    name = model.get('name')
    if name == 'AFortPawn':
        if model.get('package') != '/Script/FortniteGame' or model.get('parent') != 'ACharacter':
            raise ValueError('AFortPawn ancestry changed')
        if hashlib.sha256(PAWN_BODY.read_bytes()).hexdigest() != PAWN_HASH:
            raise ValueError('original movement-style snippet changed')
        h = one_helper(model, PAWN_HELPER)
        if (h.get('ret'), h.get('args'), h.get('const'), h.get('access'), h.get('kind'), h.get('static'), h.get('override')) != ('uint8*', ['uint8* OutValue'], True, 'public', 'helper', False, False):
            raise ValueError('movement-style helper signature changed')
        if h.get('body', '').strip() != PAWN_BODY.read_text().strip() or h.get('virtual') not in (False, True):
            raise ValueError('movement-style helper body or virtual contract changed')
        tag = 'AFortPawn::' + PAWN_HELPER
        if model.get('splice', {}).get('accepted', []).count(tag) != 1:
            raise ValueError('movement-style helper not uniquely admitted')
        getter = one_helper(model, 'GetAbilitySystemComponent')
        if (getter.get('ret'), getter.get('args'), getter.get('const'), getter.get('virtual')) != ('UAbilitySystemComponent*', [], True, True):
            raise ValueError('Fort pawn ability interface virtual getter changed')
        if 'IFortAbilitySystemInterface' not in model.get('native_interface_bases', []):
            raise ValueError('Fort pawn ability interface inheritance changed')
        style = [m for m in model.get('members_decl_order', []) if m.get('name') == 'CurrentMovementStyle']
        if len(style) != 1 or 'EFortMovementStyle' not in style[0].get('decl', ''):
            raise ValueError('Fort pawn movement style field changed')
        h['virtual'] = True
    elif name == 'UFortMovementComp_Character':
        if model.get('package') != '/Script/FortniteGame' or model.get('parent') != 'UCharacterMovementComponent':
            raise ValueError('movement component ancestry changed')
        for entry in ENTRIES:
            h = one_helper(model, entry['function'])
            expected = (HERE / Path(entry['inc_path']).name).read_text().strip()
            expected_virtual = entry.get('virtual', False)
            expected_override = entry.get('override', False)
            if (h.get('ret'), h.get('args'), h.get('const'), h.get('access'), h.get('kind'), h.get('static'), h.get('virtual'), h.get('override')) != ('float', [], True, 'public', 'helper', False, expected_virtual, expected_override) or h.get('body', '').strip() != expected:
                raise ValueError(name + ': changed helper ' + entry['function'])
            if model.get('splice', {}).get('accepted', []).count(name + '::' + entry['function']) != 1:
                raise ValueError(name + ': helper not uniquely admitted ' + entry['function'])
        selector = one_helper(model, 'GetAirControlParams')
        if (selector.get('ret'), selector.get('args'), selector.get('const')) != ('const FAirControlParams&', [], True) or not selector.get('body'):
            raise ValueError('air selector dependency missing')
        field = [m for m in model.get('members_decl_order', []) if m.get('name') == 'SurfaceSwimmingWaterControlParams']
        if len(field) != 1 or field[0].get('decl', '').strip() != 'FWaterControlParams SurfaceSwimmingWaterControlParams;':
            raise ValueError('water control property changed')
        includes = model.setdefault('cpp_includes', [])
        for inc in ('AFortPlayerPawn.h', 'AFortCharacterVehicle_Ostrich.h', 'UFortAbilitySystemComponent.h', 'UFortJumpBoostSet.h'):
            if inc not in includes: includes.append(inc)
    return model

def compact(s):
    return re.sub(r'\s+', '', s)

def finalize(model, header, cpp):
    name = model.get('name')
    if name not in ('AFortPawn', 'UFortMovementComp_Character'):
        return header, cpp
    apply(model)
    declarations = ['virtual uint8* NativeGetCurrentMovementStyle14231a980(uint8* OutValue) const;'] if name == 'AFortPawn' else [
        'float GetAirControlLevel() const;', 'virtual float GetMaxAcceleration() const override;']
    for decl in declarations:
        if compact(header).count(compact(decl)) != 1:
            raise ValueError(name + ': emitted declaration missing or duplicated: ' + decl)
    for fn in ([PAWN_HELPER] if name == 'AFortPawn' else [e['function'] for e in ENTRIES]):
        matches = list(re.finditer(r'\b' + name + '::' + fn + r'\([^;{}]*\)\s*const\s*\{', cpp))
        if len(matches) != 1: raise ValueError(name + ': missing or duplicate definition ' + fn)
        start = end = matches[0].end(); depth = 1
        while end < len(cpp) and depth:
            depth += (cpp[end] == '{') - (cpp[end] == '}'); end += 1
        if depth or compact(cpp[start:end-1]) != compact(one_helper(model, fn)['body']):
            raise ValueError(name + ': emitted definition changed ' + fn)
    return header, cpp
