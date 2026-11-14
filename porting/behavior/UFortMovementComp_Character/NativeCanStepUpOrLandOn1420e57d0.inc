// Original address: 0x1420e57d0
// CONFIDENCE: high
AActor* HitActor = Hit.GetActor();
if (!HitActor || !HitActor->CanBeBaseForCharacter(CharacterOwner))
{
    return false;
}
if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
    if (Player->bIsInVortex || Player->CurrentSkyTube != nullptr)
    {
        return false;
    }
}
if (MovementMode == MOVE_Custom && CustomMovementMode == 12)
{
    return false;
}
return true;
