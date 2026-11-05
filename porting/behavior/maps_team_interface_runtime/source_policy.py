"""Constructor-proven team actor interface bindings for native team getters."""

BINDINGS = {'ABuildingActor': ('0x146252530', 'IFortTeamActorInterface'), 'ABuildingWeakSpot': ('0x14627bff0', 'IFortTeamActorInterface'), 'AFortAIController': ('0x1462a3340', 'IFortTeamActorInterface'), 'AFortAthenaAIBotController': ('0x1462d4250', 'IFortTeamActorInterface'), 'AFortCarriedObject': ('0x14634cc50', 'IFortTeamActorInterface'), 'AFortDisconnectedAIController': ('0x1463bc380', 'IFortTeamActorInterface'), 'AFortGameplayEffectDeliveryActor': ('0x1463dfa30', 'IFortTeamActorInterface'), 'AFortPawn': ('0x146484820', 'IFortTeamActorInterface'), 'AFortPickupCreative': ('0x146492650', 'IFortTeamActorInterface'), 'AFortPlayerController': ('0x14649da30', 'IFortTeamActorInterface'), 'AFortPlayerState': ('0x1464d05e0', 'IFortTeamActorInterface'), 'AFortPvPPlayerStart': ('0x1464ed830', 'IFortTeamActorInterface'), 'AFortTeamInfo': ('0x14652fb20', 'IFortTeamActorInterface'), 'AFortAthenaVehicle': ('0x146331890', 'IFortVehicleInterface'), 'AFortCharacterVehicle': ('0x146350a00', 'IFortVehicleInterface')}

def apply(model, block, owner_fns):
    binding = BINDINGS.get(model['name'])
    if binding is None:
        return model
    validate_evidence()
    expected, interface = binding
    if model.get('block_va') != expected or block.get('block_va') != expected:
        raise ValueError('team actor native interface class identity changed')
    bases = model.setdefault('native_interface_bases', [])
    if interface not in bases:
        bases.append(interface)
    model['splice'].setdefault('header_includes', set()).add('U' + interface[1:] + '.h')
    return model

def interface_preamble(name):
    if name == 'UFortVehicleInterface':
        return ['class APawn;', 'class AFortPlayerStateZone;']
    return []

def interface_declarations(name):
    if name == 'UFortTeamActorInterface':
        return ['public:', '    virtual uint8 NativeGetTeam163() = 0;']
    if name == 'UFortVehicleInterface':
        return ['public:', '    virtual AFortPlayerStateZone* NativeGetPlayerStateForPawn163(APawn* Pawn) = 0;']
    return []

_VALIDATED = False

def validate_evidence():
    global _VALIDATED
    if _VALIDATED:
        return
    import hashlib
    import json
    from pathlib import Path
    evidence_path = Path(__file__).parent.parent / 'target_filter163_runtime' / 'native-evidence.json'
    evidence = json.loads(evidence_path.read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['windows']:
            stream.seek(int(window['begin'], 16) - int(evidence['image_base'], 16))
            data = stream.read(window['length'])
            if len(data) != window['length'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('target filter original table changed: ' + window['begin'])
    _VALIDATED = True
