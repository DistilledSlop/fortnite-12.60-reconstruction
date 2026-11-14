// Original address: 0x1420d7880
// CONFIDENCE: high
if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
{
	if (const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
	{
		if (Player->IsJumpProvidingForce() && Player->bIsWaterJump)
		{
			const FWaterJumpParams& JumpParams = Player->Native_1423320a0() ? WaterJumpParamsSprint : WaterJumpParams;
			const float MinZ = JumpParams.WaterJumpMinVelocityZ.GetValueAtLevel(0.0f);
			const float MaxZ = JumpParams.WaterJumpMaxVelocityZ.GetValueAtLevel(0.0f);
			const float ForceZ = JumpParams.WaterJumpForceZ.GetValueAtLevel(0.0f);

			Velocity.Z = FMath::Max(Velocity.Z, MinZ);
			Velocity.Z += ForceZ * DeltaSeconds;
			Velocity.Z = FMath::Min(Velocity.Z, MaxZ);
		}
	}
}

Super::ApplyAccumulatedForces(DeltaSeconds);
