"""Guarded source declarations needed by the 2,040-body checkpoint."""


def apply(model):
    if model.get('name') != 'AFortWeapon':
        return model
    if model.get('block_va', '').lower() != '0x14656a020' or model.get('package') != '/Script/FortniteGame':
        raise ValueError('original AFortWeapon identity differs')
    matches = [f for f in model.get('functions', []) if f.get('name') == 'SetWeaponPawnAnimsetOverride']
    if len(matches) != 1:
        raise ValueError('original SetWeaponPawnAnimsetOverride function missing or ambiguous')
    f = matches[0]
    body = ('// Original exec 0x142b48be0; pure writer 0x1426b3b10.\n'
            '// CONFIDENCE: HIGH\n'
            'this->WeaponPawnAnimsetOverride = WeaponPawnAnimsetOverride;')
    if (f.get('ret') != 'void' or f.get('args') != ['UFortWeaponAnimSet* WeaponPawnAnimsetOverride']
            or f.get('original_flags') != 0x4080401 or f.get('static') or f.get('const')
            or not f.get('splice') or not f.get('accepted') or f.get('body') != body):
        raise ValueError('original SetWeaponPawnAnimsetOverride signature or body differs')
    f['args'] = ['UPARAM(meta=(OverrideNativeName="WeaponPawnAnimsetOverride")) UFortWeaponAnimSet* InWeaponPawnAnimsetOverride']
    f['body'] = body.replace('= WeaponPawnAnimsetOverride;', '= InWeaponPawnAnimsetOverride;', 1)
    return model


def apply_struct(info, model):
    if info.cpp_name != 'FBuildingDuplicationData' and info.fn != '0x142755680':
        return model
    rec = info.rec
    if (info.kind != 'struct' or info.fn != '0x142755680' or info.package != '/Script/FortniteGame'
            or info.cpp_name != 'FBuildingDuplicationData' or rec.get('name') != 'BuildingDuplicationData'
            or rec.get('size') != 40 or rec.get('align') != 8 or rec.get('super_fn') is not None):
        raise ValueError('original BuildingDuplicationData identity or layout differs')
    expected = [
        (0, 'TextureData', 8, 'Object', '0x12', 4, '0x10000000000001', {'class': '0x142759360'}),
        (1, 'ClassData', 0, 'Class', '0x11', 1, '0x10000000000005',
         {'meta_class': '0x142eea520', 'class': '0x142e497d0'}),
    ]
    props = rec.get('properties', [])
    if len(props) != len(expected):
        raise ValueError('original BuildingDuplicationData property count differs')
    for prop, (index, name, offset, kind, raw, dim, flags, refs) in zip(props, expected):
        if (prop.get('index') != index or prop.get('name') != name or prop.get('offset') != offset
                or prop.get('kind') != kind or prop.get('kind_raw') != raw or prop.get('array_dim') != dim
                or prop.get('property_flags') != flags or prop.get('type_refs') != refs
                or prop.get('object_flags') != '0x45' or prop.get('replicated') is not False):
            raise ValueError('original BuildingDuplicationData property metadata differs: ' + name)
    model['blueprint_type'] = True
    return model
