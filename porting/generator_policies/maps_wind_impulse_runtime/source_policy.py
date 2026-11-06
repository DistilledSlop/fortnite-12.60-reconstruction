"""Preserve original read-only impulse arguments in Blueprint calls."""

CONTRACTS = {
    'AddWindImpulse': ('0x142b523c0',
        [('ReturnValue', 'Struct', '0x10000000000580'),
         ('WindImpulse', 'Struct', '0x10000008000182')],
        ['FFortWindImpulseRadius& WindImpulse']),
    'RemoveWindImpulse': ('0x142b52940',
        [('BlendTime', 'Float', '0x10000000000080'),
         ('ImpulseHandle', 'Struct', '0x10000008000182')],
        ['FFortWindImpulseHandle& ImpulseHandle', 'float BlendTime']),
    'SetWindImpulse': ('0x142b529a0',
        [('ReturnValue', 'Bool', '0x10000000000580'),
         ('WindImpulse', 'Struct', '0x10000008000182'),
         ('ImpulseHandle', 'Struct', '0x10000008000182')],
        ['FFortWindImpulseHandle& ImpulseHandle', 'FFortWindImpulseRadius& WindImpulse']),
}


def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'AFortWindManager':
        return
    for fn in functions:
        contract = CONTRACTS.get(fn['name'])
        if contract is None:
            continue
        create, expected, args = contract
        declared = [f for f in block['functions'] if f['name'] == fn['name']]
        if len(declared) != 1 or declared[0]['create_func'] != create:
            raise ValueError('wind impulse function identity changed: ' + fn['name'])
        flags, params, problem = function_view(declared[0], owner_fns)
        if problem or params is None:
            raise ValueError('wind impulse parameter table changed: ' + fn['name'])
        actual = [(p['name'], p['kind'], p['property_flags']) for p in params]
        if flags != 0x4420401 or sorted(actual) != sorted(expected) or fn['args'] != args:
            raise ValueError('wind impulse signature changed: ' + fn['name'])
        fn['args'] = ['const ' + arg if arg.startswith('FFort') else arg for arg in args]
