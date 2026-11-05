"""Restore reflected const input references on the four wind break functions."""


def apply_struct(info, model):
    sizes = {'FFortWindImpulseRadius': 0x4c,
             'FFortWindImpulseCylinder': 0x48,
             'FFortWindImpulseCylinderDelta': 0x90}
    expected = sizes.get(model['cpp_name'])
    if expected is not None:
        if (model['package'] != '/Script/FortniteGame' or model['size'] != expected
                or model['align'] != 4):
            raise ValueError('wind break struct identity changed: ' + model['cpp_name'])
        # Each is a parameter of the original Blueprint-callable break functions.
        model['blueprint_type'] = True
    return model

CONTRACTS = {'BreakWindImpulseRadius': ('0x142b524e0',
                            348267521,
                            'InWindImpulseRadius',
                            'FFortWindImpulseRadius',
                            [('ImpulseBlendTime', 'Float', '0x10000000000180'),
                             ('ImpulseMagnitude', 'Float', '0x10000000000180'),
                             ('ImpulseRadius', 'Float', '0x10000000000180'),
                             ('ImpulseLocation', 'Struct', '0x10000000000180'),
                             ('InWindImpulseRadius', 'Struct', '0x10000008000182')]),
 'BreakWindImpulseCylinder': ('0x142b52450',
                              348267521,
                              'InWindImpulseCylinder',
                              'FFortWindImpulseCylinder',
                              [('ImpulseDirection', 'Struct', '0x10000000000180'),
                               ('ImpulseMagnitude', 'Float', '0x10000000000180'),
                               ('ImpulseOuterRadius', 'Float', '0x10000000000180'),
                               ('ImpulseInnerRadius', 'Float', '0x10000000000180'),
                               ('ImpulseLocation', 'Struct', '0x10000000000180'),
                               ('InWindImpulseCylinder', 'Struct', '0x10000008000182')]),
 'BreakWindImpulseCylinderAdvanced': ('0x142b52480',
                                      79832065,
                                      'WindImpulseCylinder',
                                      'FFortWindImpulseCylinder',
                                      [('bIsWindChangePending', 'Bool', '0x10040000000180'),
                                       ('bIsWindChanging', 'Bool', '0x10040000000180'),
                                       ('WindWorldBounds', 'Struct', '0x10040000000180'),
                                       ('WindMagnitude', 'Float', '0x10040000000180'),
                                       ('WindWorldDirection', 'Struct', '0x10040000000180'),
                                       ('WindOuterRadius', 'Float', '0x10040000000180'),
                                       ('WindInnerRadius', 'Float', '0x10040000000180'),
                                       ('WindLocation', 'Struct', '0x10040000000180'),
                                       ('WindImpulseCylinder', 'Struct', '0x10000008000182')]),
 'BreakWindImpulseCylinderDelta': ('0x142b524b0',
                                   79832065,
                                   'WindImpulseCylinderDelta',
                                   'FFortWindImpulseCylinderDelta',
                                   [('WindImpulseHandleToModify', 'Struct', '0x10040000000180'),
                                    ('WindImpulseBounds', 'Struct', '0x10040000000180'),
                                    ('WindDeltaInnerWorldBounds', 'Struct', '0x10040000000180'),
                                    ('WindDeltaOuterWorldBounds', 'Struct', '0x10040000000180'),
                                    ('WindDeltaDesiredMagnitude', 'Float', '0x10040000000180'),
                                    ('WindDeltaSectionCurrentMagnitude', 'Float', '0x10040000000180'),
                                    ('WindDeltaPreviousMagnitude', 'Float', '0x10040000000180'),
                                    ('WindDeltaSectionCurrentBlendTime', 'Float', '0x10040000000180'),
                                    ('WindDeltaSectionBlendTime', 'Float', '0x10040000000180'),
                                    ('WindDeltaDesiredOverallBlendTime', 'Float', '0x10040000000180'),
                                    ('WindDeltaMaximumRadius', 'Float', '0x10040000000180'),
                                    ('WindDeltaOuterSectionRadius', 'Float', '0x10040000000180'),
                                    ('WindDeltaInnerSectionRadius', 'Float', '0x10040000000180'),
                                    ('WindDeltaSectionWidth', 'Float', '0x10040000000180'),
                                    ('bWindRipplesOutward', 'Bool', '0x10040000000180'),
                                    ('bWindImpulseInitialized', 'Bool', '0x10040000000180'),
                                    ('WindDeltaCenter', 'Struct', '0x10040000000180'),
                                    ('WindImpulseCylinderDelta', 'Struct', '0x10000008000182')])}

def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'AFortWindManager':
        return
    for fn in functions:
        contract = CONTRACTS.get(fn['name'])
        if contract is None:
            continue
        create, expected_flags, input_name, input_type, expected_params = contract
        declared = [f for f in block['functions'] if f['name'] == fn['name']]
        if len(declared) != 1 or declared[0]['create_func'] != create:
            raise ValueError('wind break function identity changed: ' + fn['name'])
        flags, params, problem = function_view(declared[0], owner_fns)
        if problem or params is None:
            raise ValueError('wind break parameter table changed: ' + fn['name'])
        actual_params = [(p['name'], p['kind'], p['property_flags']) for p in params]
        old = input_type + '& ' + input_name
        if (problem or flags != expected_flags or sorted(actual_params) != sorted(expected_params)
                or fn['args'].count(old) != 1 or not fn['args'] or fn['args'][0] != old):
            raise ValueError('wind break parameter contract changed: ' + fn['name'])
        fn['args'][0] = 'const ' + old
