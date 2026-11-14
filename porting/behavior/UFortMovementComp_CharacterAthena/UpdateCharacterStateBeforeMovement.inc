// Original address: 0x1419fff40
// CONFIDENCE: high
if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
	if (Player->bIsDBNO)
	{
		bWantsToCrouch = true;
	}
}
Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
