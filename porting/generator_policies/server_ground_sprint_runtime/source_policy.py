"""Sprint-owned defaults and an opt-in remote-server walking experiment.

The bridge is new server integration, not the original full compressed-input or
CheckJumpInput override. It earns no original-body credit and is off by default.
"""
import re
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def validate_constructor_source():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['source_windows']:
            stream.seek(int(window['address'], 0) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('sprint constructor evidence changed: ' + window['address'])

SPRINT_CTOR = '''
UFortGameplayAbility_Sprint::UFortGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Complete own assignments of 0x141615d90; superclass recovery is separate.
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    ActivationType = EFortGameplayAbilityActivation::Triggered;
    AbilityTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x690));
    ActivationOwnedTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x690));
}
'''

BRIDGE = '''
// Diagnostic integration only. Original full input/jump/ability recovery remains open.
void UFortMovementComp_Character::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteProbeServerGroundSprint"))) return;
    UWorld* World = GetWorld();
    AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
    if (!World || World->GetNetMode() != NM_DedicatedServer || !Pawn ||
        !Pawn->HasAuthority() || Pawn->IsLocallyControlled()) return;

    const uint8 PreviousStyle = static_cast<uint8>(Pawn->CurrentMovementStyle.GetValue());
    // Decode even while airborne so release/landing does not retain old input.
    Pawn->NativeSprintState1062Bit0 = (Flags & 0x40) != 0;
    if (IsMovingOnGround() && ((PreviousStyle == 3) != Pawn->NativeSprintState1062Bit0))
        Pawn->NativeUpdateMovementStyle14235bbf0();

    struct FObservation { float NextTime = 0.0f; int32 LastTagCount = -1; int32 LastInput = -1; };
    static TMap<TWeakObjectPtr<UFortMovementComp_Character>, FObservation> Observations;
    for (auto It = Observations.CreateIterator(); It; ++It)
        if (!It.Key().IsValid()) It.RemoveCurrent();
    FObservation& Observation = Observations.FindOrAdd(TWeakObjectPtr<UFortMovementComp_Character>(this));
    UFortAbilitySystemComponent* ASC = Pawn->AbilitySystemComponent;
    const int32 TagCount = ASC ? ASC->GetTagCount(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x690)) : 0;
    const int32 Input = Pawn->NativeSprintState1062Bit0 ? 1 : 0;
    if (World->GetTimeSeconds() >= Observation.NextTime || TagCount != Observation.LastTagCount || Input != Observation.LastInput)
    {
        Observation.NextTime = World->GetTimeSeconds() + 0.25f;
        Observation.LastTagCount = TagCount;
        Observation.LastInput = Input;
        UE_LOG(LogTemp, Display, TEXT("ServerGroundSprintProbe: time=%.3f flags=%u sprintInput=%d tagCount=%d previousStyle=%u style=%u mode=%d customMode=%d maxSpeed=%.3f velocity=%s"),
            World->GetTimeSeconds(), Flags, Input, TagCount, PreviousStyle,
            static_cast<uint8>(Pawn->CurrentMovementStyle.GetValue()), static_cast<int32>(MovementMode), CustomMovementMode, GetMaxSpeed(), *Velocity.ToString());
        if (ASC)
        {
            for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities().Items)
            {
                if (Spec.Ability && Spec.Ability->IsA<UFortGameplayAbility_Sprint>())
                    UE_LOG(LogTemp, Display, TEXT("ServerGroundSprintProbe: sprintSpec=%s class=%s active=%d"),
                        *Spec.Handle.ToString(), *Spec.Ability->GetClass()->GetPathName(), Spec.IsActive() ? 1 : 0);
            }
        }
    }
}
'''


def finalize(model, header, cpp):
    name = model.get('name')
    if name not in ('UFortGameplayAbility_Sprint', 'UFortMovementComp_Character'):
        return header, cpp
    if model.get('package') != '/Script/FortniteGame' or header.count('public:') != 1:
        raise ValueError(name + ': unexpected sprint integration owner')
    if name == 'UFortGameplayAbility_Sprint':
        validate_constructor_source()
        if model.get('parent') != 'UFortGameplayAbility_Action' or re.search(r'\b' + name + r'\s*\(', header):
            raise ValueError(name + ': changed constructor contract')
        header = header.replace('public:', 'public:\n\t' + name + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());', 1)
        return header, cpp + '\n#include "FortNativeGameplayTagsRuntime.h"\n' + SPRINT_CTOR
    if model.get('parent') != 'UCharacterMovementComponent' or 'UpdateFromCompressedFlags' in header + cpp:
        raise ValueError(name + ': full input override now conflicts with diagnostic bridge')
    header = header.replace('public:', 'public:\n\tvirtual void UpdateFromCompressedFlags(uint8 Flags) override;', 1)
    includes = ('AFortPlayerPawn.h', 'UFortAbilitySystemComponent.h', 'UFortGameplayAbility_Sprint.h',
                'FortNativeGameplayTagsRuntime.h', 'Engine/World.h', 'Misc/CommandLine.h', 'Misc/Parse.h')
    cpp += '\n' + ''.join('#include "' + path + '"\n' for path in includes)
    return header, cpp + BRIDGE
