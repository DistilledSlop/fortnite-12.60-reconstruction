"""Restore wind factory signatures and recovered impulse default construction."""

_DEFAULT_GROUPS = {
    'FFortWindImpulseRadius': (0x4c, {
        'FVector': {'Location': 'FVector::ZeroVector'},
        'float': {name: '0.0f' for name in (
            'Radius', 'CurrentRadius', 'PreviousRadius', 'Magnitude',
            'CurrentMagnitude', 'PreviousMagnitude', 'BlendTime', 'CurrentBlendTime')},
        'FBox': {'WorldBounds': 'FBox(ForceInit)'},
    }, 'Handle'),
    'FFortWindImpulseCylinderRadial': (0x3c, {
        'FVector': {'Location': 'FVector::ZeroVector'},
        'float': {name: '0.0f' for name in ('InnerRadius', 'OuterRadius', 'Magnitude')},
        'FBox': {'WorldBounds': 'FBox(ForceInit)'},
        'bool': {'bIsChanging': 'false', 'bIsChangePending': 'false'},
    }, 'Handle'),
    'FFortWindImpulseCylinder': (0x48, {
        'FVector': {'WindDirection': 'FVector::ZeroVector'},
    }, None),
    'FFortWindImpulseCylinderDelta': (0x90, {
        'FVector': {'DeltaCenter': 'FVector::ZeroVector'},
        'bool': {'bInitialized': 'false', 'bRippleOutward': 'true'},
        'float': {'SectionWidth': '5000.0f', **{name: '0.0f' for name in (
            'InnerSectionRadius', 'OuterSectionRadius', 'MaximumRadius',
            'DesiredOverallBlendTime', 'SectionBlendTime', 'SectionCurrentBlendTime',
            'PreviousMagnitude', 'SectionCurrentMagnitude', 'DesiredMagnitude')}},
        'FBox': {name: 'FBox(ForceInit)' for name in (
            'OuterWorldBounds', 'InnerWorldBounds', 'WindImpulseBounds')},
    }, 'WindImpulseHandleToModify'),
}


def apply_struct(info, model):
    contract = _DEFAULT_GROUPS.get(model['cpp_name'])
    if contract is None:
        return model
    size, groups, handle = contract
    expected = {name: (kind, value) for kind, values in groups.items()
                for name, value in values.items()}
    actual = {member['name']: member for member in model['members']}
    if (model['package'] != '/Script/FortniteGame' or model['size'] != size
            or model['align'] != 4
            or set(actual) != set(expected) | ({handle} if handle else set())):
        raise ValueError('wind default-construction layout changed: ' + model['cpp_name'])
    for name, (kind, value) in expected.items():
        if actual[name]['decl'] != kind + ' ' + name + ';':
            raise ValueError('wind default member type changed: ' + name)
        actual[name]['decl'] = kind + ' ' + name + ' = ' + value + ';'
    if handle and actual[handle]['decl'] != 'FFortWindImpulseHandle ' + handle + ';':
        raise ValueError('wind default handle type changed')
    return model


def declarations(model):
    if model['cpp_name'] != 'FFortWindImpulseCylinderDelta':
        return []
    return [
        '\t// Constructing the handle consumes a UID before the original invalid-target reset.',
        '\tFFortWindImpulseCylinderDelta() { WindImpulseHandleToModify.UID = -1; }',
        '',
    ]

CONTRACTS = {
    'MakeWindImpulseRadius': (
        '0x142b526f0',
        [('ReturnValue', 'Struct', '0x10000000000580'),
         ('ImpulseBlendTime', 'Float', '0x10000000000080'),
         ('ImpulseMagnitude', 'Float', '0x10000000000080'),
         ('ImpulseRadius', 'Float', '0x10000000000080'),
         ('ImpulseLocation', 'Struct', '0x10000008000182')],
        ['ImpulseLocation']),
    'MakeWindImpulseCylinder': (
        '0x142b52690',
        [('ReturnValue', 'Struct', '0x10000000000580'),
         ('ImpulseDirection', 'Struct', '0x10000008000182'),
         ('ImpulseMagnitude', 'Float', '0x10000000000080'),
         ('ImpulseOuterRadius', 'Float', '0x10000000000080'),
         ('ImpulseInnerRadius', 'Float', '0x10000000000080'),
         ('ImpulseLocation', 'Struct', '0x10000008000182')],
        ['ImpulseLocation', 'ImpulseDirection']),
}


def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'AFortWindManager':
        return
    for fn in functions:
        contract = CONTRACTS.get(fn['name'])
        if contract is None:
            continue
        create, expected_params, inputs = contract
        declared = [f for f in block['functions'] if f['name'] == fn['name']]
        if len(declared) != 1 or declared[0]['create_func'] != create:
            raise ValueError('wind make function identity changed: ' + fn['name'])
        flags, params, problem = function_view(declared[0], owner_fns)
        if problem or params is None:
            raise ValueError('wind make parameter table changed: ' + fn['name'])
        actual = [(p['name'], p['kind'], p['property_flags']) for p in params]
        if flags != 0x14c22401 or sorted(actual) != sorted(expected_params):
            raise ValueError('wind make parameter contract changed: ' + fn['name'])
        for input_name in inputs:
            old = 'FVector& ' + input_name
            if fn['args'].count(old) != 1:
                raise ValueError('wind make vector signature changed: ' + fn['name'])
            index = fn['args'].index(old)
            fn['args'][index] = 'const ' + old
