// Original address: 0x1420da140
// CONFIDENCE: high
if (Velocity.IsZero() && !bOrientRotationToMovement)
{
	return CurrentRotation;
}
return Super::ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRotation);
