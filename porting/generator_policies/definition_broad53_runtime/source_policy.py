"""Restore a proven native const-reference return omitted by the generic decoder."""
def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'UFortWeaponItemDefinition':
        return
    if block.get('block_va') != '0x14656f6b0':
        raise ValueError('weapon definition class identity changed')
    declared = [f for f in block.get('functions', []) if f.get('name') == 'GetReticleCornerAngles']
    if len(declared) != 1:
        raise ValueError('reticle-angle reflection function identity changed')
    flags, params, problem = function_view(declared[0], owner_fns)
    returns = [p for p in params if int(p.get('property_flags', '0'), 0) & 0x400]
    if problem or len(returns) != 1 or returns[0].get('kind') != 'Array':
        raise ValueError('reticle-angle return reflection changed')
    if int(returns[0]['property_flags'], 0) & 0x8000402 != 0x8000402:
        raise ValueError('reticle-angle return no longer const-reference')
    for fn in functions:
        if fn['name'] == 'GetReticleCornerAngles':
            if fn['ret'] != 'TArray<float>' or fn['args'] or not fn['const']:
                raise ValueError('reticle-angle native signature changed')
            fn['ret'] = 'const TArray<float>&'
