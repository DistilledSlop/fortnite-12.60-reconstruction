"""Isolated derived pawn-stat row proposal; do not activate until integrated with base row."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACTS = {
    'FFortAIPawnStats': ('0x1427a41c0', 256, '0x1462b5350', True, '6e71e4f604c1f4b58e9f4d34f9f5b68b3a589eeecb5ae0c61cedfd9acfd17a49'),
    'FFortPlayerPawnStats': ('0x142a7a760', 200, '0x1464cb4a8', False, 'b9d070aaa1481dbc0583976e703764ea38aeff22cabe47075ff1e9a2b25de9d8'),
}
FIELDS = {name: json.loads((HERE / (name[1:] + '.json')).read_text()) for name in CONTRACTS}


def validate(info):
    fn, size, vtable, post, digest = CONTRACTS[info.cpp_name]
    rec = info.rec
    ops = rec.get('struct_ops') or {}
    traits = ops.get('traits') or {}
    actual = hashlib.sha256(json.dumps(rec['properties'], sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    if (info.kind != 'struct' or info.fn != fn or info.package != '/Script/FortniteGame'
            or rec.get('size') != size or rec.get('align') != 8 or rec.get('super_fn') != '0x142a1ade0'
            or ops.get('vtable_va') != vtable or ops.get('ops_size') != size or ops.get('ops_align') != 8
            or traits.get('HasPostSerialize') is not post or traits.get('HasDestructor') is not True
            or traits.get('HasCopy') is not True or traits.get('HasSerializer') is not False
            or traits.get('HasStructuredSerializer') is not False or actual != digest):
        raise ValueError(info.cpp_name + ' pinned identity, properties or struct ops changed')


def add_roots(gate, seeds):
    for name, (fn, *_rest) in CONTRACTS.items():
        info = gate.info(fn)
        if info is None:
            raise ValueError('missing ' + name)
        validate(info)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError(name + ' is not emittable: ' + reason)
        seeds.setdefault(fn, set()).add('')


def apply_struct(info, model):
    if info.cpp_name not in CONTRACTS:
        return model
    validate(info)
    if model['parent'] != 'FFortPawnStats' or model.get('net') is not None:
        raise ValueError(info.cpp_name + ' parent or serializer changed')
    fields = FIELDS[info.cpp_name]
    if [m['name'] for m in model['members']] != [f[0] for f in fields]:
        raise ValueError(info.cpp_name + ' fields missing or reordered')
    for m, (name, ctype, offset) in zip(model['members'], fields):
        if m['ctype'] != ctype or m['decl'] != '%s %s;' % (ctype, name) or m.get('bitfield') or m['offset'] != offset:
            raise ValueError(info.cpp_name + ' field changed: ' + name)
    model['includes'] = list(dict.fromkeys(model.get('includes', []) + struct_includes(model)))
    return model


def struct_includes(model):
    if model['cpp_name'] not in CONTRACTS:
        return []
    includes = ['FFortPawnStats.h']
    if model['cpp_name'] == 'FFortPlayerPawnStats':
        includes += ['AFortPawn.h', 'UFortAbilitySystemComponent.h', 'GameplayTagContainer.h', 'FortNativeGameplayTagsRuntime.h']
    return includes


def declarations(model):
    name = model['cpp_name']
    if name not in CONTRACTS:
        return []
    if name == 'FFortAIPawnStats':
        defaults = {field: ('nullptr' if ctype.endswith('*') else 'NAME_None' if ctype == 'FName' else '0') for field, ctype, _ in FIELDS[name]}
    else:
        defaults = dict(MaxJumpTime='0.3f', MaxStamina='100.0f', StaminaRegenRate='15.0f', StaminaRegenDelay='1.0f', SprintingStaminaExpenditureRate='20.0f', PersonalVehicleFallingDamageTableRow='NAME_None')
    lines = ['\t// Original struct-ops constructor: ' + ('0x1427a0680' if name == 'FFortAIPawnStats' else '0x142a756e0'),
             '\t' + name + '()', '\t\t: ' + ',\n\t\t  '.join('%s(%s)' % (field, defaults[field]) for field, _, _ in FIELDS[name])]
    if name == 'FFortAIPawnStats':
        return lines + ['\t{}', '']
    lines += ['\t{', '\t\tHealthRegenRate = 10.0f;', '\t\tHealthRegenDelay = 5.0f;', '\t\tHealthRegenThreshold = 0.3f;',
              '\t\tShieldRegenRate = 10.0f;', '\t\tShieldRegenDelay = 5.0f;', '\t\tShieldRegenThreshold = 1.0f;', '\t}', '']
    for signature, filename in [
        ('float NativeFallingDamage_14231b6e0(const AFortPawn* Pawn, float Value) const override', 'PlayerFallingDamage.inc'),
        ('float NativeVehicleEjectDamage_1406964e0(const AFortPawn* Pawn, float Value) const override', 'PlayerVehicleEjectDamage.inc')]:
        lines += ['\t' + signature, '\t{'] + ['\t\t' + s if s else '' for s in (HERE / filename).read_text().splitlines()] + ['\t}', '']
    return lines


def footer(model):
    name = model['cpp_name']
    if name not in CONTRACTS:
        return []
    return ['template<> struct TStructOpsTypeTraits<%s> : public TStructOpsTypeTraitsBase2<%s>' % (name, name),
            '{ enum { WithPostSerialize = %s, WithCopy = true }; };' % ('true' if CONTRACTS[name][3] else 'false'),
            'static_assert(sizeof(%s) == %d, "pawn stats row size drift");' % (name, CONTRACTS[name][1]),
            'static_assert(alignof(%s) == 8, "pawn stats row alignment drift");' % name] + [
            'static_assert(STRUCT_OFFSET(%s, %s) == %d, "pawn stats field drift");' % (name, field, offset)
            for field, _, offset in FIELDS[name]]
