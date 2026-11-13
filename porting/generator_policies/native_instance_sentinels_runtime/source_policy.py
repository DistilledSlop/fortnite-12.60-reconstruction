"""Restore original native constructor defaults that must exist on each instance."""
import re

# class: parent, (exact member declaration, assignment, original store)
CLASSES = {
    'AFortGameStateZone': ('AFortGameState_InGame', (
        ('int32 WaitingToLeaveZoneTimeLeft;', '-1', '0x141e8b15d'),
        ('bool bDBNODeathEnabled;', 'true', '0x141e8b19f'),
    )),
    'AFortPawn': ('ACharacter', (
        ('uint8 DamageZoneActiveBitMask;', '255', '0x1422fd743'),
    )),
    'AFortPlayerPawn': ('AFortPawn', (
        ('TEnumAsByte<EFortBuildingState::Type> BuildingState;', 'EFortBuildingState::None', '0x1422ffb6b'),
    )),
    'AFortPlayerController': ('APlayspacePlayerController', (
        ('int32 OverriddenBackpackSize;', '-1', '0x1423661a8'),
    )),
    'AFortPlayerPawnAthena': ('AFortPlayerPawn', (
        ('float ServerWorldTimeRevivalTime;', '-1.0f', '0x141a08f7c'),
    )),
    'AFortPlayerState': ('APlayerState', (
        ('int32 InitialExperienceLevel;', '-1', '0x1423fa6e2'),
        ('int32 InitialExperienceAmount;', '-1', '0x1423fa6e2'),
    )),
    'AFortWeapon': ('AActor', (
        ('int32 WeaponLevel;', '-1', '0x14268de54'),
    )),
}


def finalize(model, header, cpp):
    name = model['name']
    if name not in CLASSES:
        return header, cpp
    parent, fields = CLASSES[name]
    if model.get('parent') != parent:
        raise ValueError(name + ': unexpected parent')
    names = []
    for decl, value, address in fields:
        field = re.search(r'\b(\w+)\s*;$', decl).group(1)
        names.append(field)
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        if len(members) != 1 or members[0].get('decl', '').strip() != decl:
            raise ValueError(name + ': unexpected model declaration for ' + field)
        candidates = re.findall(r'^\s*[^\n;{}]*\b' + re.escape(field) + r'\b[^\n;{}]*;', header, re.MULTILINE)
        if len(candidates) != 1 or candidates[0].strip() != decl:
            raise ValueError(name + ': missing or competing header declaration for ' + field)

    decl_pattern = re.compile(r'^\s*(?:explicit\s+)?' + name + r'\s*\(', re.MULTILINE)
    definitions = list(re.finditer(r'\b' + name + r'::' + name + r'\s*\(', cpp))
    declarations = list(decl_pattern.finditer(header))
    if len(definitions) > 1 or len(declarations) > 1 or bool(definitions) != bool(declarations):
        raise ValueError(name + ': expected at most one matching native constructor')
    if not definitions:
        if header.count('public:') < 1:
            raise ValueError(name + ': missing public declaration section')
        header = header.replace('public:', 'public:\n\t' + name + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n', 1)
        cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n}\n' % (name, name)
    pattern = re.compile(r'\b' + name + r'::' + name + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError(name + ': expected one supported native constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{':
            depth += 1
        elif cpp[end] == '}':
            depth -= 1
        end += 1
    if depth:
        raise ValueError(name + ': unterminated constructor')
    constructor = cpp[match.start():end - 1]
    if any(re.search(r'\b' + re.escape(field) + r'\b', constructor) for field in names):
        raise ValueError(name + ': competing constructor field use; merge explicitly')
    assignments = ''.join('    %s = %s; // Original store %s.\n' % (field, value, address)
                          for field, (_, value, address) in zip(names, fields))
    return header, cpp[:end - 1] + assignments + cpp[end - 1:]
