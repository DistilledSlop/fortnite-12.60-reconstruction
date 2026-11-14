"""Complete pinned FFortPawnStats row; isolated proposal pending root integration."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'struct-contract.json').read_text())
FIELDS = json.loads((HERE / 'fields.json').read_text())
INCLUDES = ['Engine/DataTable.h', 'Engine/CurveTable.h', 'Curves/RealCurve.h',
            'Serialization/Archive.h', 'EFortControlRecoveryBehavior.h', 'SharedFortGameLog.h']


def validate(info):
    rec = info.rec
    digest = hashlib.sha256(json.dumps(rec['properties'], sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    ops = rec.get('struct_ops') or {}
    traits = ops.get('traits') or {}
    if (info.cpp_name != 'FFortPawnStats' or info.kind != 'struct'
            or info.fn != CONTRACT['fn'] or info.package != CONTRACT['package']
            or rec.get('size') != CONTRACT['size'] or rec.get('align') != CONTRACT['align']
            or rec.get('super_fn') != CONTRACT['super_fn']
            or ops.get('vtable_va') != CONTRACT['ops_vtable']
            or ops.get('ops_size') != 168 or ops.get('ops_align') != 8
            or traits.get('HasPostSerialize') is not True
            or traits.get('HasDestructor') is not True or traits.get('HasCopy') is not True
            or traits.get('HasSerializer') is not False
            or traits.get('HasStructuredSerializer') is not False
            or digest != CONTRACT['properties_sha256']):
        raise ValueError('FFortPawnStats pinned identity, properties or struct ops changed')


def add_roots(gate, seeds):
    info = gate.info(CONTRACT['fn'])
    if info is None:
        raise ValueError('missing FFortPawnStats reflection record')
    validate(info)
    reason = gate.reason(info)
    if reason is not None:
        raise ValueError('FFortPawnStats is not emittable: ' + reason)
    seeds.setdefault(info.fn, set()).add('')


def apply_struct(info, model):
    if info.cpp_name != 'FFortPawnStats':
        return model
    validate(info)
    if model['parent'] != 'FTableRowBase' or model.get('net') is not None:
        raise ValueError('FFortPawnStats parent or serializer changed')
    if [m['name'] for m in model['members']] != [f[0] for f in FIELDS]:
        raise ValueError('FFortPawnStats fields missing or reordered')
    for m, (name, ctype, offset) in zip(model['members'], FIELDS):
        if m['ctype'] != ctype or m['decl'] != '%s %s;' % (ctype, name) or m.get('bitfield'):
            raise ValueError('FFortPawnStats field type changed: ' + name)
        if name == 'bAllowChainStun':
            if m['offset'] is not None or m['kind'] != 'Bool':
                raise ValueError('FFortPawnStats native bool metadata changed')
            m['offset'] = offset  # setter 1407462a0 is MOV byte [RCX+a4],1.
        elif m['offset'] != offset:
            raise ValueError('FFortPawnStats field offset changed: ' + name)
    model['includes'] = list(dict.fromkeys(model.get('includes', []) + INCLUDES))
    model['forward_decls'] = sorted(set(model['forward_decls']) | {'AFortPawn'})
    return model


def _body(filename):
    return ['\t\t' + s if s else '' for s in (HERE / filename).read_text().splitlines()]


def declarations(model):
    if model['cpp_name'] != 'FFortPawnStats':
        return []
    defaults = {name: '0.0f' for name, ctype, _ in FIELDS if ctype == 'float'}
    defaults.update(MaximumHealth='100.0f', SpeedBackwardsMultiplier='1.0f',
                    AccelerationStrafeMultiplierSprint='1.0f',
                    ControlResistanceRegenThreshold='1.0f', KnockbackMultiplier='1.0f')
    for name in ('SpeedWalk', 'SpeedRun', 'SpeedSprint', 'SpeedFly', 'SpeedCrouchedRun', 'SpeedCrouchedSprint'):
        defaults[name] = '10.0f'
    defaults.update(FallingDamageTable='nullptr', FallingDamageTableRow='NAME_None',
                    VehicleEjectDamageTable='nullptr', bAllowChainStun='false',
                    ControlRecoveryBehavior='EFortControlRecoveryBehavior::DefaultControl')
    lines = ['\t// 0x1423002f0 / native struct-ops constructor 0x142a16dd0.',
             '\tFFortPawnStats()', '\t\t: ' + ',\n\t\t  '.join('%s(%s)' % (name, defaults[name]) for name, _, _ in FIELDS),
             '\t{}', '']
    for signature, filename in [
        ('void PostSerialize(const FArchive& Ar)', 'PostSerialize.inc'),
        ('virtual float NativeFallingDamage_14231b6e0(const AFortPawn* Pawn, float Value) const', 'NativeFallingDamage_14231b6e0.inc'),
        ('virtual float NativeFallingThreshold_142321b50() const', 'NativeFallingThreshold_142321b50.inc')]:
        lines += ['\t' + signature, '\t{'] + _body(filename) + ['\t}', '']
    # Full row vtable also has slot +0x20. Its original target is XORPS XMM0,XMM0; RET.
    lines += ['\tvirtual float NativeVehicleEjectDamage_1406964e0(const AFortPawn* Pawn, float Value) const',
              '\t{', '\t\treturn 0.0f;', '\t}', '']
    return lines


def footer(model):
    if model['cpp_name'] != 'FFortPawnStats':
        return []
    return [
        'template<> struct TStructOpsTypeTraits<FFortPawnStats> : public TStructOpsTypeTraitsBase2<FFortPawnStats>',
        '{ enum { WithPostSerialize = true, WithCopy = true }; };',
        'static_assert(sizeof(FFortPawnStats) == 168, "pawn stats row size drift");',
        'static_assert(alignof(FFortPawnStats) == 8, "pawn stats row alignment drift");',
    ] + ['static_assert(STRUCT_OFFSET(FFortPawnStats, %s) == %d, "pawn stats field drift");' % (name, offset)
         for name, _, offset in FIELDS]


def render_isolated(info):
    """Render for source review without mutating the global generator or native tree."""
    validate(info)
    lines = ['// Isolated generator proposal. Generated from pinned struct_blocks.json.',
             '#pragma once', '#include "CoreMinimal.h"']
    lines += ['#include "%s"' % p for p in INCLUDES]
    lines += ['class AFortPawn;', '#include "FFortPawnStats.generated.h"', '',
              'USTRUCT()', 'struct FORTNITEGAME_API FFortPawnStats : public FTableRowBase',
              '{', '\tGENERATED_BODY()', '']
    for name, ctype, offset in FIELDS:
        lines += ['\t// Original offset 0x%x.' % offset, '\tUPROPERTY()', '\t%s %s;' % (ctype, name), '']
    lines += declarations({'cpp_name': 'FFortPawnStats'}) + ['};', '']
    lines += footer({'cpp_name': 'FFortPawnStats'})
    return '\n'.join(lines) + '\n'
