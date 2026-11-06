// CONFIDENCE: high. Native target 0x141a1c5a0.
if (TrailParticles)
{
    const bool bActive = FMath::Abs(MovementLengthSquared) > 0.1f || (bActivateTrailOnRationalMovement && FMath::Abs(RotationalMovementAcceleration) > 0.1f);
    TrailParticles->SetFloatParameter(ParachuteTrailParameterName, bActive ? 1.0f : 0.0f);
}
