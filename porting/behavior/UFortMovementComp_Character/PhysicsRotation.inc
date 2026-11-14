// Original address: 0x1420eada0
// CONFIDENCE: high
if (MovementMode == MOVE_Custom && (CustomMovementMode == 1 || CustomMovementMode == 2))
{
	if (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		NativeUpdateParachuteRotation1420f5ff0();
	}
	return;
}

if (VelocityBasedTurnCurve)
{
	if (!bWasUsingVelocityBasedTurnCurve)
	{
		RotationYawRateToRestore = RotationRate.Yaw;
	}
	const float MaxSpeed = GetMaxSpeed();
	const float SpeedRatio = (FMath::Abs(MaxSpeed) > 1e-8f) ? (Velocity.Size2D() / MaxSpeed) : 0.0f;
	RotationRate.Yaw = VelocityBasedTurnCurve->GetFloatValue(SpeedRatio);
	bWasUsingVelocityBasedTurnCurve = true;
}
else
{
	if (bWasUsingVelocityBasedTurnCurve)
	{
		RotationRate.Yaw = RotationYawRateToRestore;
	}
	bWasUsingVelocityBasedTurnCurve = false;
}

Super::PhysicsRotation(DeltaTime);
