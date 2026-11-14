// Original address: 0x1420e1980
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
			return SkyTubeSkydivingControlParamsPassive.MaxAcceleration.GetValueAtLevel(0.0f) * 0.5f;
		}
	}
}
return Super::GetMaxBrakingDeceleration();
