"""Original pawn aim rotation and driving predicates on the vehicle interface."""


def interface_declarations(name):
    if name != 'UFortVehicleInterface':
        return []
    return ['public:', '\tvirtual bool NativeSeatPredicate_00C8(uint8 SeatIndex) = 0;',
            '\tvirtual bool NativeSeatPredicate_0568(uint8 SeatIndex) = 0;']


def apply(model):
    if model.get('name') != 'AFortPlayerPawn':
        return model
    functions = {f['name']: f for f in model.get('helpers', [])}
    for name, result, const in [('GetBaseAimRotation', 'FRotator', True),
                                ('NativeVehicleSeatPredicate_14235ca20', 'bool', True)]:
        function = functions.get(name)
        if (function is None or function.get('ret') != result
                or function.get('args') != [] or function.get('const') != const
                or function.get('body') is None):
            raise ValueError('pawn aim rotation dependency changed: %s (%r); helpers=%r; rejected=%r' %
                             (name, {k: function.get(k) for k in ('ret', 'args', 'const')}
                              if function else None, sorted(functions), model.get('splice', {}).get('rejected', [])))
    includes = model.setdefault('cpp_includes', [])
    if 'UFortVehicleInterface.h' not in includes:
        includes.append('UFortVehicleInterface.h')
    return model
