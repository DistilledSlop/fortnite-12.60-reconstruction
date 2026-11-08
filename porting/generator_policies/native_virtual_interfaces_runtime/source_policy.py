"""Native virtual methods on the vehicle and team-actor interfaces (target-filter family).

The recovered `Native*163` helpers are called through `IFortVehicleInterface*` and `IFortTeamActorInterface*` pointers, so the
interfaces must declare those virtuals and the implementing classes must derive from the interface. Interface slots are
declared non-pure with a neutral default so a class that gains the base before it has every helper still compiles.
Implementers listed here are the topmost classes that own the helper; their subclasses inherit the base.
"""
VEHICLE_METHODS = [
    ('bool', 'NativeCanPassengersTakeDamage163', ''),
    ('bool', 'NativeContainsPlayers163', ''),
    ('UFortVehicleSeatComponent*', 'NativeGetSeatComponent163', ''),
    ('bool', 'NativeIsSeatFilterApplicable163', 'uint8 Seat'),
    ('TArray<APawn*>', 'NativeGetAllPassengers163', ''),
    ('AFortPlayerStateZone*', 'NativeGetPlayerStateForPawn163', 'APawn* Pawn'),
]
TEAM_METHODS = [('uint8', 'NativeGetTeam163', '')]
DEFAULT = {'bool': 'false', 'uint8': '0', 'TArray<APawn*>': '{}'}
INTERFACES = {
    'UFortVehicleInterface': dict(iname='IFortVehicleInterface', methods=VEHICLE_METHODS,
                                  ubase='UInterface', ibase=None, include='UObject/Interface.h',
                                  forwards=['class APawn;', 'class AFortPlayerStateZone;', 'class UFortVehicleSeatComponent;'],
                                  implementers={'AFortAthenaVehicle', 'AFortCharacterVehicle'}),
    'UFortTeamActorInterface': dict(iname='IFortTeamActorInterface', methods=TEAM_METHODS, forwards=[],
                                    ubase='UGenericTeamAgentInterface', ibase='IGenericTeamAgentInterface', include='GenericTeamAgentInterface.h',
                                    implementers={'ABuildingActor', 'ABuildingWeakSpot', 'AFortAIController', 'AFortAthenaAIBotController',
                                                  'AFortCarriedObject', 'AFortDisconnectedAIController', 'AFortGameplayEffectDeliveryActor',
                                                  'AFortPawn', 'AFortPickupCreative', 'AFortPlayerController', 'AFortPlayerState',
                                                  'AFortPvPPlayerStart', 'AFortTeamInfo'}),
}
BY_IMPLEMENTER = {}
for _u, _d in INTERFACES.items():
    for _c in _d['implementers']:
        BY_IMPLEMENTER.setdefault(_c, []).append(_u)


def derived_interface(name, parent):
    """UFortTeamActorInterface derives from the engine's UGenericTeamAgentInterface (AIModule), not directly from UInterface."""
    if name != 'UFortTeamActorInterface':
        return False
    if parent.get('parent_name') != 'UGenericTeamAgentInterface' or parent.get('package') != '/Script/FortniteGame':
        raise ValueError('team actor interface ancestry changed')
    return True


def _default(ret):
    if ret.endswith('*'):
        return 'nullptr'
    return DEFAULT[ret]


def apply(model):
    """Make an implementing class derive from its native interfaces."""
    for u in BY_IMPLEMENTER.get(model['name'], []):
        d = INTERFACES[u]
        bases = model.setdefault('native_interface_bases', [])
        if d['iname'] not in bases:
            bases.append(d['iname'])
        model.setdefault('splice', {}).setdefault('header_includes', set()).add(u + '.h')
    return model


def render_header(name, source_label, api):
    d = INTERFACES.get(name)
    if d is None:
        return None
    decls = ''.join('    virtual %s %s(%s);\n' % (r, n, a) for r, n, a in d['methods'])
    fwd = ''.join(f + '\n' for f in d['forwards'])
    return f'''// Generated from {source_label}; native_virtual_interfaces_runtime policy.
#pragma once
#include "CoreMinimal.h"
#include "{d['include']}"
{fwd}#include "{name}.generated.h"

UINTERFACE()
class {api} {name} : public {d['ubase']}
{{
    GENERATED_BODY()
}};

class {api} {d['iname']}{(' : public ' + d['ibase']) if d['ibase'] else ''}
{{
    GENERATED_IINTERFACE_BODY()
public:
{decls}}};
'''


def render_cpp(name, source_label):
    d = INTERFACES.get(name)
    if d is None:
        return None
    defs = ''.join('%s %s::%s(%s)\n{\n    return %s;\n}\n\n' % (r, d['iname'], n, a, _default(r)) for r, n, a in d['methods'])
    incs = '#include "AFortPlayerStateZone.h"\n#include "UFortVehicleSeatComponent.h"\n#include "GameFramework/Pawn.h"\n' if name == 'UFortVehicleInterface' else ''
    return f'''// Generated native interface default implementations ({source_label}).
#include "{name}.h"
{incs}
{defs}'''
