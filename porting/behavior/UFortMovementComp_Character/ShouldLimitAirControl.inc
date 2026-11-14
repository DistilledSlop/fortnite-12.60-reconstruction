// Original address: 0x1420efc20
// CONFIDENCE: high
const bool bCustomAirMode = MovementMode == MOVE_Custom && (
	CustomMovementMode == 3 ||
	CustomMovementMode == 4 ||
	CustomMovementMode == 5 ||
	CustomMovementMode == 9 ||
	CustomMovementMode == 12);
if (bCustomAirMode)
{
	if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
	{
		if (Player->CurrentSkyTube != nullptr)
		{
			return false;
		}
	}
}
return Super::ShouldLimitAirControl(DeltaTime, FallAcceleration);
