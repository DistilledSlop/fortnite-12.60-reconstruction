// Original address: 0x1420e6d30
// CONFIDENCE: high
if (MovementMode != MOVE_Custom || (CustomMovementMode != 1 && CustomMovementMode != 2))
{
    if (CharacterOwner)
    {
        const FRotator CurrentRot = CharacterOwner->GetActorRotation();
        CharacterOwner->SetActorRotation(FRotator(0.0f, CurrentRot.Yaw, 0.0f));
    }
}
