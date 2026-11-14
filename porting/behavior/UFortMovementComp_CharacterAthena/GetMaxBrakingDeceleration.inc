// Original address: 0x1419d5340
// CONFIDENCE: high
if (MovementMode == MOVE_Flying)
{
	if (AFortPawn* FortPawn = Cast<AFortPawn>(CharacterOwner))
	{
		return FortPawn->NativeGetBrakingDecelerationFlying14231a5a0() * MaxAccelerationFlyingMultiplier;
	}
}
return Super::GetMaxBrakingDeceleration();
