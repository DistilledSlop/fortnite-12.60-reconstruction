"""Admit the original Fortnite movement RPC receiver and its one CVar owner."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RUNTIME = ROOT / 'behavior' / 'movement_servermove67_runtime'
NAME = 'UFortMovementComp_Character'
FUNCTION = 'ServerMove_Implementation'
ARGS = ['float TimeStamp', 'FVector_NetQuantize10 InAccel',
        'FVector_NetQuantize100 ClientLoc', 'uint8 CompressedMoveFlags',
        'uint8 ClientRoll', 'uint32 View', 'UPrimitiveComponent* ClientMovementBase',
        'FName ClientBaseBoneName', 'uint8 ClientMovementMode']
SUPPORT = '../../../../behavior/movement_servermove67_runtime/cpp_support.inc'


def apply(model):
    if model.get('name') != NAME:
        return model
    if (model.get('parent') != 'UCharacterMovementComponent' or
            model.get('package') != '/Script/FortniteGame'):
        raise ValueError(NAME + ': movement receiver ancestry changed')
    helpers = [h for h in model.get('helpers', []) if h.get('name') == FUNCTION]
    competitors = [h for key in ('functions', 'static_helpers', 'fields', 'members_decl_order')
                   for h in model.get(key, []) if h.get('name') == FUNCTION]
    if len(helpers) != 1 or competitors:
        raise ValueError(NAME + ': missing or competing movement receiver; rejected=' + repr(model.get('splice', {}).get('rejected', [])))
    h = helpers[0]
    actual = (h.get('ret'), h.get('args'), h.get('const'), h.get('access'),
              h.get('kind'), h.get('static'), h.get('virtual'), h.get('override'))
    expected = ('void', ARGS, False, 'public', 'helper', False, True, True)
    if actual != expected:
        raise ValueError(NAME + ': movement receiver signature changed: ' + repr(actual))
    if h.get('body', '').strip() != (RUNTIME / 'ServerMove_Implementation.inc').read_text().strip():
        raise ValueError(NAME + ': movement receiver body changed')
    if model.get('splice', {}).get('accepted', []).count(NAME + '::' + FUNCTION) != 1:
        raise ValueError(NAME + ': movement receiver not uniquely admitted')
    includes = model.setdefault('cpp_includes', [])
    if SUPPORT in includes:
        raise ValueError(NAME + ': competing movement receiver CVar support')
    includes.append(SUPPORT)
    return model
