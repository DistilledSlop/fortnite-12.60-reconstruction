// Original address: 0x1420d9260
// CONFIDENCE: high
const bool bCustomAirMode = MovementMode == MOVE_Custom && (
	CustomMovementMode == 3 ||
	CustomMovementMode == 4 ||
	CustomMovementMode == 5 ||
	CustomMovementMode == 9 ||
	CustomMovementMode == 12);

bool bUsingSkyTubeVel = false;
if (bCustomAirMode)
{
	if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
	{
		if (Player->CurrentSkyTube != nullptr)
		{
			bUsingSkyTubeVel = true;
			Velocity = NativeAirborneVelocity_1e0c;
			Acceleration = CustomFallingAcceleration;
			const float ZScalar = (Acceleration.Z <= 0.0f
				? SkyTubeDownwardAccelerationScalar
				: SkyTubeUpwardAccelerationScalar).GetValueAtLevel(0.0f);
			Acceleration.Z *= ZScalar;
		}
	}
}

Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);

if (bUsingSkyTubeVel)
{
	NativeAirborneVelocity_1e0c = Velocity;
}
