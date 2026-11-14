"""Synthetic access to the engine's protected ability handle; no original body claim."""
import re

CLASS = 'UFortGameplayAbility'
METHOD = 'NativeCurrentSpecHandle'
DECL = 'FGameplayAbilitySpecHandle NativeCurrentSpecHandle() const { return CurrentSpecHandle; }'


def validate_required_classes(names):
    names = set(names)
    if 'AFortWeapon' in names and CLASS not in names:
        raise ValueError('AFortWeapon ability handle bridge requires ' + CLASS)


def declarations(model):
    if model.get('name') != CLASS:
        return []
    if (model.get('block_va', '').lower(), model.get('package'), model.get('parent')) != (
            '0x1463d65a0', '/Script/FortniteGame', 'UGameplayAbility'):
        raise ValueError(CLASS + ': identity or ancestry changed')
    for key in ('functions', 'helpers', 'static_helpers', 'fields', 'members_decl_order'):
        if any(item.get('name') == METHOD for item in model.get(key, [])):
            raise ValueError(CLASS + ': synthetic bridge name collides with ' + key)
    tag = CLASS + '::' + METHOD
    splice = model.get('splice') or {}
    if tag in splice.get('accepted', []) or any(r.get('entry') == tag for r in splice.get('rejected', [])):
        raise ValueError(tag + ': synthetic bridge must not be a recovered snippet')
    return ['public:', '\t// Synthetic access bridge for the inherited engine field.', '\t' + DECL]


def validate_rendered(model, header, cpp):
    if model.get('name') != CLASS:
        return
    declarations(model)  # repeat identity and collision checks against the final model
    lines = header.splitlines()
    indices = [i for i, line in enumerate(lines) if line.strip() == DECL]
    if len(indices) != 1 or len(re.findall(r'\b' + METHOD + r'\s*\(', header)) != 1:
        raise ValueError(CLASS + ': synthetic bridge missing, altered, or duplicated')
    access = next((lines[i].strip() for i in range(indices[0] - 1, -1, -1)
                   if lines[i].strip() in ('public:', 'protected:', 'private:')), None)
    if access != 'public:' or re.search(r'\b' + CLASS + r'::' + METHOD + r'\s*\(', cpp):
        raise ValueError(CLASS + ': synthetic bridge access/definition changed')
