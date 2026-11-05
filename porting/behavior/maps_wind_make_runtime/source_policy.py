"""Restore const vector input references for the two wind constructors."""

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
