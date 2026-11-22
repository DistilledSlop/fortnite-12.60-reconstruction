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
const float MinSpeed = Super::GetMinAnalogSpeed();
const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
return Player && Player->bIsDBNO ? FMath::Min(MinSpeed, GetMaxSpeed()) : MinSpeed;
