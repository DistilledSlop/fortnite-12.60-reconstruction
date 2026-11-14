// CONFIDENCE: HIGH
// Original 0x1420e2690. PawnOwner, not CharacterOwner, is used here.
if (MovementMode == MOVE_Custom && CustomMovementMode == 9)
{
    if (const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(PawnOwner))
        return static_cast<float>(Pawn->GetNumActiveBalloons());
}
return 0.0f;
