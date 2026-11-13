"""Recovered default construction of FAirControlParams (0x1420d2af0)."""

_FIELDS = ('MaxAcceleration', 'LateralFriction', 'MaxLateralSpeed',
           'TerminalVelocity', 'UpwardTerminalVelocity', 'GravityScalar',
           'CustomGravityCeiling', 'CustomGravityCeilingWidth')


def declarations(model):
    if model['cpp_name'] != 'FAirControlParams':
        return []
    members = [(m['offset'], m['decl']) for m in model['members']]
    expected = [(i * 32, 'FScalableFloat %s;' % name)
                for i, name in enumerate(_FIELDS)]
    if (model['package'] != '/Script/FortniteGame' or model['size'] != 256
            or members != expected):
        raise ValueError('FAirControlParams reflected identity changed')
    # UE FScalableFloat(float) initializes Value, sentinel, Curve and cache.
    initializers = ', '.join('%s(0.0f)' % name for name in _FIELDS)
    return ['\tFAirControlParams() : %s {}' % initializers, '']
