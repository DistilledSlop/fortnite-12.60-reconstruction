"""Pinned complete FFortGameplayAttributeData constructor/setters and native layout."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'struct-contract.json').read_text())
FIELDS = json.loads((HERE / 'fields.json').read_text())


def struct_includes(model):
    return ['xmmintrin.h'] if model['cpp_name'] == 'FFortGameplayAttributeData' else []


def validate(info):
    digest = hashlib.sha256(json.dumps(info.rec, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    if (info.cpp_name != 'FFortGameplayAttributeData' or info.kind != 'struct'
            or info.fn != CONTRACT['fn'] or info.package != CONTRACT['package']
            or digest != CONTRACT['record_sha256']):
        raise ValueError('FFortGameplayAttributeData original identity, fields or struct ops changed')


def add_roots(gate, seeds):
    info = gate.info(CONTRACT['fn'])
    if info is None:
        raise ValueError('missing FFortGameplayAttributeData reflection record')
    validate(info)
    reason = gate.reason(info)
    if reason is not None:
        raise ValueError('FFortGameplayAttributeData is not emittable: ' + reason)
    seeds.setdefault(info.fn, set()).add('')


def apply_struct(info, model):
    if info.cpp_name != 'FFortGameplayAttributeData':
        return model
    validate(info)
    if (model['parent'] != 'FGameplayAttributeData' or model.get('net') is not None
            or model.get('size') != 40 or model.get('align') != 8):
        raise ValueError('FFortGameplayAttributeData parent/layout/serialization changed')
    if [m['name'] for m in model['members']] != [f[0] for f in FIELDS]:
        raise ValueError('FFortGameplayAttributeData fields missing or reordered')
    for member, (name, ctype, offset) in zip(model['members'], FIELDS):
        if (member['ctype'] != ctype or member['decl'] != '%s %s;' % (ctype, name)
                or member.get('bitfield') or member['kind'] != ('Bool' if ctype == 'bool' else 'Float')):
            raise ValueError('FFortGameplayAttributeData native field type changed: ' + name)
        if ctype == 'bool':
            if member['offset'] is not None:
                raise ValueError('FFortGameplayAttributeData native bool metadata changed: ' + name)
            # SetBitFuncs are MOV byte [RCX+18/19/1a],1, not packed bools.
            member['offset'] = offset
        elif member['offset'] != offset:
            raise ValueError('FFortGameplayAttributeData native float offset changed: ' + name)
    model['includes'] = list(dict.fromkeys(model.get('includes', []) + ['xmmintrin.h']))
    return model


def declarations(model):
    if model['cpp_name'] != 'FFortGameplayAttributeData':
        return []
    lines = [
        '\t// Native struct-ops constructor 0x142840060.',
        '\tFFortGameplayAttributeData()',
        '\t\t: FGameplayAttributeData(), Minimum(-MAX_flt), Maximum(MAX_flt),',
        '\t\t  bIsCurrentClamped(false), bIsBaseClamped(false), bShouldClampBase(false),',
        '\t\t  UnclampedBaseValue(0.0f), UnclampedCurrentValue(0.0f)',
        '\t{}', '',
    ]
    for name in ('SetCurrentValue', 'SetBaseValue'):
        lines += ['\tvirtual void %s(float NewValue) override' % name, '\t{']
        lines += ['\t\t' + s if s else '' for s in (HERE / (name + '.inc')).read_text().splitlines()]
        lines += ['\t}', '']
    lines += ['\t' + line if line else '' for line in (HERE.parent / 'attribute_limit_runtime' / 'AttributeLimitMethods.inc').read_text().splitlines()]
    lines += [
        'private:',
        '\t// Bit tests preserve unordered handling even when target compiler assumes finite comparisons.',
        '\tstatic bool NativeIsNaN(float Value)',
        '\t{',
        '\t\tuint32 Bits;',
        '\t\tFMemory::Memcpy(&Bits, &Value, sizeof(Bits));',
        '\t\treturn (Bits & 0x7fffffffu) > 0x7f800000u;',
        '\t}',
        '\tstatic bool NativeOrderedDifferent(float Left, float Right)',
        '\t{',
        '\t\treturn !NativeIsNaN(Left) && !NativeIsNaN(Right) && Left != Right;',
        '\t}',
        '\tfloat NativeClampValue(float Value) const',
        '\t{',
        '\t\t// COMISS/JB selects Minimum for a NaN lower bound.',
        '\t\tif (NativeIsNaN(Minimum) || Value < Minimum)',
        '\t\t{',
        '\t\t\treturn Minimum;',
        '\t\t}',
        '\t\t// MINSS(Maximum, Value): Value wins equality (including signed zero) and NaN.',
        '\t\treturn _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(Maximum), _mm_set_ss(Value)));',
        '\t}',
        'public:', '',
    ]
    return lines


def footer(model):
    if model['cpp_name'] != 'FFortGameplayAttributeData':
        return []
    return [
        'template<> struct TStructOpsTypeTraits<FFortGameplayAttributeData> : public TStructOpsTypeTraitsBase2<FFortGameplayAttributeData>',
        '{ enum { WithCopy = true }; };',
        'static_assert(sizeof(FFortGameplayAttributeData) == 40, "attribute data size drift");',
        'static_assert(alignof(FFortGameplayAttributeData) == 8, "attribute data alignment drift");',
    ] + ['static_assert(STRUCT_OFFSET(FFortGameplayAttributeData, %s) == %d, "attribute data field drift");' % (name, offset)
         for name, _, offset in FIELDS]


def render_isolated(info):
    validate(info)
    lines = ['// Isolated proposal; integrate through gen_native_classes.py.', '#pragma once',
             '#include "CoreMinimal.h"', '#include "AttributeSet.h"', '#include "xmmintrin.h"',
             '#include "FFortGameplayAttributeData.generated.h"', '', 'USTRUCT()',
             'struct FORTNITEGAME_API FFortGameplayAttributeData : public FGameplayAttributeData', '{', '\tGENERATED_BODY()', '']
    for name, ctype, offset in FIELDS:
        lines += ['\t// Original offset 0x%x.' % offset, '\tUPROPERTY()', '\t%s %s;' % (ctype, name), '']
    return '\n'.join(lines + declarations({'cpp_name': 'FFortGameplayAttributeData'}) + ['};', ''] + footer({'cpp_name': 'FFortGameplayAttributeData'})) + '\n'
