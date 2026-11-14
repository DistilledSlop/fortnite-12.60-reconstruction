// Original address: 0x1420e20f0
// CONFIDENCE: high
if (MovementMode == MOVE_Custom && CustomMovementMode == 4)
{
	if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
	{
		if (Player->CurrentSkyTube != nullptr)
		{
			return SkyTubeSkydivingControlParamsPassive.MaxLateralSpeed.GetValueAtLevel(0.0f);
		}
	}
}
return Super::GetMinAnalogSpeed();
