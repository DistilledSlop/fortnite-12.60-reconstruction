// Original address: 0x1420e00f0
// CONFIDENCE: high
if (AFortPawn* FortPawn = Cast<AFortPawn>(CharacterOwner))
{
	const float OverrideAccel = FortPawn->NativeGetJumpHorizontalAccelerationOverride14231a7a0();
	if (OverrideAccel > 0.0f)
	{
		return Acceleration.GetSafeNormal2D() * OverrideAccel;
	}
}
return Super::GetFallingLateralAcceleration(DeltaTime);
