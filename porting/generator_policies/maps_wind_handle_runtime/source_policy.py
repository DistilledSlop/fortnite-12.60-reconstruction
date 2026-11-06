"""Restore the shared default-construction sequence for native wind handles."""


def _validate(model):
    if model['cpp_name'] != 'FFortWindImpulseHandle':
        return False
    members = [(m['offset'], m['decl']) for m in model['members']]
    if (model['package'] != '/Script/FortniteGame' or model['size'] != 4
            or model['align'] != 4 or members != [(0, 'int32 UID;')]):
        raise ValueError('FFortWindImpulseHandle reflected identity changed')
    return True


def preamble(model):
    if not _validate(model):
        return []
    return [
        '// One exported allocator shares the original counter across module boundaries.',
        'namespace FortWindNative',
        '{',
        '    FORTNITEGAME_API uint32 AllocateImpulseUID();',
        '}',
        '',
    ]


def apply_struct(info, model):
    if _validate(model):
        # The original Blueprint-callable query takes this struct as input.
        model['blueprint_type'] = True
    return model


def declarations(model):
    if not _validate(model):
        return []
    return [
        '\t// The original default constructor consumes a UID even for exec temporaries.',
        '\tFFortWindImpulseHandle() : UID(static_cast<int32>(FortWindNative::AllocateImpulseUID())) {}',
        '',
    ]


def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'AFortWindManager':
        return
    contracts = {
        'HasWindImpulse': ('0x142b525a0', 0x54420401,
                           'WindImpulseHandle', '0x10000008000182',
                           'FFortWindImpulseHandle& WindImpulseHandle',
                           'const FFortWindImpulseHandle& WindImpulseHandle'),
        'GetWindManager': ('0x142b52570', 0x14022401,
                           'WorldContextObject', '0x10000000000082',
                           'UObject* WorldContextObject',
                           'const UObject* WorldContextObject'),
    }
    for fn in functions:
        contract = contracts.get(fn['name'])
        if contract is None:
            continue
        create, expected_flags, param_name, param_flags, old, new = contract
        declared = [f for f in block['functions'] if f['name'] == fn['name']]
        if len(declared) != 1 or declared[0]['create_func'] != create:
            raise ValueError('wind query function identity changed: ' + fn['name'])
        flags, params, problem = function_view(declared[0], owner_fns)
        inputs = [p for p in params if p['name'] == param_name]
        if (problem or flags != expected_flags or len(inputs) != 1
                or inputs[0]['property_flags'] != param_flags or fn['args'] != [old]):
            raise ValueError('wind query input contract changed: ' + fn['name'])
        # Restore the const input qualifier retained in the property flags.
        fn['args'] = [new]
