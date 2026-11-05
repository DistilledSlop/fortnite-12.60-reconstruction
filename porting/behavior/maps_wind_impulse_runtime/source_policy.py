"""Preserve the original read-only impulse argument in Blueprint calls."""


def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'AFortWindManager':
        return
    for fn in functions:
        if fn['name'] != 'AddWindImpulse':
            continue
        declared = [f for f in block['functions'] if f['name'] == fn['name']]
        if len(declared) != 1 or declared[0]['create_func'] != '0x142b523c0':
            raise ValueError('wind impulse addition identity changed')
        flags, params, problem = function_view(declared[0], owner_fns)
        actual = [(p['name'], p['kind'], p['property_flags']) for p in params]
        expected = [('ReturnValue', 'Struct', '0x10000000000580'),
                    ('WindImpulse', 'Struct', '0x10000008000182')]
        if (problem or flags != 0x4420401 or sorted(actual) != sorted(expected)
                or fn['args'] != ['FFortWindImpulseRadius& WindImpulse']):
            raise ValueError('wind impulse addition signature changed')
        fn['args'] = ['const FFortWindImpulseRadius& WindImpulse']
