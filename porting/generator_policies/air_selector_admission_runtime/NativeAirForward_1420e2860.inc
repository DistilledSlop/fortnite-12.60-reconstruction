// CONFIDENCE: high
// Original 0x1420e2860: leader aim is deliberately distinct from 0x1420e4740.
if (!CharacterOwner) return FVector::ForwardVector;
if (!CharacterOwner->Controller) return CharacterOwner->GetBaseAimRotation().Vector();
const AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(CharacterOwner->Controller);
if (Controller && Controller->NativeHasSkydiveLeader_1419dd2d0())
{
    // 0x1419b6d20 reads APlayerState::PawnPrivate at original +0x278 and casts.
    const AFortPlayerState* Leader = Controller->SkydiveLeader;
    const AFortPlayerPawn* LeaderPawn = Cast<AFortPlayerPawn>(Leader->GetPawn());
    if (LeaderPawn) return LeaderPawn->GetBaseAimRotation().Vector();
}
return CharacterOwner->GetControlRotation().Vector();
