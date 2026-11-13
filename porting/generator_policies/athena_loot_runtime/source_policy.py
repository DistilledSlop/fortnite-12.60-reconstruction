"""Pinned 12.60 loot table rows, including native-byte bools and constructors."""
import hashlib
import json
from pathlib import Path

CONTRACTS = json.loads(Path(__file__).with_name('struct-contracts.json').read_text())
# (member, emitted C++ type, original offset). Bool offsets come from setters,
# not the null offsets in the immutable property records.
FIELDS = {
    'FFortLootLevelData': [
        ('Category', 'FName', 8), ('LootLevel', 'int32', 16),
        ('MinItemLevel', 'int32', 20), ('MaxItemLevel', 'int32', 24),
    ],
    'FFortLootPackageData': [
        ('LootPackageID', 'FName', 8), ('Weight', 'float', 16),
        ('NamedWeightMult', 'FName', 20), ('PotentialNamedWeights', 'TArray<FName>', 32),
        ('Count', 'int32', 48), ('LootPackageCategory', 'int32', 52),
        ('GameplayTags', 'FGameplayTagContainer', 56), ('RequiredTag', 'FName', 88),
        ('LootPackageCall', 'FString', 96), ('ItemDefinition', 'TSoftObjectPtr<UFortItemDefinition>', 112),
        ('PersistentLevel', 'FString', 152), ('MinWorldLevel', 'int32', 168),
        ('MaxWorldLevel', 'int32', 172), ('bAllowBonusDrops', 'bool', 176),
        ('Annotation', 'FString', 184),
    ],
    'FFortLootTierData': [
        ('TierGroup', 'FName', 8), ('Weight', 'float', 16),
        ('QuotaLevel', 'TEnumAsByte<ELootQuotaLevel::Type>', 20), ('LootTier', 'int32', 24),
        ('MinWorldLevel', 'int32', 28), ('MaxWorldLevel', 'int32', 32),
        ('StreakBreakerCurrency', 'FString', 40), ('StreakBreakerPointsMin', 'int32', 56),
        ('StreakBreakerPointsMax', 'int32', 60), ('StreakBreakerPointsSpend', 'int32', 64),
        ('LootPackage', 'FName', 68), ('LootPreviewPackage', 'FName', 76),
        ('NumLootPackageDrops', 'float', 84), ('LootPackageCategoryWeightArray', 'TArray<int32>', 88),
        ('LootPackageCategoryMinArray', 'TArray<int32>', 104), ('LootPackageCategoryMaxArray', 'TArray<int32>', 120),
        ('GameplayTags', 'FGameplayTagContainer', 136), ('RequiredGameplayTags', 'FGameplayTagContainer', 168),
        ('bAllowBonusLootDrops', 'bool', 200), ('Annotation', 'FString', 208),
    ],
}
INITIALIZERS = {
    # Struct ops constructor 0x142989390 writes -1 at offsets 16, 20, 24.
    'FFortLootLevelData': [('LootLevel', '-1'), ('MinItemLevel', '-1'), ('MaxItemLevel', '-1')],
    'FFortLootPackageData': [('Weight', '0.0f'), ('Count', '0'), ('LootPackageCategory', '-1'),
                             ('MinWorldLevel', '-1'), ('MaxWorldLevel', '-1'), ('bAllowBonusDrops', 'false')],
    'FFortLootTierData': [('Weight', '0.0f'), ('QuotaLevel', 'static_cast<ELootQuotaLevel::Type>(0)'),
                          ('LootTier', '0'), ('MinWorldLevel', '-1'), ('MaxWorldLevel', '-1'),
                          ('StreakBreakerPointsMin', '0'), ('StreakBreakerPointsMax', '0'),
                          ('StreakBreakerPointsSpend', '0'), ('NumLootPackageDrops', '0.0f'),
                          ('bAllowBonusLootDrops', 'false')],
}

def validate(info, expected):
    rec = info.rec
    digest = hashlib.sha256(json.dumps(rec['properties'], sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    ops = rec.get('struct_ops') or {}
    traits = ops.get('traits') or {}
    if (info.kind != 'struct' or info.fn != expected['fn'] or info.package != expected['package']
            or rec.get('size') != expected['size'] or rec.get('align') != expected['align']
            or rec.get('super_fn') != expected['super_fn'] or ops.get('vtable_va') != expected['ops_vtable']
            or ops.get('ops_size') != expected['size'] or ops.get('ops_align') != expected['align']
            or traits.get('HasSerializer') is not False or traits.get('HasStructuredSerializer') is not False
            or traits.get('HasDestructor') is not True or traits.get('HasCopy') is not True
            or digest != expected['properties_sha256']):
        raise ValueError('loot row identity, ops or properties changed: ' + info.cpp_name)

def add_roots(gate, seeds):
    for name, expected in sorted(CONTRACTS.items()):
        info = gate.info(expected['fn'])
        if info is None or info.cpp_name != name:
            raise ValueError('missing loot row: ' + name)
        validate(info, expected)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('unemittable loot row %s: %s' % (name, reason))
        seeds.setdefault(info.fn, set()).add('')

def apply_struct(info, model):
    expected = CONTRACTS.get(info.cpp_name)
    if expected is None:
        return model
    validate(info, expected)
    if model['parent'] != 'FTableRowBase' or model.get('net') is not None:
        raise ValueError('loot row parent or serializer changed: ' + info.cpp_name)
    members = model['members']
    fields = FIELDS[info.cpp_name]
    if [m['name'] for m in members] != [name for name, _, _ in fields]:
        raise ValueError('loot row members missing or reordered: ' + info.cpp_name)
    for m, (name, ctype, offset) in zip(members, fields):
        if ctype == 'bool':
            if (m['kind'] != 'Bool' or m['bitfield'] or m['ctype'] != 'bool'
                    or m['decl'] != 'bool %s;' % name or m['offset'] is not None):
                raise ValueError('loot bool record changed: ' + info.cpp_name + '.' + name)
            m.update(ctype='bool', decl='bool %s;' % name, bitfield=False, offset=offset)
        elif m['ctype'] != ctype or m['offset'] != offset or m['decl'] != '%s %s;' % (ctype, name):
            raise ValueError('loot field type or offset changed: ' + info.cpp_name + '.' + name)
    return model

def declarations(model):
    name = model['cpp_name']
    if name not in INITIALIZERS:
        return []
    initializers = INITIALIZERS[name]
    return ['\t%s()' % name, '\t\t: ' + ',\n\t\t  '.join('%s(%s)' % x for x in initializers),
            '\t{}', '']

def footer(model):
    name = model['cpp_name']
    if name not in FIELDS:
        return []
    expected = CONTRACTS[name]
    return (['static_assert(sizeof(%s) == %d, "loot row size drift");' % (name, expected['size']),
             'static_assert(alignof(%s) == %d, "loot row alignment drift");' % (name, expected['align'])]
            + ['static_assert(STRUCT_OFFSET(%s, %s) == %d, "loot row field offset drift");' % (name, field, offset)
               for field, _, offset in FIELDS[name]] + [''])
